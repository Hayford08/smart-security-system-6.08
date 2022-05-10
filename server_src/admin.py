import sqlite3
import sys

sys.path.append('/var/jail/home/team26/server_src/')
from database_request import get_all_users, get_user_door_access_input, add_door_access, remove_door_access


def request_handler(request):

    if request["method"] == "POST" and "admin" not in request["form"]:
        username = request["form"]["username"]
        remove_door_access(username)
        for elt in request["form"]:
            if elt != "username":
                action = request["form"][elt]          
                if(action == "on"):
                    add_door_access(int(elt), username)

    output =''''''
    for user in get_all_users():
        output+=f'''<b>{user[0]}</b> {get_user_door_access_input(user[0])} <br>'''
    
    return f'''<!DOCTYPE html>
        <html>
            <body>
            <h2> For your eyes only </h2>
            <h3> User Door Access </h3>
            <div>
                {output} 
            </div>
            <form method = "get" action="interfacer.py">
                <input type="submit" value="Return to home">
            </form>
        </html>
        '''