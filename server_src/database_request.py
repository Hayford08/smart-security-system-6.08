import requests
import datetime
import sqlite3
import json
import uuid
import sys

# session database
session_db = '/var/jail/home/team26/server_src/session.db'

# door access database
door_access_db = '/var/jail/home/team26/server_src/door_access.db'

# users database
database = '/var/jail/home/team26/server_src/database.db'

def get_id():
    """
    This function returns the unique id for the computer the user uses
    """
    return hash(uuid.UUID(int=uuid.getnode()))


def get_user_info_from_session(user_hash):
    """
    This function returns the user's username and password given the user hash
    """
    user_info = None
    with sqlite3.connect(session_db) as c:
        c.execute("""CREATE TABLE IF NOT EXISTS session_info (user_hash real, username text, password text);""")
        user_info = c.execute("""SELECT username, password FROM session_info WHERE user_hash = ?;""", (user_hash,)).fetchone()
    return user_info

def get_user_door_access(username):
    """
    This function takes in a username and returns all the doors that the user has access to
    as an html list type
    """
    door_data = []
    with sqlite3.connect(door_access_db) as c:
        door_data = c.execute('''SELECT door_id FROM door_user_table WHERE username = ?;''', (username,)).fetchall()

    door_list = ""
    for door_id in door_data:
        door_list += "<li> Door " + str(door_id[0]) + "</li>"
    return door_list

def get_user_door_access_input(username):
    """
    This function takes in a username and returns all the doors that the user has access to as
    an html checkbox type
    """
    door_data = []
    with sqlite3.connect(door_access_db) as c:
        door_data = c.execute('''SELECT door_id FROM door_user_table WHERE username = ?;''', (username,)).fetchall()
    
    allowed_doors = set()
    for door_id in door_data:
        allowed_doors.add(door_id[0])
    
    door_list = f'''<form method = "post" action="admin.py"> <input name = 'username' type = "hidden" value ={username}>'''
    for door_id in get_all_doors():
        if door_id[0] in allowed_doors:
            door_list += f'''<div><input type="checkbox" id={str(door_id[0])} name = {str(door_id[0])} checked> Door {str(door_id[0])} </label></div> '''
        else:
            door_list += f'''<div><input type="checkbox" id={str(door_id[0])} name = {str(door_id[0])}> Door {str(door_id[0])} </label></div> '''
    door_list += f'''<input type="submit" value="Update {username}'s access">'''
    door_list += '''</form>'''
    return door_list

def create_user_session(user_hash, username, password):
    # create database for session retrieval if it's not already created
    with sqlite3.connect(session_db) as c:
        c.execute("""CREATE TABLE IF NOT EXISTS session_info (user_hash real, username text, password text);""")
        c.execute("""INSERT INTO session_info (user_hash, username, password) VALUES (?, ?, ?);""", (user_hash, username, password))


def delete_user_session(user_hash):
    """
    This function removes the user info from the sessions 
    """
    with sqlite3.connect(session_db) as c:
        c.execute("DELETE FROM session_info WHERE user_hash = ?;""", (user_hash,))

def check_admin(username):
    """
    Checks if a given user is an admin
    """
    with sqlite3.connect(database) as c:
        return c.execute("""SELECT * from users WHERE username = ? AND is_admin = ? """, (username, 1)).fetchone()!=None

def get_all_users():
    """
    Gets all users in the system
    """
    with sqlite3.connect(database) as c:
        return c.execute("""SELECT username FROM users""").fetchall()

def get_all_doors():
    """
    Gets all doors in the system
    """
    with sqlite3.connect(door_access_db) as c:
        return c.execute("""SELECT DISTINCT door_id from door_user_table""").fetchall()

def add_door_access(door, username):
    """
    Allows user 'username' to access door 'door'
    """
    with sqlite3.connect(door_access_db) as c:
        c.execute("""INSERT INTO  door_user_table (door_id, username) VALUES (?, ?)""", (door, username))
    
def remove_door_access(username):
    """
    Removes users 'username'
    """
    with sqlite3.connect(door_access_db) as c:
        c.execute("""DELETE FROM door_user_table where username = ?""", (username,))
        
