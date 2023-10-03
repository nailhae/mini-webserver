#!/usr/bin/env python3
import cgi

print("Content-Type: text/html")
print()

form = cgi.FieldStorage()

# Default value is 5
value = 5

if "number" in form:
    try:
        value = int(form.getvalue("number"))
    except ValueError:
        pass

if "action" in form:
    if form.getvalue("action") == "add":
        value += 1
    elif form.getvalue("action") == "subtract":
        value -= 1

# if value == 0 in form:
#     value == 1
        
print("""
<html>
    <head>
        <meta http-equiv="content-type" content="text/html; charset=UTF-8">
        <title>print</title>
    </head>
<body>
<form method="POST" action="/cgi-bin/print.py">
<input type="hidden" name="number" value="{}">
<button type="submit" name="action" value="add">+</button>
<button type="submit" name="action" value="subtract">-</button>
</form>
<p>{}</p>
</body>
</html>""".format(value, 'webservㅁㄴㅇㄹㅁㄴ '*value))