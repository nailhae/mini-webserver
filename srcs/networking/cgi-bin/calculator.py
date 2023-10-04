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
        try:
            m_int = int(m)
            n_int = int(n)
            result_string = 'String from browser: {}'.format(m_int * n_int)
        except ValueError:
            result_string = 'Invalid input'

        html_text += f'<p>{result_string}</p>'

    html_text += '</body>\n</html>\n'

    print (html_text)
