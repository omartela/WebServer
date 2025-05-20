import socket
import time

s = socket.socket()
s.connect(('127.0.0.1', 8004))

req = "GET / HTTP/1.1\r\nHost: 127.0.0.1\r\n"

# Send one character at a time
# for c in req:
	# s.send(c.encode())
	# time.sleep(1.5)  # 1.5 seconds between each byte (too slow)
s.send(req.encode())
# Finish headers
s.send(b"\r\n\r\n")
