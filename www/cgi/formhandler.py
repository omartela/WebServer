#!/usr/bin/env python3

import os
import cgi
import cgitb
import urllib.parse

cgitb.enable()

print("Content-Type: text/plain\r\n\r\n",  end="")

method = os.environ.get("REQUEST_METHOD", "UNKNOWN")

print(f"Request Method: {method}")

if method == "GET":
    query_string = os.environ.get("QUERY_STRING", "")
    print(f"\nQUERY_STRING: {query_string}")
    params = urllib.parse.parse_qs(query_string)
elif method == "POST":
    form = cgi.FieldStorage()
    params = {}
    for key in form.keys():
        params[key] = form.getlist(key)
else:
    print("\nUnsupported request method.")
    params = {}

if params:
    print("\nParsed parameters:")
    for key, values in params.items():
        for value in values:
            print(f"{key} = {value}")
else:
    print("\nNo parameters found.")