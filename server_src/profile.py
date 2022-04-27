import sqlite3
import sys

from server_src.authentication import update_credentials

sys.path.append('/var/jail/home/team26/server_src/')
from database_request import get_id, get_user_info_from_session


def request_handler(request):

    user_hash = get_id()
    user_info = get_user_info_from_session(user_hash)
    if user_info:
        username, password, pincode = user_info[0], user_info[1], user_info[2]
    else: # User is not logged in, unauthorized access
        return "unauthorized access"

    if request["method"] == "POST":
        
        new_password = request['form']['new_password']
        new_pincode = request['form']['pincode']
        update_credentials(username, password, {'pincode': new_pincode, 'password': new_password})
    
    return f'''<!DOCTYPE html>
        <html>
            <body>
            <h2> Welcome to your profile! </h2>
            <h3> Your current credentials are: </h3>
            <div>
                <b> Password: {password}</b>
                <b> Pincode: {pincode}</b>
            </div>
            <h3> If you want to disable a method of authentication, leave it blank. </h3>
            <div>
                <label for="Password">New Password: </label><br>
                <input type="password" id="new_password" name="new_password"><br>
                <label for="Pincode"> Pincode: </label><br>
                <input type="password" id="pincode" name="pincode"><br>
                <input type="submit" value="Update">
            </div>
        </html>
        '''