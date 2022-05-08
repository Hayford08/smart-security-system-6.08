def login_form(delete_session):
    return f'''<!DOCTYPE html>
        <html>
        <head>
        <meta charset="UTF-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1.0" />
        <title>Smart-Security-System Login</title>
        <script type="text/javascript" src="session.js"></script>
        </head>
            <body onload="checkForSession({"true" if delete_session else "false"})">
            <form method = "post">
                <label for="username">Username: </label><br>
                <input type="text" id="username" name="username"><br>
                <label for="password"> Password: </label><br>
                <input type="password" id="password" name="password"><br>
                <input type="submit" value="Submit" name="submit" id="submit">
            </form>
            </body>
        </html>
        '''

def error_login_form():
    return f'''<!DOCTYPE html>
        <html>
        <head>
        <meta charset="UTF-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1.0" />
        <title>Smart-Security-System Login</title>
        <script type="text/javascript" src="session.js"></script>
        </head>
            <body onload="checkForSession(false)">
            <p style= "color:red;"> Wrong Username-Password Combination </p>
            <form method = "post">
                <label for="username">Username: </label><br>
                <input type="text" id="username" name="username"><br>
                <label for="password"> Password: </label><br>
                <input type="password" id="password" name="password"><br>
                <input type="submit" value="Submit" id="submit">
            </form>
            </body>
        </html>
        '''
