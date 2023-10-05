#!/usr/local/bin/python3
print("content-type:text/html; charset=UTF-8\n")

import cgi, os
form = cgi.FieldStorage()

if 'name' in form:
    pageId = form["name"].value
    # description = open('/cgi-bin/' + pageId).read()
    description = open(os.getcwd() + '/cgi-bin/' + pageId).read()
else:
    pageId = 'Welcome'
    description = 'Hello Python'

print('''<!doctype html>
<html>
<head>
  <title>WEB1 - Welcome</title>
  <meta charset="utf-8">
</head>
<body>
  <h1><a href="index.html">WEB</a></h1>
  <ol>
    <li><a href="fileprint.html">fileprint</a></li>
    <li><a href="print.html">print</a></li>
    <li><a href="fileupload.html">fileupload</a></li>
  </ol>
  <h2>{title}</h2>
  <p>{desc}</p>
</body>
</html>
'''.format(title=pageId, desc=description))