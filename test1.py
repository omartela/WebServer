import http.client
import os
import mimetypes
import uuid



def check_response(response, expected_status, expected_in_body=""):
    status_ok = response.status == expected_status
    body = response.read().decode()
    body_ok = expected_in_body in body
    print(f"-> Status: {response.status} {'✓' if status_ok else '✗'}")
    print(f"-> Body contains '{expected_in_body}': {'✓' if status_ok else '✗'}")
    return status_ok and body_ok

def static_file(path, expected_type, HOST, PORT):
    conn = http.client.HTTPConnection(HOST, PORT)
    conn.request("GET", path)
    response = conn.getresponse()
    type = response.getheader("Content-Type")
    print(f"GET {path}: {response.status} {response.reason}")
    print(f"-> Content-Type: {type}")
    print(response.read().decode())
    conn.close()
    return type == expected_type

def send_get(path, HOST, PORT):
    conn = http.client.HTTPConnection(HOST, PORT)
    conn.request("GET", path)
    response = conn.getresponse()
    print(f"GET {path}: {response.status} {response.reason}")
    print(response.read().decode())
    conn.close()

def send_post(path, content, content_type, HOST, PORT):
    conn = http.client.HTTPConnection(HOST, PORT)
    headers = {
        "Content-Type": content_type,
        "Content-Length": str(len(content))
    }
    conn.request("POST", path, body=content, headers=headers)
    response = conn.getresponse()
    print(f"POST {path}: {response.status} {response.reason}")
    print(response.read().decode())
    conn.close()

def send_cgi_post(path, data, type, HOST, PORT):
    conn = http.client.HTTPConnection(HOST, PORT)
    headers = {
        "Content-Type": type,
        "Content-Length": str(len(data))
    }
    conn.request("POST", path, body=data, headers=headers)
    response = conn.getresponse()
    print(f"POST {path}: {response.status} {response.reason}")
    print(response.read().decode())
    conn.close()

def send_multipart(path, filename, content, HOST, PORT):
    boundary = uuid.uuid4().hex
    conn = http.client.HTTPConnection(HOST, PORT)
    type = mimetypes.guess_type(filename)[0] or 'application/octet-stream'
    lines = [
        f"--{boundary}",
        f'Content-Disposition: form-data; name="file"; filename="{filename}"',
        f"Content-Type: {type}",
        "",
        content,
        f"--{boundary}--",
        "",
    ]
    body = "\r\n".join(lines).encode("utf-8")
    headers = {
        "Content-Type": f"multipart/form-data; boundary={boundary}",
        "Content-Length": str(len(body))
    }
    conn.request("POST", path, body=body, headers=headers)
    response = conn.getresponse()
    print(f"POST {path}: {response.status} {response.reason}")
    print(response.read().decode())
    conn.close()

def send_delete(path, HOST, PORT):
    conn = http.client.HTTPConnection(HOST, PORT)
    conn.request("DELETE", path)
    response = conn.getresponse()
    print(f"DELETE {path}: {response.status} {response.reason}")
    print(response.read().decode())
    conn.close()

# Example usage
if __name__ == "__main__":
    HOST = "127.0.0.1"
    PORT = 8001
    HOST2 = "127.0.0.2"
    PORT2 = 8002

    # send_get("/index.html", HOST, PORT)
    # send_get("/index.html", HOST2, PORT2)
    # send_get("/cgi-bin/echo.py", HOST, PORT)
    # send_get("/cgi-bin/echo.py", HOST2, PORT2)
    # print("^GET DONE^\n")

    send_post("/uploads/test.txt", "This is a test upload.", "text/plain", HOST, PORT)
    # send_post("/uploads/test.txt", "This is a test upload.", "text/plain", HOST2, PORT2)
    # file_name = "test_upload.txt"
    # file_content = "This is content of the test file."
    # send_multipart("/uploads", file_name, file_content, HOST, PORT)
    # send_multipart("/uploads", file_name, file_content, HOST2, PORT2)
    # form_data = "name=NikolaiTest&lang=Python"
    # send_cgi_post("/cgi-bin/echo_post.py", form_data, "application/x-www-form-urlencoded", HOST, PORT)
    # send_cgi_post("/cgi-bin/echo_post.py", form_data, "application/x-www-form-urlencoded", HOST2, PORT2)
    # print("^POST DONE^\n")

    # static_file("/www/index.html", "text/html", HOST, PORT)
    # static_file("/www/index.html", "text/html", HOST2, PORT2)
    # print("^Static file DONE^\n")

    # send_get("/uploads/test_upload.txt", HOST, PORT)
    # send_get("/uploads/test_upload.txt", HOST2, PORT2)
    # send_get("/uploads/test.txt", HOST, PORT)
    # send_get("/uploads/test.txt", HOST2, PORT2)
    # send_get("uploads/this", HOST,PORT)
    # send_get("uploads/this", HOST2, PORT2)
    # print("^GET DONE^\n")

    # send_delete("/uploads/test_upload.txt", HOST, PORT)
    # send_delete("/uploads/test_upload.txt", HOST2, PORT2)
    # send_delete("/uploads/test.txt", HOST, PORT)
    # send_delete("/uploads/test.txt", HOST2, PORT2)
    # print("^DELETE DONE^\n")

    #send_get("/uploads/test_upload.txt")  # Check if deleted
    #send_get("uploads/test.txt")
    #"application/x-www-form-urlencoded"