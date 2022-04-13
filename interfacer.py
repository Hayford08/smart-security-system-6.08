import requests
import datetime
import json

def request_handler(request):
    logged_in = False

    if not logged_in:
        if request["method"]=="POST":
            send_to = "https://608dev-2.net/sandbox/sc/team26/server.py"
            data = {}
            try:
                data["username"] = request["form"]["username"]
                data["password"] = request["form"]["password"]
            except:
                return -1
            r = requests.post(send_to, data)
            try:
                response_data = r.json()
                return f'''<!DOCTYPE html>
                <html>
                    <body>
                    <h2> Welcome {data["username"]}</h2>
                    <h3> Today's Credentials are :</h3>
                    <ul>
                    <li> Voice Activation Passcode : {response_data["voice_passcode"]} </li>
                    <li> Gesture Passcode : {response_data["gesture_passcode"]} </li>
                    <li> Pin Passcode : {response_data["pin_passcode"]} </li>
                    </ul>
                    </body>
                </html>
                '''
            except:
                return error_login_form()

        return login_form()

def login_form():
    return f'''<!DOCTYPE html>
        <html>
            <body>
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