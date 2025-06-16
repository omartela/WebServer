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
print("\r\n\r\nBody:")
with open("/tmp/cgi_was_run.txt", "a") as f:
    f.write("CGI script executed at {}!\n".format(time.time()))
    body = sys.stdin.read();
    f.write(body);
    print(f"{body}");
