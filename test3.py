import requests
import socket
import time

BASE_URL = "http://localhost:8080"

def test_basic_connection():
    try:
        response = requests.get(BASE_URL)
        print(f"Basic connection test: {response.status_code}")
    except Exception as e:
        print(f"Basic connection test failed: {e}")


def test_slow_header_send():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(('localhost', 8080))
        s.sendall(b"GET / HTTP/1.1\r\n")
        time.sleep(5)  # Delay to simulate slow header send
        s.sendall(b"Host: localhost\r\n\r\n")
        response = s.recv(4096)
        print(f"Slow header send test: {response.decode()}")
    except Exception as e:
        print(f"Slow header send test failed: {e}")
    finally:
        s.close()


def test_oversized_header():
    try:
        oversized_header = "A" * 9000
        headers = {"X-Oversized": oversized_header}
        response = requests.get(BASE_URL, headers=headers)
        print(f"Oversized header test: {response.status_code}")
    except Exception as e:
        print(f"Oversized header test failed: {e}")


def test_slow_body_send():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(('localhost', 8080))
        s.sendall(b"POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 100\r\n\r\n")
        time.sleep(5)
        s.sendall(b"A" * 50)
        time.sleep(5)
        s.sendall(b"A" * 50)
        response = s.recv(4096)
        print(f"Slow body send test: {response.decode()}")
    except Exception as e:
        print(f"Slow body send test failed: {e}")
    finally:
        s.close()


def test_oversized_body():
    try:
        oversized_body = "A" * 1000000  # 1 MB body
        response = requests.post(BASE_URL, data=oversized_body)
        print(f"Oversized body test: {response.status_code}")
    except Exception as e:
        print(f"Oversized body test failed: {e}")


if __name__ == "__main__":
    test_basic_connection()
    test_slow_header_send()
    test_oversized_header()
    test_slow_body_send()
    test_oversized_body()
