import sqlite3

gestures_db = '/var/jail/home/team26/server_src/gestures.db'

def request_handler(request):
    with sqlite3.connect(gestures_db) as c:
        c.execute("""SELECT """)