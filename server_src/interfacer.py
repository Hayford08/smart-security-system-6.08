import requests
import datetime
import sqlite3
import json
import uuid
import sys

sys.path.append('/var/jail/home/team26/server_src/')
from authentication import authenticate_login, get_credentials
from database_request import get_user_door_access, check_admin
from forms import login_form, error_login_form

# session database
session_db = '/var/jail/home/team26/server_src/session.db'

def do_post_request(username, password, message_to_display=None):
    raw_data = get_credentials(username)
    data={'username': username, 'password': password}
    try:
        # Doors this users have access 
        user_door_access = get_user_door_access(username)

        output = f'''<!DOCTYPE html>
        <html>
        <head><script type="text/javascript" src="session.js"></script></head>
        <body onload="storeCredentials('{username}', '{password}')">'''

        admin_button = ''
        if check_admin(username):
            admin_button = '''<br>
                <form method="post" action = "admin.py">
                <input type="submit" value="Click to enter Admin Mode", name=admin>
                </form>
                <br>'''

        if message_to_display:
            output += f'''<p style= "color:red;"> {message_to_display}</p>'''
        output += f'''<h2> Welcome {data["username"]}</h2>
            <h3> Door(s) you have access to: </h3> 
            <ul>
                {user_door_access}
            </ul>
            <h3> Today's Credentials are:</h3>

            <li> Passcode: {raw_data[1]} </li>
            <li> Pincode: {raw_data[2]} </li> 
            <li> Voice Phrase: {raw_data[3]} </li> 
            <li> Gesture Combinations: {raw_data[6]} </li> 
            <br>
            <br><br>
            <form method="post" action = "profile.py">
            <input type="text" name="username" value={data["username"]} hidden/>
            <input type="text" name="password" value={data["password"]} hidden/>
            <input type="submit" value="Change Profile", name=profile>
            </form>
            {admin_button}
            <form method = "post">
                <input type="submit" value="Logout" name="logout">
            </form> 
            </ul>
            </body>
        </html>
        '''
        return output
    except:
        return None

def request_handler(request):
    data={}
    if request["method"] == "POST" and not 'profile' in request['form']:
        # check if it is log out
        if 'logout' in request['form']:
            # delete_user_session(user_hash)
            return login_form(delete_session=True)
        else:
            # if not user_info: # Ignore login requests when the user is already logged in
            username = data["username"] = request["form"]["username"]
            password = data["password"] = request["form"]["password"]
            if not authenticate_login(username, password):
                return error_login_form()
            return do_post_request(username, password)

    # Ask user to fill login form
    return login_form(delete_session=False)
