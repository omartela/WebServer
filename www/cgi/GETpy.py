import cgi

# Output the headers
print("Content-Type: text/html", end="")  # Content-Type header
print("\r\n\r\n", end="")  # Blank line indicating the end of headers

# Start the HTML structure
print("<!DOCTYPE html>")
print("<html lang='en'>")
print("<head>")
print("<meta charset='UTF-8'>")
print("<meta name='viewport' content='width=device-width, initial-scale=1.0'>")
print("<title>Greetings</title>")

# Adding some CSS for the red theme
print("<style>")
print("body { font-family: Arial, sans-serif; background-color: #ffebee; color: #d32f2f; text-align: center; padding: 20px; margin: 0; }")
print("h1 { color: #b71c1c; font-size: 3em; }")
print("p { font-size: 1.5em; margin-top: 20px; }")
print("footer { font-size: 1em; color: #555; margin-top: 40px; }")
print("button { font-size: 1.2em; padding: 10px 20px; background-color: #d32f2f; color: white; border: none; border-radius: 5px; cursor: pointer; }")
print("button:hover { background-color: #c62828; }")
print("</style>")

print("</head>")
print("<body>")

# Handling the form data
form = cgi.FieldStorage()

# Retrieve values from the form
fname = form.getvalue('fname', 'Guest')  # Default value is 'Guest' if not provided
lname = form.getvalue('lname', 'User')  # Default value is 'User' if not provided

# Display a friendly greeting
print("<h1>Hello, " + fname + " " + lname + "!</h1>")
print("<p>Welcome to our page. We're glad to see you here!</p>")

# Optional: Add a footer
print("<footer>CGI Script Example - Python</footer>")

# End HTML
print("</body>")
print("</html>")