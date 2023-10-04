#!/usr/bin/env python3
import cgi

print("Content-type: text/html\r\n\r\n")

form = cgi.FieldStorage()
user_input = form.getvalue("input")

filename = "cgi-bin/test.html"

try:
    with open(filename, 'r') as f:
        lines = f.readlines()
except Exception as e:
    print(f" {e}")

value = 0
for line in lines:
    if "current value:" in line:
        try:
            value_str = line.split(":", 1)[1].strip()
            value = int(value_str)
        except (IndexError, ValueError):
            pass

if "increment" in form:
    value += 1
elif "decrement" in form:
    if value <= 0:
        value == 0  
    else:
        value -= 1

with open(filename, 'w') as f:
    f.write("<!DOCTYPE HTML>\n")
    f.write("<HTML><H1>Print</H1>")
    f.write("current value: " + str(value) + '\n')
    Max = value
    f.write('<form method="POST">\n')
    f.write('<input type="submit" name="increment" value="+">\n')
    f.write('<input type="submit" name="decrement" value="-">\n')
    f.write('</form>')
    for i in range(Max):
        f.write("<H2>Hello</H2>\n")
    f.write("</HTML>")

with open(filename, 'r') as f:
    print(f.read())

