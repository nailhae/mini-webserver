#!/usr/bin/env python3
import cgi
import sys
import os

print("Content-Type: text/html; charset=UTF-8")
print()
form = cgi.FieldStorage()

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

if value == 0 in form:
    value == 1
        
print("""
<html>
    <head>
        <meta http-equiv="content-type" content="text/html; charset=UTF-8">
        <title>print</title>
    </head>
<body>
<form method="POST" action="/cgi-bin/print.py">
<input type="hidden" name="number" value="{}">
<input type="hidden" name="input" value="{}">
<button type="submit" name="action" value="add">+</button>
<button type="submit" name="action" value="subtract">-</button>
</form>""".format(value, user_input))
print("""<p>{}</p>""".format((user_input + "<br>") * value))
print("</body></html>")
