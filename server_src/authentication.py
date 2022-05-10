import sqlite3

database = '/var/jail/home/team26/server_src/database.db'
door_access_db = '/var/jail/home/team26/server_src/door_access.db'
def setup():
    """
    This is the function to create the default usernames, passwords, pincodes, voice_phrase, and is_admin in the database
    """
    values = [('dkriezis', '8130070', '542071', 'banana', 0, 'E0 16 99 1B', 'left,right,up'), ('hayford', '5317558', '798333', 'orange', 0, '73 25 A1 31', 'left,up,down'), 
              ("Admin", "Admin", "470236", 'puppy', 1, 'AC EC 64 68', 'left'),  ('vladap', '9466123', '670426', 'rug', 0, '63 E1 B3 31', 'left,right,left'), 
              ('muhender', '4616833', '057548', 'north', 0, 'D3 CC 5F 6D', 'up,down,left'), ('mazaheri','8150042', '113312', 'cat', 0, '06 A4 40 B7', 'right,down,up')]
    with sqlite3.connect(database) as c:
        c.execute("""DROP TABLE users""")
        c.execute("""CREATE TABLE IF NOT EXISTS users (username text, password text, pincode text, voice_phrase text, is_admin integer, card_id text, gesture_password text);""")
        for tup in values:
            c.execute("""INSERT INTO users (username, password, pincode, voice_phrase, is_admin, card_id, gesture_password) VALUES (?,?,?,?,?,?,?)""", (tup[0], tup[1], tup[2], tup[3], tup[4], tup[5], tup[6]))

def authenticate_login(username, password):
    with sqlite3.connect(database) as c:
        return c.execute("""SELECT * FROM users WHERE username = ? AND password = ?""", (username, password)).fetchone() != None

def authenticate_pincode(username, pincode):
    with sqlite3.connect(database) as c:
        return c.execute("""SELECT * FROM users WHERE username = ? AND pincode = ?""", (username, pincode)).fetchone() != None

def authenticate_gestures(username, gesture):
    with sqlite3.connect(database) as c:
        return c.execute("""SELECT * from users WHERE username = ? AND gesture_password = ? """, (username, gesture)).fetchone() != None
        
def authenticate_voice_phrase(username, voice_phrase):
    with sqlite3.connect(database) as c:
        return c.execute("""SELECT * FROM users WHERE username = ? AND voice_phrase = ?""", (username, voice_phrase)).fetchone() != None

def get_credentials(username):
    with sqlite3.connect(database) as c:
        return c.execute("""SELECT * FROM users WHERE username = ?""", (username,)).fetchone()

def retrieve_username(card_id):
    with sqlite3.connect(database) as c:
        object = c.execute("""SELECT * FROM users WHERE card_id = ?""", (card_id,)).fetchone()
        if object != None:
            return object[0]
        return None
    
def get_authentication_methods(username):
    with sqlite3.connect(database) as c:
        object = c.execute("""SELECT * FROM users WHERE username = ?""", (username,)).fetchone()
        return 'password=' + str(object[1] != '') + '\npincode=' + str(object[2] != '') + '\nvoice=' + str(object[3] != '') + '\n'

def checkAccess(username, door_id):
    with sqlite3.connect(database) as c:
        c.execute("""SELECT * FROM door_user_table WHERE username = ? AND door_id = ?""", (username, door_id)).fetchone() != None
        

def update_credentials(username, password, data):
    # data is a dictionary. For now just write data = {"pincode": actual_pincode_value}
    with sqlite3.connect(database) as c:
        object = c.execute("""SELECT * FROM users WHERE username = ? AND password = ?""", (username, password)).fetchone()
        if "pincode" not in data:
            data["pincode"] = object[2]
        if "voice_phrase" not in data:
            data["voice_phrase"] = object[3]
        c.execute("""UPDATE users SET voice_phrase = ? WHERE username = ? AND password = ?""", (data['voice_phrase'], username, password)).fetchone()
        c.execute("""UPDATE users SET pincode = ? WHERE username = ? AND password = ?""", (data['pincode'], username, password)).fetchone()
        c.execute("""UPDATE users SET password = ? WHERE username = ? AND password = ?""", (data['password'], username, password)).fetchone()
    return "Password Updated Successfully"

def request_handler(request):
    # Supports:
    #       ?checkAccess&username=<username>&door_id=<door_id> => returns True if the user has access to the door, False otherwise
    #       ?checkAccess&card_id=<card_id>&door_id=<door_id> => returns True if the user with the given card_id has access to the door, False otherwise

    #       ?getUsername&card_id=<card_id> => returns the username of the user

    #       ?getAuthenticationMethods&username=<username> => returns the list of authentication methods the user is using
    #                                                           as: "password=<True/False>\npincode=<True/False>\n"

    #       ?authenticate&username=<username>&password=<password> => returns true if the password matches the username
    #       ?authenticate&username=<username>&pincode=<pincode> => returns true if the pincode matches the username
    #       ?authenticate&username=<username>&voice_phrase=<voice_phrase> => returns true if the voice phrase matches the username
    #       => returns "Unsupported Request" otherwise
    if request['method'] == 'GET':
        if 'checkAccess' in request['args']:
            door_id = request['values']['door_id']
            if 'username' in request['values']:
                username = request['values']['username']
            elif 'card_id' in request['values']:
                username = retrieve_username(request['values']['card_id'])
            return checkAccess(username, door_id)

        elif 'getUsername' in request['args']:
            card_id = request['values']['card_id']
            return retrieve_username(card_id)
        elif 'getAuthenticationMethods' in request['args']:
            username = request['values']['username']
            return get_authentication_methods(username)

        elif 'authenticate' in request['args']:
            username = request['values']['username']
            if 'password' in request['values']:
                password = request['values']['password']
                return authenticate_login(username, password)
            elif 'pincode' in request['values']:
                pincode = request['values']['pincode']
                return authenticate_pincode(username, pincode)
            elif 'voice_phrase' in request['values']:
                voice_phrase = request['values']['voice_phrase']
                return authenticate_voice_phrase(username, voice_phrase)
            return False
        
    if request["method"] == "POST":
        if 'authenticate_gesture' in request['form']:          
            return authenticate_gestures(request['form']['username'], request['form']['gesture_sequence'][:-1])
            

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

#setup()
