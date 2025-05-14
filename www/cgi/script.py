#!/usr/bin/env python3
import sys, os
print("Content-Type: text/plain\n")
print("Method:", os.environ.get("REQUEST_METHOD"))
print("Body:", sys.stdin.read())