#!/usr/bin/python3
import cgi
import sys
import os
form = cgi.FieldStorage()

name = form.getvalue('name')
mbti = form.getvalue('mbti')
print(os.environ["QUERY_STRING"], file=sys.stderr)
print("Content-type: text/html\r\n\r\n")
print("<html>")
print("<head>")
print("<meta charset=UTF-8>")
print("<title>Hello CGI</title>")
print("<html>")
print("<head>")
print("<h2>Hello %s %s</h2>" % (name, mbti))
print("</body>")
print("</html>")