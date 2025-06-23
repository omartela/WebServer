#!/usr/bin/env python3
import sys
import os
import time

def log(msg):
    with open("cgi_debug.log", "a") as f:
        f.write(f"[{time.strftime('%Y-%m-%d %H:%M:%S')}] {msg}\n")
        f.flush()

try:
    log("CGI script started")

    body = sys.stdin.read()
    log(f"Read {len(body)} bytes from stdin")

    body = body * 2  # Simulate large output
    log(f"Amplified body to {len(body)} bytes")

    print("Content-Type: text/plain")
    print(f"Content-Length: {len(body)}")
    print("")  # End of headers

    sys.stdout.write(body)
    sys.stdout.flush()
    log("CGI script completed successfully")

except Exception as e:
    log(f"ERROR: {str(e)}")
