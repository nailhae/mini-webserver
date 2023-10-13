#!/usr/bin/env python
import cgi

# 표준 출력 헤더 설정
print("Content-type: text/html\n")

# HTML 문서 시작
print('''<!DOCTYPE html>
<head>
    <meta charset="UTF-8">
    <title>Webserv Test</title>
</head>
<body>
  <h2><a href="/index.html">home</a></h2>''')

form = cgi.FieldStorage()

name = form.getvalue("name", "Not Provided")
nickname = form.getvalue("nickname", "Not Provided")

print("<p>hello {}</p>".format(name))
print("<p>aka {}</p>".format(nickname))

print("</body>")
print("</html>")
