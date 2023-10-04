#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import cgi, os, sys
print("Content-type: text/html\r\n\r\n")


# form = cgi.FieldStorage()
if __name__ == "__main__":
    len = os.getenv("CONTENT_LENGTH")
    boundary = os.getenv("CONTENT_TYPE")
    query_str = sys.stdin.read()
    print(query_str)

    if boundary:
        params = boundary.split(';')
        # print(boundary)
        for param in params[1:]:
            key, split = param.split('=')

    print(split)
    filename = ''
    print(query_str)
    # if query_str:
    #     params = query_str.split()
    # if 'filename' in form:
    #     fileitem = form['filename']

    #     if fileitem.filename:
    #         fn = os.path.basename(fileitem.filename)
    #         open(os.getcwd() + '/cgi-bin/tmp/' + fn, 'wb').write(fileitem.file.read())
    #         message = '파일: ' + fn + '\n파일경로' + os.getcwd() + '/cgi-bin/tmp'
    #     else:
    #         message = '실패'
    # else:
    #     message = "'filename' 필드가 없습니다."

# print("""
# <html>
# <body>
# <p>%s</p>
# </body>
# </html>
# """ % (message))