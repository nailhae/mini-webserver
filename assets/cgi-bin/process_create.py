#!/usr/local/bin/python3

import cgi, os

form = cgi.FieldStorage()

title = form["title"].value
description = form["description"].value

opened_file = open(os.getcwd() + '/assets/html/' + title, 'w')
opened_file.write(description)

# Redirection
# print("Location: /index.html" + title)
print("Location: /index.html")
print("\r\n\r\n")