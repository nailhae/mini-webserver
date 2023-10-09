#!/usr/local/bin/python3
print("content-type:text/html; charset=UTF-8\n")

import cgi, os
form = cgi.FieldStorage()
print(form)

# if 'name' in form:
#     # pageId = form['name'].value
#     # description = open('/cgi-bin/' + pageId).read()
#     pageId = 'test.html'
#     description = 'Hello CGI'
#     print(pageId)
#     try:
#       description = open(os.getcwd() + '/cgi-bin/' + pageId).read()
#     except IOError as e:
#       description = 'Hello CGI'
# else:
#     pageId = 'Welcome'
#     description = 'Hello CGI'
pageId = 'Welcome'
description = 'Hello CGI'

print('''<!doctype html>
<html>
<head>
  <title>WEB1 - Welcome</title>
  <meta charset="utf-8">
</head>
<body>
  <h1><a href="/index.html">WEB</a></h1>
  <ol>
    <li><a href="/fileprint.html">fileprint</a></li>
    <li><a href="/print.html">print</a></li>
    <li><a href="/fileupload.html">fileupload</a></li>
  </ol>
  <h2>{title}</h2>
  <p>{desc}</p>
</body>
</html>
'''.format(title=pageId, desc=description))