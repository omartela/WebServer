import sys
import time

while True:
    with open("test123.txt", "a") as f:
        f.write("A")
    time.sleep(1)
    #break

# import os

# url = "http://localhost:8000"  # Change this to your server URL

# while True:
#     os.system(f"curl -m 5 {url}")
#     time.sleep

# import socket
# import time

# host = "localhost"
# port = 8000

# s = socket.create_connection((host, port))
# print("Connected. Sending trickle data...")

# try:
#     while True:
#         s.send(b"A")   # One byte at a time
#         time.sleep(2)  # Keep connection alive, slow drip
# except KeyboardInterrupt:
#     s.close()

# import socket
# import time

# host = "localhost"
# port = 8000

# # Open a socket and send a complete HTTP request
# s = socket.create_connection((host, port))
# s.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")  # Full request

# # Do nothing after this — just keep the connection alive
# print("Request sent. Now idle...")

# while True:
#     time.sleep(10)

