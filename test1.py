import http.client
import os
import mimetypes
import uuid


HOST = "localhost"
PORT = 8080  # Change to your server's port

def send_get(path):
    conn = http.client.HTTPConnection(HOST, PORT)
    conn.request("GET", path)
    response = conn.getresponse()
    print(f"GET {path}: {response.status} {response.reason}")
    print(response.read().decode())
    conn.close()

def send_post(path, content, content_type="text/plain"):
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

def send_cgi_post(path, data, type="application/x-www-form-urlencoded"):
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

def send_multipart(path, filename, content):
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

def send_delete(path):
    conn = http.client.HTTPConnection(HOST, PORT)
    conn.request("DELETE", path)
    response = conn.getresponse()
    print(f"DELETE {path}: {response.status} {response.reason}")
    print(response.read().decode())
    conn.close()

# Example usage
if __name__ == "__main__":
    send_get("www/index.html")
    send_get("/cgi-bin/echo.py")
    print("^GET DONE^\n")
    send_post("/uploads/test.txt", "This is a test upload.")
    file_name = "test_upload.txt"
    file_content = "This is content of the test file."
    send_multipart("/uploads", file_name, file_content)
    form_data = "name=NikolaiTest&lang=Python"
    send_cgi_post("/cgi-bin/echo_post.py", form_data)
    print("^POST DONE^\n")
    send_get("/uploads/test_upload.txt")  # Check if uploaded
    send_get("/uploads/test.txt")
    print("^GET DONE^\n")
    send_delete("/uploads/test_upload.txt")
    send_delete("/uploads/test.txt")
    print("^DELETE DONE^\n")
    send_get("/uploads/test_upload.txt")  # Check if deleted
    send_get("uploads/test.txt")