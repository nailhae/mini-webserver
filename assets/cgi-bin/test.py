import cgi
import sys
import os


content_length = int(os.environ.get("CONTENT_LENGTH", 10))
request_body = sys.stdin.read(content_length)

form = cgi.parse_qs(request_body)

if "increment" in form or "decrement" in form:
    print("Status: 303 See other")
    print("Location: /test.html")
    print()
else:
    print("Content-Type: text/html")
    print()
    print("""
        <form method='POST'>
            <input type='submit' name='increment' value='+'>
            <input type='submit' name='decrement' value='-'>
        </form>
        """)

