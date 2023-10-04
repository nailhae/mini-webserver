#!/usr/bin/env python3

import cgi, os

form = cgi.FieldStorage()

if 'filename' in form:
    fileitem = form['filename']

    if fileitem.filename:
        fn = os.path.basename(fileitem.filename)
        open(os.getcwd() + '/cgi-bin/tmp/' + fn, 'wb').write(fileitem.file.read())
        message = '파일: ' + fn + '\n파일경로' + os.getcwd() + '/cgi-bin/tmp'
    else:
        message = '실패'
else:
    message = "'filename' 필드가 없습니다."


print("Content-Type: text/html;charset=utf-8\r\n")

print("""
<html>
<body>
<p>%s</p>
</body>
</html>
""" % (message))