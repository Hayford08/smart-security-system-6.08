import sqlite3


database = '/var/jail/home/team26/server_src/database.db'
def setup():
    """
    This is the function to create the default usernames, passwords, pincodes, and is_admin in the database
    """
    values = [('dkriezis', '8130070', '542071', 0), ('hayford', '5317558', '798333', 0), ("Admin", "Admin", "470236", 1),
        ('vladap', '9466123', '670426', 0), ('muhender', '4616833', '057548', 0), ('mazaheri','8150042', '113312', 0)]
    with sqlite3.connect(database) as c:
        c.execute("""CREATE TABLE IF NOT EXISTS users (username text, password text, pincode text, is_admin integer);""")
        for tup in values:
            c.execute("""INSERT INTO users (username, password, pincode, is_admin) VALUES (?,?,?,?)""", (tup[0], tup[1], tup[2], tup[3]))

def authenticate_login(username, password):
    with sqlite3.connect(database) as c:
        return c.execute("""SELECT * FROM users WHERE username = ? AND password = ?""", (username, password)).fetchone() != None

def get_credentials(username, password):
    with sqlite3.connect(database) as c:
        return c.execute("""SELECT * FROM users WHERE username = ? AND password = ?""", (username, password)).fetchone()

def update_passcodes(username, password, data):
    # data is a dictionary. For now just write data = {"pincode": actual_pincode_value}
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


#TODO -- Next week???
# def update_isadmin(username, value):
#     """
#     This function updates the user's admin status
#     """
#     with sqlite3.connect(database) as c:
#         object = c.execute("""SELECT * FROM users WHERE username = ? """, (username,)).fetchone()
#         c.execute("""UPDATE users SET pincode = ? WHERE username = ? AND password = ?""", (object[], username, password)).fetchone()
#     return "Password Updated Successfully"

# setup()