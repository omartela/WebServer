#!/usr/bin/env python3
import socket

HOST = '127.0.0.1'  # or your server's IP
PORT = 8001        # change if needed

def send_request_and_read_response():
    with socket.create_connection((HOST, PORT)) as sock:
        # Send basic HTTP request
        request = (
            "GET /cgi/big_response.py HTTP/1.1\r\n"
            f"Host: {HOST}\r\n"
            "Connection: close\r\n"
            "\r\n"
        )
        sock.sendall(request.encode())

        total = 0
        while True:
            chunk = sock.recv(4096)
            if not chunk:
                break
            total += len(chunk)
            print(f"Received {len(chunk)} bytes, total: {total}")
        
        print(f"\n✅ Total response size: {total / (1024*1024):.2f} MB")

if __name__ == "__main__":
    send_request_and_read_response()
