import requests
import sys
import datetime
import sqlite3
import json

sys.path.append('/var/jail/home/team26/project/')
import authentication

def request_handler(request):
    if request["method"] == "POST":
        username, password = request["form"]["username"], request["form"]["password"]
        if not authentication.authenticate_login(username, password):
            return "Error"
        return str(request) + f'''<!DOCTYPE html>
                <html>
                    <body>
                    <h2> Welcome {username}</h2>
                    </body>
                </html>
                '''
    return "Error"