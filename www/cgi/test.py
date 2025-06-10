#!/usr/bin/env python3
import os, sys, time
import os, sys, time
print("Content-Type: text/plain\n")
print(f"Method: {os.environ.get('REQUEST_METHOD')}")
print(f"Query: {os.environ.get('QUERY_STRING', '')}")
print(f"Query: {os.environ.get('QUERY_STRING', '')}")
print(f"Content-Length: {os.environ.get('CONTENT_LENGTH')}")
print(f"Content-Type: {os.environ.get('CONTENT_TYPE')}")
print("\r\n\r\nBody:", end='')
print(sys.stdin.read(), end='');
