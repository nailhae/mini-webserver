#!/usr/local/bin/python3
print("content-type:text/html; charset=UTF-8\r\n")
import cgi,os

# files = os.listdir('./html')
files = os.listdir('./assets/html')
liststr = ''
count = 0

for item in files:
    if item.endswith(".html"):
      liststr = liststr + '<li ><a href="/{name}">{name}</a></li>'.format(name=item)
    # count += 1
    # if count >= 8:
    #     break

form = cgi.FieldStorage()
if 'name' in form:
    pageId = form["name"].value
    description = open(os.getcwd() + '/assets/html/' + pageId).read()
else:
    pageId = 'Welcome'
    description = 'Hello Webserv'

print('''<!doctype html>
<html>
<head>
  <title>WEB1 - Welcome</title>
  <meta charset="utf-8">
</head>
<body style="max-height: 500rem; overflow: auto; color: red">
  <h1><a href="/index.html">WEB</a></h1>
  <ol >{listStr}</ol>
  <form action="process_create.py" method="post">
    <p><input type="text" name="title" placeholder="title"></p>
    <p><textarea name="description" placeholder="description"></textarea></p>
    <p><input type="submit"></p>
  </form>
</body>
</html>
'''.format(listStr=liststr))