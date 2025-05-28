#!/usr/bin/env python3

import sys
import os

print("Content-Type: text/plain")
print("")  # End of headers

# Read entire stdin (request body)
body = sys.stdin.read()

# Respond with a large response (repeat the input)
for _ in range(100):
    sys.stdout.write(body)
    sys.stdout.flush()
