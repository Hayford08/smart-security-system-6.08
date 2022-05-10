import sqlite3
import sys

sys.path.append('/var/jail/home/team26/server_src/')
from database_request import get_id, get_user_info_from_session
from authentication import update_credentials, get_credentials


def request_handler(request):

    user_hash = get_id()
    user_info = get_user_info_from_session(user_hash)
    if user_info:
        username, password = user_info[0], user_info[1]
        
    else: # User is not logged in, unauthorized access
        return "unauthorized access"

    if request["method"] == "POST" and 'profile' not in request['form']:
        new_password = request['form']['new_password']
        new_pincode = request['form']['pincode']
        new_phrase = request['form']['voice_phrase']
        update_credentials(username, password, {'pincode': new_pincode, 'password': new_password, 'voice_phrase': new_phrase})

    data = get_credentials(username)
    password = data[1]
    pincode = data[2]
    voice_phrase = data[3]
    
    return f'''<!DOCTYPE html>
        <html>
            <body>
            <h2> Welcome to your profile! </h2>
            <h3> Your current credentials are: </h3>
            <div>
                <li> Password: {password}</li>
                <li> Pincode: {pincode}</li>
                <li> Voice Phrase: {voice_phrase}</li>
            </div>
            <h3> If you want to disable a method of authentication, leave it blank. </h3>
            <div>
                <form method="post" onsubmit="storeCredentials(localStorage.getItem('name'), document.getElementById('new_password').value">
                    <label for="Password">New Password: </label><br>
                    <input type="password" id="new_password" name="new_password"><br>
                    <label for="Pincode"> Pincode: </label><br>
                    <input type="password" id="pincode" name="pincode"><br>
                    <label for="VoicePhrase"> Voice Phrase: </label><br>
                    <input type="password" id="voice_phrase" name="voice_phrase"><br>
                    <input type="submit" value="Update", name=update>
                </form>
                <form method="post" action = "interfacer.py">
                    <input type="submit" value="Return to home", name=profile>
                </form>
            </div>
        </html>
        '''
