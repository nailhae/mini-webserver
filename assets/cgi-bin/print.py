#!/usr/bin/env python3
import cgi
import sys
import os

print("Content-Type: text/html; charset=UTF-8")
print()
form = cgi.FieldStorage()

# while True:
#     print()
value = 5
user_input = "Hello"
if "number" in form:
    try:
        value = int(form.getvalue("number"))
    except ValueError:
        pass

if "input" in form:
    try:
        user_input = form.getvalue("input")
        
    except ValueError:
        pass

if "action" in form:
    if form.getvalue("action") == "add":
        value += 1
    elif form.getvalue("action") == "subtract":
        value -= 1

if value <= 0:
    value = 0
        
print("""
<html>
    <head>
        <meta http-equiv="content-type" content="text/html; charset=UTF-8">
        <title>print</title>
        <a style="font-size:30; Color: skyblue" href="/index.html">home</a>
    </head>
<body>
<form method="POST" action="/cgi-bin/print.py">
</br>
<input type="hidden" name="number" value="{}">
<input type="hidden" name="input" value="{}">
<button style="width: 100px; height: 50px;border: 3px solid skyblue;background-color: rgba(0,0,0,0);" type="submit" name="action" value="add">+</button>
<button style="width: 100px; height: 50px;border: 3px solid skyblue;background-color: rgba(0,0,0,0);" type="submit" name="action" value="subtract">-</button>
</form>""".format(value, user_input))
print("<p>{}</p>".format(form))
print("""<p>{}</p>""".format((user_input + "<br>") * value))
print("</body></html>")
