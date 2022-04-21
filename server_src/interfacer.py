import requests
import datetime
import sqlite3
import json
import uuid
import sys

sys.path.append('/var/jail/home/team26/server_src/')
from authentication import authenticate_login, get_credentials


def get_id():
    """
    This function returns the unique id for the computer the user uses
    """
    return hash(uuid.UUID(int=uuid.getnode()))


# create session database 
session_db = '/var/jail/home/team26/test/session.db'
user_hash = get_id()

def do_post_request(url, username, password):
    raw_data = get_credentials(username, password)
    data={'username': username, 'password': password}
    #r = requests.post(url, data)
    try:
        #response_data = r.json()

        # create database for session retrieval if it's not already created
        with sqlite3.connect(session_db) as c:
            c.execute("""CREATE TABLE IF NOT EXISTS session_info (user_hash real, username text, password text);""")
            c.execute("""INSERT INTO session_info (user_hash, username, password) VALUES (?, ?, ?);""", (user_hash, username, password))

        return f'''<!DOCTYPE html>
        <html>
            <body>
            <h2> Welcome {data["username"]}</h2>
            <h3> Today's Credentials are:</h3>
            <ul>
            <li> Passcode: {raw_data[1]} </li>
            <li> Pincode: {raw_data[2]} </li>

            <br>
            <form method = "post">
                <input type="submit" value="Logout" name="logout">
            </form>

            </ul>
            </body>
        </html>
        '''
    except:
        return None

def request_handler(request):
    send_to = "https://608dev-2.net/sandbox/sc/team26/test/server.py"
    data={}
    user_hash = get_id()
    if request["method"]=="POST":
        # check if it is log out
        try:
            logout = request["form"]["logout"]
            with sqlite3.connect(session_db) as c:
                c.execute("DELETE FROM session_info WHERE user_hash = ?;""", (user_hash,))
            return login_form()
        except:
            try:
                username = data["username"] = request["form"]["username"]
                password = data["password"] = request["form"]["password"]
            except:
                return -1

            if not authenticate_login(username, password):
                return error_login_form()
            return do_post_request(send_to, username, password)

    # Request is a get
    # check if there is a session for this user
    user_info = None
    with sqlite3.connect(session_db) as c:
        c.execute("""CREATE TABLE IF NOT EXISTS session_info (user_hash real, username text, password text);""")
        user_info = c.execute("""SELECT user_hash, username, password FROM session_info WHERE user_hash = ?;""", (user_hash,)).fetchone()
            
    # Found a session 
    if user_info:
        username, password = user_info[1], user_info[2]
        return do_post_request(send_to, username, password)

    # Ask user to fill login form 
    return login_form()

def login_form():
    return f'''<!DOCTYPE html>
        <head>
        <meta charset="UTF-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1.0" />
        <title>Smart-Security-System Login</title>
        <script type="text/javascript" src="session.js"></script>
        </head>
        <html>
            <body>
            <form method = "post">
                <label for="username">Username: </label><br>
                <input type="text" id="username" name="username"><br>
                <label for="password"> Password: </label><br>
                <input type="password" id="password" name="password"><br>
                <input type="submit" value="Submit" name="submit">
            </form>
            </body>
        </html>
        '''

def error_login_form():
    return f'''<!DOCTYPE html>
        <html>
            <body>
            <p style= "color:red;"> Wrong Username-Password Combination </p>
            <form method = "post">
                <label for="username">Username: </label><br>
                <input type="text" id="username" name="username"><br>
                <label for="password"> Password: </label><br>
                <input type="password" id="password" name="password"><br>
                <input type="submit" value="Submit">
            </form>
            </body>
        </html>
        '''







"""
f'''<!DOCTYPE html>
        <html>
            <body>
            <h2> Welcome {data["username"]}</h2>
            <h3> Today's Credentials are :</h3>
            <ul>
            <li> Voice Activation Passcode : {response_data["voice_passcode"]} </li>
            <li> Gesture Passcode : {response_data["gesture_passcode"]} </li>
            <li> Pin Passcode : {response_data["pin_passcode"]} </li>

            <br>
            <form method = "post">
                <input type="submit" value="Logout" name="logout">
            </form>

            </ul>
            </body>
        </html>
        '''
"""