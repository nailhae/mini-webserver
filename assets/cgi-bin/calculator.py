#!/usr/bin/env python3
import cgi
import os
import sys

print("Content-Type: text/html; charset=UTF-8")
print()
form = cgi.FieldStorage()

# len = os.getenv("CONTENT_LENGTH")
# print(len)
# query_str = sys.stdin.read()
# print(query_str)
m = form.getvalue("m")
n = form.getvalue("n")
# if query_str:
#     params = query_str.split('&')
#     for param in params:
#         key, value = param.split('=')
#         if key == 'm':
#             m = value
#         elif key == 'n':
#             n = value
html_text = "<!DOCTYPE html>\n<html>\n<head>\n"
html_text += '\t<title>'+'test'+'</title>\n'
html_text += "\t<meta charset='utf-8'>\n<a href='/index.html'>Home</a>"
html_text += '</head>\n\n'
html_text += '<body>\n'
html_text += '<h3>Hello world by cgi</h3>'
if m and n:
    try:
        m_int = int(m)
        n_int = int(n)
        result_string = 'String from browser: {}'.format(m_int * n_int)
    except ValueError:
        result_string = 'Invalid input'
    html_text += f'<p>{result_string}</p>'
html_text += '</body>\n</html>\n'
print (html_text)