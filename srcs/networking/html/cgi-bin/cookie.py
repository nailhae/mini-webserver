#!/usr/bin/env python3
from http import cookies
import cgi
import os

form = cgi.FieldStorage()
name = form.getvalue("name")
value = form.getvalue("value")

cookie = cookies.SimpleCookie()
cookie[name] = value
cookie[name]['path'] = '/'

def show_cookie(c):
    for a, morsel in c.items():
        print(a,':', morsel.value,'<br>')

print("Content-type: text/html; charset=UTF-8")
print(cookie.output())
print()

print("<a href='/index.html'>Home</a><br>")
if 'HTTP_COOKIE' in os.environ: 
    cookie.load(os.environ["HTTP_COOKIE"])
    for name, key in cookie.items():
        print("name: ",name,' : ',"key: ",key.value, '<br>' )
else:
    print("실패 ㅜㅜ")