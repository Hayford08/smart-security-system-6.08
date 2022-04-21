import sqlite3


database = '/var/jail/home/team26/server_src/database.db'

def setup():
    with sqlite3.connect(database) as c:
        c.execute("""CREATE TABLE IF NOT EXISTS users (username text, password text, pincode text);""")
        c.execute("""INSERT INTO users (username, password, pincode) VALUES (?,?,?)""", ("Admin", "Admin", "0000"))

def authenticate_login(username, password):
    with sqlite3.connect(database) as c:
        return c.execute("""SELECT * FROM users WHERE username = ? AND password = ?""", (username, password)).fetchone() != None
setup()

def get_credentials(username, password):
    with sqlite3.connect(database) as c:
        return c.execute("""SELECT * FROM users WHERE username = ? AND password = ?""", (username, password)).fetchone()

def update_passcodes(username, password, data):
    if not authenticate_login(username, password):
        return "Failed"
    with sqlite3.connect(database) as c:
        object = c.execute("""SELECT * FROM users WHERE username = ? AND password = ?""", (username, password)).fetchone()
        if "pincode" not in data:
            data["pincode"] = object[2]
        c.execute("""UPDATE users SET pincode = ? WHERE username = ? AND password = ?""", (data["pincode"], username, password)).fetchone()
    return "Password Updated Successfully"

def request_handler(request):
    if request['method'] == 'GET':
        try:
            username = request['values']['username']
            password = request['values']['password']
            return authenticate_login(username, password)
        except:
            pass
    return False