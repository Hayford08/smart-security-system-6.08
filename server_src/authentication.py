import sqlite3

from matplotlib.style import use
from requests import NullHandler


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

def authenticate_pincode(username, pincode):
    with sqlite3.connect(database) as c:
        return c.execute("""SELECT * FROM users WHERE username = ? AND pincode = ?""", (username, pincode)).fetchone() != None

def get_credentials(username, password):
    with sqlite3.connect(database) as c:
        return c.execute("""SELECT * FROM users WHERE username = ? AND password = ?""", (username, password)).fetchone()

def retrieve_username(card_id):
    with sqlite3.connect(database) as c:
        object = c.execute("""SELECT * FROM users WHERE card_id = ?""", (card_id)).fetchone()
        if object != None:
            return object[0]
        return None
    
def get_authentication_methods(username):
    with sqlite3.connect(database) as c:
        object = c.execute("""SELECT * FROM users WHERE username = ?""", (username)).fetchone()
        return 'password=' + str(object[1] != '') + '\npincode=' + str(object[2] != '') + '\n'

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
    # Supports:
    #       ?getUsername&card_id=<card_id> => returns the username of the user
    #       ?getAuthenticationMethods&username=<username> => returns the list of authentication methods the user is using
    #                                                           as: "password=<True/False>\npincode=<True/False>\n"
    #       ?authenticate&username=<username>&password=<password> => returns true if the password matches the username
    #       ?authenticate&username=<username>&pincode=<pincode> => returns true if the pincode matches the username
    #       => returns "Unsupported Request" otherwise
    if request['method'] == 'GET':
        if 'getUsername' in request['args']:
            card_id = request['values']['card_id']
            return retrieve_username(card_id)
        elif 'getAuthenticationMethods' in request['args']:
            username = request['values']['username']
            return get_authentication_methods(username)

        elif 'authenticate' in request['args']:
            username = request['values']['username']
            if request['values']['type'] == 'password':
                password = request['values']['password']
                return authenticate_login(username, password)
            elif request['values']['type'] == 'pincode':
                pincode = request['values']['pincode']
                return authenticate_pincode(username, pincode)
            return False
    return "Unsupported Request"


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