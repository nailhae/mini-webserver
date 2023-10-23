#!/usr/bin/python3
import cgi
import sys
import os
# form = cgi.FieldStorage()

# name = "123"
# mbti = "123"
name = form.getvalue('name')
mbti = form.getvalue('mbti')
print("Content-type: text/html\r\n\r\n")
# print(form)
print("<html>")
print("<head>")
print("<meta charset=UTF-8>")
print("<title>Hello CGI</title>")
print("<html><a href='/index.html'>Home</a>")
print("<head>")
print("<h2>Hello %s %s</h2>" % (name, mbti))
print("</body>")
print("</html>")