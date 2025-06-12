#!/usr/bin/env python3

import aiohttp
import asyncio
import pytest
import random
import uuid
import mimetypes
from urllib.parse import urlencode

HOST = "http://127.0.0.2:8004"
CGI_PATH = "/cgi-bin/echo_post.py"
STATIC_PATHS = ["cgi/", "uploads/", "images/"]

sample_data = {
    "GET": [("index.html", "text/html"),
            ("form.html", "text/html"),
            ("product.html", "text/html"),
            ("cgi/", "text/html"),
            ("uploads/", "text/html"),
            ("images/", "text/html"),
            ("images/nikolai.jpg", "image/jpeg"),
            ("formhandler.py", "text/html"),
            ("formhandler.html", "text/html"),
            ],
    "POST": [("test.txt", "text/plain", 'This is test'),
             ("formhandler.py", "application/x-www-form-urlencoded", "name=Mouse&lang=en"),
             ("test.py", "application/x-www-form-urlencoded", ""),
             ("test.py", "application/x-www-form-urlencoded", "This is test"),
            ],
    "MPPOST": [("test_stress.html", "text/html", "<h1>Stress</h1>"),
               ("sample_image.jpg", "image/jpeg", open("www/images/sample_image.jpg", "rb").read()),
               ("data.json", "application/json", '{"name": "Joel", "role": "tester", "valid": true}'),
              ],
    "CGI": ["formhandler.py",
            "GET.py",
            "test.py",

           ],
    "DELETE": [("upload/test.txt", "text/html")],
}

sample_queries = [
    {"name": "Alice", "lang": "Python"},
    {"id": 1, "name": "Keyboard", "lang": "en", "price": 49.99},
    {"id": 2, "name": "Screen", "lang": "none", "price": 340.99},
    {"id": 3, "name": "Hoover", "lang": "none", "price": 76.83},
    {"id": 4, "name": "Mouse", "lang": "en", "price": 19.99},
    {"id": 5, "name": "Souris", "lang": "fr", "price": 24.99},
]

expected_config = {
    "/uploads/": {"allowed_methods": ["GET", "POST", "DELETE"]},
    "/cgi/": {"allowed_methods": ["GET", "POST"], "is_cgi": True},
    "/images/": {"allowed_methods": ["GET", "POST", "DELETE"], "autoindex": False},
}

async def fetch(session, method, url, **kwargs):
    try:
        async with session.request(method, url, **kwargs) as response:
            text = await response.text()
            return response.status, text[:100]
    except Exception as e:
        return 0, str(e)

def is_expected_error(action, url, status):
    # Allow 404 on DELETE (file might not exist)
    if action == "DELETE" and status == 404:
        return True
    # Allow 404 on GET to known missing resources
    if action == "GET" and any(missing in url for missing in ["missing", "notfound"]):
        return True
    # Allow 405 if method is not allowed on some endpoints
    if status == 405:
        return True
    # Accept 403 if access denied (optional)
    if status == 403:
        return True
    return False

def validate_config_behavior(id, path, method, status):
    config = expected_config.get(path)
    if not config:
        return True  # unknown path, can't test
    if method not in config["allowed_methods"] and status != 405:
        print(f"[{id} ❌] Method {method} was allowed on {path} but shouldn't be!")
        return False
    if method in config["allowed_methods"] and status == 405:
        print(f"[{id} ❌] Method {method} was blocked on {path} but should be allowed!")
        return False
    return True

