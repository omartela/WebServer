#!/usr/bin/env python3
import sys
import os
content_length = os.environ.get('CONTENT_LENGTH')
body = ''
if content_length:
    body = sys.stdin.read(int(content_length))
print("Content-Type: text/plain")
print("\r\n\r\n")
print(f"Received POST data:\n{body}")