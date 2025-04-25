import http.client
import os

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

def send_delete(path):
    conn = http.client.HTTPConnection(HOST, PORT)
    conn.request("DELETE", path)
    response = conn.getresponse()
    print(f"DELETE {path}: {response.status} {response.reason}")
    print(response.read().decode())
    conn.close()

# Example usage
if __name__ == "__main__":
    send_get("/index.html")
    print("^GET DONE^\n")
    send_post("/upload/test.txt", "This is a test upload.")
    print("^POST DONE^\n")
    send_get("/upload/test.txt")  # Check if uploaded
    send_delete("/upload/test.txt")
    print("^DELETE DONE^\n")
    send_get("/upload/test.txt")  # Check if deleted