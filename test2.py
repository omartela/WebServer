import socket
import time

s = socket.socket()
s.connect(('127.0.0.1', 8080))

req = "GET / HTTP/1.1\r\n"

#Send one character at a time
try:
    for c in req:
        s.send(c.encode())
        time.sleep(1.5)  # 1.5 seconds between each byte (too slow)
    s.send(b"\r\n\r\n")
except BrokenPipeError:
    print("Server closed connection")
    response = s.recv(1024)
    print(f"Response: {response}")

try:
    response = s.recv(1024)
    print(response)
except Exception as e:
    print("No response:", e)
finally:
    s.close()



# s = socket.socket()
# s.connect(('127.0.0.1', 8080))

# req1 = "GET /index HTTP/1.1\r\nHost: localhost\r\n\r\n"
# req2 = "WRITE /second HTTP/1.1\r\nHost: localhost\r\n\r\n"
# s.sendall(req1.encode())

# BUFFER = 1
# second = False
# while True:
#     response = s.recv(BUFFER)
#     print(response)
#     if not response:
#         break
#     time.sleep(3)
#     if second == False:
#         s.sendall(req2.encode())
#         second = True

# s.close()

# import socket
# import threading
# import time

# HOST = "localhost"
# PORT = 8080
# NUM_CLIENTS = 5
# EXPECTED_RESPONSE_START = "HTTP/1.1 200 OK"

# # Simulate slow reading client
# def slow_client(index):
#     with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
#         s.connect((HOST, PORT))
#         request = (
#             f"POST / HTTP/1.1\r\n"
#             f"Host: {HOST}:{PORT}\r\n"
#             f"Connection: keep-alive\r\n"
#             f"Content-Length: 299"
#             f"\r\n"
#             f"If youve got a question about one behavior, you should compare your program behavior with NGINXs. For example, check how does server_name work. Weve shared with you a small tester. Its not mandatory to pass it if everything works fine with your browser and tests, but it can help you hunt some bugs."
#         )
#         s.sendall(request.encode())

#         received = b""
#         while True:
#             chunk = s.recv(32)  # small recv buffer to simulate slowness
#             if not chunk:
#                 break
#             received += chunk
#             time.sleep(0.05)  # pause to slow down the reading

#         response_str = received.decode(errors="ignore")
#         print(f"[Client {index}] Received {len(received)} bytes")
#         assert response_str.startswith(EXPECTED_RESPONSE_START), f"[Client {index}] Invalid response!"

# threads = []
# for i in range(NUM_CLIENTS):
#     t = threading.Thread(target=slow_client, args=(i,))
#     threads.append(t)
#     t.start()

# for t in threads:
#     t.join()

# print("✅ All clients received correct responses with slow recv()")