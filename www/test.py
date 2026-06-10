#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html\n")
print("<html>")
print("<head><title>CGI Test</title></head>")
print("<body>")
print("<h1>✓ CGI работает!</h1>")
print("<p>Метод запроса: " + os.environ.get('REQUEST_METHOD', 'UNKNOWN') + "</p>")
print("<p>Query String: " + os.environ.get('QUERY_STRING', 'пусто') + "</p>")
print("<p>Content-Length: " + os.environ.get('CONTENT_LENGTH', '0') + "</p>")
print("</body>")
print("</html>")
