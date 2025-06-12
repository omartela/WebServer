#!/usr/bin/env python3

import aiohttp
import asyncio
import pytest
import random
import uuid
import mimetypes
from urllib.parse import urlencode

HOST = "http://127.0.0.2:8004"
# UPLOAD_PATH = "/uploads"
CGI_PATH = "/cgi-bin/echo_post.py"
STATIC_PATHS = ["/", "/cgi/", "/upload/", "/images/"]

sample_data = {
    "GET": [("/index.html", "text/html")],
    "POST": [("formhandler.py", "application/json", '{"test": true}')],
    "MPPOST": [("test_stress.html", "text/html", "<h1>Stress</h1>")],
    "CGI": [("name=test&lang=python",)],
    "DELETE": [("/uploads/index.html", "text/html")],
}

sample_queries = [
    {"name": "Alice", "lang": "Python"},
    {"search": "stress", "type": "html"},
    {"client": "test", "q": "value"},
]

async def fetch(session, method, url, **kwargs):
    try:
        async with session.request(method, url, **kwargs) as response:
            text = await response.text()
            return response.status, text[:100]
    except Exception as e:
        return 0, str(e)

async def run_single_client(client_id, iterations=10, delay=0.1):
    connector = aiohttp.TCPConnector(limit=10)
    stats = {"success": 0, "fail": 0}
    async with aiohttp.ClientSession(connector=connector) as session:
        for i in range(iterations):
            action = random.choice(list(sample_data.keys()))
            item = random.choice(sample_data[action])
            try:
                if action == "GET":
                    path, _ = item
                    query = urlencode(random.choice(sample_queries))
                    full_url = f"{HOST}{path}?{query}"
                    print(f"[Client {client_id} GET] {full_url}")
                    status, _ = await fetch(session, "GET", full_url)

                elif action == "POST":
                    filename, content_type, content = item
                    base_path = random.choice(STATIC_PATHS)  # Pick a random location
                    path = f"{base_path.rstrip('/')}/{filename}"  # Cleanly join the path and filename
                    full_url = f"{HOST}{path}"
                    print(f"[Client {client_id} POST] {full_url}")
                    status, _ = await fetch(
                        session,
                        "POST",
                        full_url,
                        headers={"Content-Type": content_type},
                        data=content
                    )

                elif action == "DELETE":
                    path, _ = item
                    full_url = f"{HOST}{path}"
                    print(f"[Client {client_id} DELETE] {full_url}")
                    status, _ = await fetch(session, "DELETE", full_url)

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
                    full_url = f"{HOST}{path}"
                    print(f"[Client {client_id} MPPOST] {full_url}")
                    status, _ = await fetch(session, "POST", full_url, headers=headers, data=body.encode())

                elif action == "CGI":
                    query = urlencode(random.choice(sample_queries))
                    base_path = random.choice(STATIC_PATHS)  # Pick a random location
                    path = f"{base_path.rstrip('/')}/{filename}"  # Cleanly join the path and filename
                    full_url = f"{HOST}{path}"
                    print(f"[Client {client_id} CGI] {full_url}")
                    status, _ = await fetch(session, "GET", full_url)

                if 200 <= status < 300:
                    stats["success"] += 1
                else:
                    stats["fail"] += 1

            except Exception as e:
                print(f"[Client {client_id} ERROR] {e}")
                stats["fail"] += 1

            await asyncio.sleep(delay)

    return stats

@pytest.mark.asyncio
async def test_async_stress():
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