async def run_single_client(client_id, iterations=10, delay=0.1):
    counter = 0
    connector = aiohttp.TCPConnector(limit=10)
    stats = {"success": 0, "fail": 0}
    async with aiohttp.ClientSession(connector=connector) as session:
        for i in range(iterations):
            request_id = f"C{client_id}-R{counter}"
            counter += 1
            action = random.choice(list(sample_data.keys()))
            item = random.choice(sample_data[action])
            http_method = "POST" if action in ["MPPOST", "CGI"] else action
            try:
                if action == "GET":
                    path, _ = item
                    full_url = f"{HOST}/{path}"
                    if random.choice([True, False]):
                        query = urlencode(random.choice(sample_queries))
                        full_url += f"?{query}"
                    print(f"[{request_id} GET] {full_url}")
                    status, body = await fetch(session, "GET", full_url)
                    print(f"[{request_id} GET RESPONSE] {body}")

                elif action == "POST":
                    filename, content_type, content = item
                    base_path = random.choice(STATIC_PATHS)  # Pick a random location
                    path = f"{base_path.rstrip('/')}/{filename}"  # Cleanly join the path and filename
                    full_url = f"{HOST}/{path}"
                    print(f"[{request_id} POST] {full_url}")
                    status, body = await fetch(
                        session,
                        "POST",
                        full_url,
                        headers={"Content-Type": content_type},
                        data=content
                    )
                    print(f"[{request_id} POST RESPONSE] {body}")

                elif action == "DELETE":
                    path, _ = item
                    full_url = f"{HOST}/{path}"
                    print(f"[{request_id} DELETE] {full_url}")
                    status, body = await fetch(session, "DELETE", full_url)
                    print(f"[{request_id} DELETE RESPONSE] {body}")

                elif action == "MPPOST":
                    filename, content_type, content = item
                    boundary = uuid.uuid4().hex
                    body = (
                        f"--{boundary}\r\n"
                        f'Content-Disposition: form-data; name="file"; filename="{filename}"\r\n'
                        f"Content-Type: {content_type}\r\n\r\n"
                        f"{content}\r\n"
                        f"--{boundary}--\r\n"
                    )
                    headers = {"Content-Type": f"multipart/form-data; boundary={boundary}"}
                    base_path = random.choice(STATIC_PATHS)  # Pick a random location
                    path = f"{base_path.rstrip('/')}/{filename}"  # Cleanly join the path and filename
                    full_url = f"{HOST}/{path}"
                    print(f"[{request_id} MPPOST] {full_url}")
                    status, body = await fetch(session, "POST", full_url, headers=headers, data=body.encode())
                    print(f"[{request_id} MPPOST RESPONSE] {body}")

                elif action == "CGI":
                    filename = item
                    base_path = "/cgi"
                    path = f"{base_path}/{filename}"
                    full_url = f"{HOST}{path}"
                    if random.choice([True, False]):
                        query = urlencode(random.choice(sample_queries))
                        full_url += f"?{query}"

                    print(f"[{request_id} CGI] {full_url}")
                    status, body = await fetch(session, "GET", full_url)
                    print(f"[{request_id} CGI RESPONSE] {body}")

                # Extract top-level path for config validation
                url_path = full_url.replace(HOST, "").split("?")[0]  # Remove host and query
                base_path = "/" + url_path.strip("/").split("/")[0] + "/"  # e.g., /upload/
                
                if not validate_config_behavior(request_id, base_path, http_method, status):
                    print(f"[{request_id} 🚫 CONFIG VIOLATION] {action} on {base_path} returned {status}")
                    stats["fail"] += 1
                    continue  # Skip normal handling to avoid double-counting
                if 200 <= status < 300:
                    print(f"[{request_id} ✅ SUCCESS] {action} {full_url} [{status}]")
                    stats["success"] += 1
                elif is_expected_error(action, full_url, status):
                    print(f"[{request_id} ⚠️ EXPECTED ERROR] {action} {full_url} [{status}]")
                    stats["success"] += 1
                else:
                    print(f"[{request_id} ❌ FAIL] {action} {full_url} [{status}]")
                    print(f"[{request_id} RESPONSE BODY] {body}")
                    stats["fail"] += 1

            except Exception as e:
                print(f"[{request_id} ERROR] {e}")
                stats["fail"] += 1

            await asyncio.sleep(delay)

    return stats

@pytest.mark.asyncio
async def test_async_stress():
    print()
    clients = 3
    iterations = 10
    delay = 0.05
    results = await asyncio.gather(*(run_single_client(cid, iterations, delay) for cid in range(clients)))
    total_success = sum(r["success"] for r in results)
    total_fail = sum(r["fail"] for r in results)

    print(f"\n✅ Total success: {total_success}")
    print(f"❌ Total fail: {total_fail}")

    assert total_success > 0, "No successful requests"
    assert total_fail < (clients * iterations), "All requests failed"


