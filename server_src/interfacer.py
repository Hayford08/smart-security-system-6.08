import requests
import datetime
import sqlite3
import json
import uuid
import sys

sys.path.append('/var/jail/home/team26/server_src/')
from authentication import authenticate_login, get_credentials, update_passcodes
from database_request import get_id, get_user_door_access, get_user_info_from_session, create_user_session, delete_user_session
from forms import login_form, error_login_form

# session database
session_db = '/var/jail/home/team26/server_src/session.db'

def do_post_request(username, password, message_to_display=None):
    user_hash = get_id()
    raw_data = get_credentials(username, password)
    data={'username': username, 'password': password}
    try:
        # Create a session for current log in 
        create_user_session(user_hash, username, password)

        # Doors this users have access 
        user_door_access = get_user_door_access(username)

        output = f'''<!DOCTYPE html>
        <html>
        <body>'''

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

            <br>
            <form method = "post">
                <label for="new_password">Enter new passcode: </label><br>
                <input type="text" id="new_password" name="new_password"><br>
                <input type="submit" value="Submit" name="submit_new_password">
                <br><br>
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
    user_hash = get_id()
    if request["method"]=="POST":
        # check if it is log out
        try:
            logout = request["form"]["logout"]
            # remove user session
            delete_user_session(user_hash)
            return login_form()
        except:
            try:
                # check if it is a change password
                new_password = request['form']['new_password']
                username, password = get_user_info_from_session(user_hash)
                data = {"pincode": new_password}
                change_message = update_passcodes(username, password, data)
                return do_post_request(username, password, change_message)
            except:
                # User just logged in
                try:
                    username = data["username"] = request["form"]["username"]
                    password = data["password"] = request["form"]["password"]
                except:
                    return -1

            if not authenticate_login(username, password):
                return error_login_form()
            return do_post_request(username, password)

    # Request is a get
    # check if there is a session for this user
    user_info = get_user_info_from_session()

    # Found a session
    if user_info:
        username, password = user_info[0], user_info[1]
        return do_post_request(username, password)

    # Ask user to fill login form
    return login_form()
