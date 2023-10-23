#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import cgi, os, sys
print("Content-type: text/html\r\n\r\n")

form = cgi.FieldStorage()
# A nested FieldStorage instance holds the file
fileitem = form['filename']
# print(form)
# print(fileitem)
# Test if the file was uploaded
if fileitem.filename:
    # strip leading path from file name
    # to avoid directory traversal attacks
    fn = os.path.basename(fileitem.filename)
    try :
        open(os.getcwd() + '/assets/html/tmp/' + fn, 'wb').write(fileitem.file.read())
        message = 'The file [' + fn + '] was uploaded successfully'
    except Exception as e:
        message = 'Error occurred: can not open file'

else:
    message = 'Error occurred: can not open file'

print ("""\
<html>
    <a href='/index.html'>Home</a>
<body>
<h2>%s</h2>
</body></html>
""" % (message,))


# # form = cgi.FieldStorage()
# if __name__ == "__main__":
#     len = os.getenv("CONTENT_LENGTH")
#     boundary = os.getenv("CONTENT_TYPE")
#     query_str = sys.stdin.read()
#     # print(query_str)

#     if boundary:
#         params = boundary.split(';')
#         for param in params[1:]:
#             key, split = param.split('=')

#     # print(split)
#     # print("|||||||||||||||||||")
#     filename = ''
#     # print(query_str)
#     # print()

# #     if query_str:
# #         params_1 = query_str.split(split)
# #         for param_1 in params_1:
# #             key_1 = param_1.split(';')
# #             # for param_2 in key_1:
# #                 # print(os.getcwd())
# #                 # print(param_2)
# #                 # key_2, value = param_2.split('=')
# #                 # if key_2 == 'filename':
# #                 #     filename = value
# #     filename = '07.jpg'
# #     if filename:
# #         if filename:
# #             # fn = os.path.basename(filename)
# #             fn = filename
# #             print(fn + '------||||||||||||')
# #             open(os.getcwd() + '/cgi-bin/tmp/' + fn, 'wb').write(filename.file.read())
# #             message = '파일: ' + fn + '\n파일경로' + os.getcwd() + '/cgi-bin/tmp'
# #         else:
# #             message = '실패'
# #     else:
# #         message = "'filename' 필드가 없습니다."

# # print("""
# # <html>
# # <body>
# # <p>%s</p>
# # </body>
# # </html>
# # """ % (message))