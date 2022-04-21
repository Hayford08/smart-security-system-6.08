import datetime
import requests
import sqlite3
import json

proj_db = '/var/jail/home/team26/project/project.db' #assumes you have a la07a dir in your home dir
seconds_in_day = 24 * 60 * 60

def request_handler(request):
    with sqlite3.connect(proj_db) as c:
            c.execute("""CREATE TABLE IF NOT EXISTS users (username text, password text);""")
            c.execute("""CREATE TABLE IF NOT EXISTS voice_passwords (password text);""")
            c.execute("""CREATE TABLE IF NOT EXISTS gestures_passwords (password text);""")
            c.execute("""CREATE TABLE IF NOT EXISTS pin_passwords (password text);""")
            c.execute("""CREATE TABLE IF NOT EXISTS current_passwords (_time timestamp, voice_password text, gestures_passwords text, pin_passwords text);""")
            voice_passwords = ["enter", "friend", "pass", "let me pass", "this is my home", "edna mode"]
            gesture_passwords = ["down,right,up", "up,down,up", "left,up,down,right", "right,down,left"]
            pin_passwords = ["123456789", "helloworld", "kinginthenorth", "starwars", "124d", "609867"]
            # for pas in voice_passwords:
            #     c.execute("""INSERT INTO voice_passwords VALUES (?)""", (pas,))
            # for pas in gesture_passwords:
            #     c.execute("""INSERT INTO gestures_passwords VALUES (?)""", (pas,))
            # for pas in pin_passwords:
            #     c.execute("""INSERT INTO pin_passwords VALUES (?)""", (pas,))
            
            # info1 = c.execute("""SELECT * FROM voice_passwords""");
            # info2 = c.execute("""SELECT * FROM gestures_passwords""");
            # info3 = c.execute("""SELECT * FROM pin_passwords""");

            
    if request["method"] == "POST":
        username = ""
        password = ""
        try:
            username = request["form"]["username"]
            password = request["form"]["password"]
        except:
            return -1

        if username and password:
            with sqlite3.connect(proj_db) as c:
                info = c.execute('''SELECT * FROM users WHERE username = ? and password = ?''', (username, password))
                info_rows=[row for row in info]
                if len(info_rows) != 1:
                    return "False"
                # there exists user in the database so I am logged in
                current = info_rows[0]
                data={}
                data["username"] = current[0]
                data["password"] = current[1]
                
                day = datetime.datetime.today()
                day_info = c.execute("""SELECT * from current_passwords""")
                day_info_rows = [row for row in day_info]
                

                # # if not empty else just insert
                if day_info_rows:
                    # if day changes, delete previous and update
                    # else select the passcodes of that day
                    passcode_info = [row for row in c.execute("""SELECT * FROM current_passwords""")]
                    # return passcode_info[0][0]
                    datetime_object = datetime.datetime.strptime(passcode_info[0][0], "%Y-%m-%d %H:%M:%S.%f")
                    if datetime.datetime.today().date() == datetime_object.date():
                        data =  {"voice_passcode" : passcode_info[0][1], 
                            "gesture_passcode" : passcode_info[0][2], 
                            "pin_passcode" : passcode_info[0][3]
                            }
                        return json.dumps(data, indent = 4)
                    else:
                        c.execute("""DELETE FROM current_passwords""")
                    
                voice_passcode =[row for row in c.execute("""SELECT * FROM voice_passwords ORDER BY RANDOM() LIMIT 1""")]
                gesture_passcode = [row for row in c.execute("""SELECT * FROM gestures_passwords ORDER BY RANDOM() LIMIT 1""")]
                pin_passcode = [row for row in c.execute("""SELECT * FROM pin_passwords ORDER BY RANDOM() LIMIT 1""")]
                voice_passcode = voice_passcode[0][0]
                gesture_passcode = gesture_passcode[0][0]
                pin_passcode = pin_passcode[0][0]
                c.execute("""INSERT INTO current_passwords (_time, voice_password, gestures_passwords, pin_passwords) VALUES (?, ?, ?, ?)""", (datetime.datetime.now(), voice_passcode, gesture_passcode, pin_passcode))

                data = {"voice_passcode" : voice_passcode, 
                        "gesture_passcode" : gesture_passcode, 
                        "pin_passcode" : pin_passcode
                       }
                return json.dumps(data, indent = 4)


                