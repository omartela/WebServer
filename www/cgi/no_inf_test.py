import sys
import time

while True:
    with open("test123.txt", "a") as f:
        f.write("A")
    time.sleep(1)
    break