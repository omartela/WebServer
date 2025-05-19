import socket
import time

# s = socket.socket()
# s.connect(('127.0.0.1', 8004))

# req = "GET / HTTP/1.1\r\n"

# #Send one character at a time,
# for c in req:
#     s.send(c.encode())
#     time.sleep(1.5)  # 1.5 seconds between each byte (too slow)

# #Finish headers,
# s.send(b"\r\n\r\n")


s = socket.socket()
s.connect(('127.0.0.1', 8080))

req1 = "GET /first HTTP/1.1\r\nHost: localhost\r\n\r\n"
req2 = "WRITE /second HTTP/1.1\r\nHost: localhost\r\n\r\n"
s.sendall(req1.encode())

BUFFER = 1
second = False
while True:
    response = s.recv(BUFFER)
    print(response)
    if not response:
        break
    time.sleep(3)
    if second == False:
        s.sendall(req2.encode())
        second = True

s.close()

#Finish headers,
# s.send(b"\r\n\r\n")