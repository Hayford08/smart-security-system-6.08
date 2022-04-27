import sqlite3
import sys

sys.path.append('/var/jail/home/team26/server_src/')
from database_request import get_id, get_all_users, get_user_door_access_input, add_door_access, remove_door_access, get_user_info_from_session, check_admin


def request_handler(request):

    user_hash = get_id()
    user_info = get_user_info_from_session(user_hash)
    if user_info:
        username, password = user_info[0], user_info[1]
        if not check_admin(username): # User does not have admin privileges, unauthorized access
            return "unauthorized access"
    else: # User is not logged in, unauthorized access
        return "unauthorized access"

    if request["method"] == "POST" and "admin" not in request["form"]:
        username = request["form"]["username"]
        remove_door_access(username)
        for elt in request["form"]:
            if elt != "username":
                action = request["form"][elt]          
                if(action == "on"):
                    add_door_access(int(elt), username)

    output ='''''';
    for user in get_all_users():
        output+=f''' <b>{user[0]}</b> {get_user_door_access_input(user[0])} <br>'''
    
    return f'''<!DOCTYPE html>
        <html>
            <body>
            <h2> For your eyes only </h2>
            <h3> User Door Acess </h3>
            <div>
                {output} 
            </div>
        </html>
        '''