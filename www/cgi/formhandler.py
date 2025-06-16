#!/usr/bin/env python3
import os
import cgi
import cgitb
import urllib.parse

cgitb.enable()
print("Content-Type: text/html\r\n\r\n")

# In-memory product DB
PRODUCTS = [
    {"id": 1, "name": "Keyboard", "lang": "en", "price": 49.99},
    {"id": 2, "name": "Screen", "lang": "none", "price": 340.99},
    {"id": 3, "name": "Hoover", "lang": "none", "price": 76.83},
    {"id": 4, "name": "Mouse", "lang": "en", "price": 19.99},
    {"id": 5, "name": "Souris", "lang": "fr", "price": 24.99},
]

# Get query params
method = os.environ.get("REQUEST_METHOD", "UNKNOWN")
params = {}

if method == "GET":
    query_string = os.environ.get("QUERY_STRING", "")
    params = urllib.parse.parse_qs(query_string)
elif method == "POST":
    form = cgi.FieldStorage()
    for key in form.keys():
        params[key] = form.getlist(key)

# Match products
def match_products(params):
    results = PRODUCTS
    if "name" in params:
        name_filter = params["name"][0].lower()
        results = [p for p in results if name_filter in p["name"].lower()]
    if "lang" in params:
        lang_filter = params["lang"][0].lower()
        results = [p for p in results if lang_filter in p["lang"].lower()]
    return results

# Prepare search results HTML
results_html = ""
if params:
    matches = match_products(params)
    if matches:
        results_html += "<h2>Search Results:</h2><ul>"
        for p in matches:
            results_html += f"<li>{p['name']} ({p['lang']}) - ${p['price']}</li>"
        results_html += "</ul>"
    else:
        results_html += "<p>No matching products found.</p>"

# Load and render the HTML template
with open("/path/to/form.html", "r") as f:
    html_template = f.read()

output_html = html_template.replace("<!-- SEARCH_RESULTS -->", results_html)
print(output_html)