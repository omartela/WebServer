#!/usr/bin/env python3
import sys
import os

print("Content-Type: text/plain\n")
print("Request Method:", os.environ.get("REQUEST_METHOD"))
print("Content Length:", os.environ.get("CONTENT_LENGTH"))
print("Body:")
print(sys.stdin.read())
