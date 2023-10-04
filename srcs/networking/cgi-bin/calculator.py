#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

print("Content-type: text/html\r\n\r\n")

if __name__ == "__main__":
    len = os.getenv("CONTENT_LENGTH")
    query_str = sys.stdin.read()

    m = ''
    n = ''
    if query_str:
        params = query_str.split('&')
        for param in params:
            key, value = param.split('=')
            if key == 'm':
                m = value
            elif key == 'n':
                n = value
    html_text = '<!DOCTYPE html>\n<html>\n<head>\n'
    html_text += '\t<title>'+'test'+'</title>\n'
    html_text += '\t<meta charset="utf-8">\n'
    html_text += '</head>\n\n'
    html_text += '<body>\n'

    html_text += '<h3>Hello world by python cgi</h3>'

    if m and n:
        html_text += f'String from browser: {m * n}'

    html_text += '</body>\n</html>\n'

    print (html_text)
