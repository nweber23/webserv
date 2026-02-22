#!/usr/bin/python3
import os

print("Content-Type: text/html")
print()
print("<html><body>")
print("<h1>CGI OK</h1>")
print("<pre>")
print("REQUEST_METHOD=" + os.getenv("REQUEST_METHOD", ""))
print("QUERY_STRING=" + os.getenv("QUERY_STRING", ""))
print("CONTENT_LENGTH=" + os.getenv("CONTENT_LENGTH", ""))
print("HTTP_COOKIE=" + os.getenv("HTTP_COOKIE", ""))
print("</pre>")
print("</body></html>")
