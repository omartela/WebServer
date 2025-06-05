#!/usr/bin/env python3
import socket

HOST = '127.0.0.1'  # or your server's IP
PORT = 8080        # change if needed

def send_request_and_read_response():
    with socket.create_connection((HOST, PORT)) as sock:
        # Send basic HTTP request
        request = (
            "GET /cgi/big_response.py HTTP/1.1\r\n"
            f"Host: {HOST}\r\n"
            "Connection: close\r\n"
            "\r\n"
        )
        response_data = b""
        sock.sendall(request.encode())

        total = 0
        while True:
            chunk = sock.recv(8192)
            if not chunk:
                break
            response_data += chunk
            total += len(chunk)
            print(f"📦 Received {len(chunk)} bytes, total: {total}")
    
    print(f"\n✅ Final total received: {total / 1024:.1f} KB")
    print("\n🔽 Response:")
    print(response_data.decode(errors="replace"))

if __name__ == "__main__":
    send_request_and_read_response()