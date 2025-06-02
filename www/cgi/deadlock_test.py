#!/usr/bin/env python3
import sys
import time

print("Content-Type: text/plain")
print()  # End of headers
sys.stdout.flush()

# Write a large response chunk by chunk BEFORE reading all input
chunk = "X" * (64 * 10)  # 64 KB chunk

for _ in range(1):  # ~640 KB output total
    sys.stdout.write(chunk)
    sys.stdout.flush()
    time.sleep(0.1)  # slow it down a bit to simulate streaming

# Now try to read all input (this may block if pipe buffer is full)
body = sys.stdin.read()

# Just echo back the size of input received at the end
sys.stdout.write(f"\nReceived input size: {len(body)} bytes\n")
sys.stdout.flush()
