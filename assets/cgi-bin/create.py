#!/usr/local/bin/python3
print("content-type:text/html; charset=UTF-8\n")
import cgi,os

# files = os.listdir('./html')
files = os.listdir('./assets/cgi-bin')
liststr = ''
for item in files:
    liststr = liststr + '<li><a href="/{name}">{name}</a></li>'.format(name=item)

form = cgi.FieldStorage()
if 'name' in form:
    pageId = form["name"].value
    description = open(os.getcwd() + '/assets/cgi-bin/' + pageId).read()
else:
    pageId = 'Welcome'
    description = 'Hello Webserv'

print('''<!doctype html>
<html>
<head>
  <title>WEB1 - Welcome</title>
  <meta charset="utf-8">
</head>
<body>
  <h1><a href="/index.html">WEB</a></h1>
  <ol>{listStr}</ol>
  <form action="process_create.py" method="post">
    <p><input type="text" name="title" placeholder="title"></p>
    <p><textarea name="description" placeholder="description"></textarea></p>
    <p><input type="submit"></p>
  </form>
</body>
</html>
'''.format(title=pageId, desc=description, listStr=liststr))