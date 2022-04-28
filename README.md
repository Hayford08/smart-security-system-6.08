# smart-security-system-6.08

### Cardboard door construction for illustrative operation of the security system

For easier visualisation of functionality of the system a cardboard system (can be treated as a box or door) which holds some secret information, object, or simply something that should be secured to some level. Real world references can be anything from opening a door in lobby 7 to keeping secure seed phrase for crypto wallet or other assets. Currently the system works as a visualization rather than actually reliable piece physical safe.

The door is connected to a box and operates the following way: it is connected to the main part at one axis at the edge of the box (rotational axis on the video), and from one side of it connected to the servo which upon respective signal can move 90 degrees one and other direction affecting respectively door opening and closing.

The default (test) functionality is changed by a button press (pin 45), and respectively opens and closes the door when pressed.

### Integration with RFID scanner: LED indicator

For visible system reaction on correct/incorrect id (can be applied to any sort of lock) there is also an LED set up and connected to ESP32-S2. After RFID integration it turns red if there is an attempt to unlock the system with a wrong card, green if there is an attempt to unlock the door with a valid card (and respectively door is opening). Blue if button 45 is pressed (also effects in closing the door if door not already closed). This system helps to track system response on different attempts of interacting with the system without a need to connect with a serial monitor or LCD. 

### RFID scanning
We have connected the RFID such that its SDA pin is connected to pin 34 and its RST pin is connected to pin 20.

To communicate RFID we use the library <MFRC522.h>.

First we create an mfrc522 class that handles the communication with the RFID.

```cpp
MFRC522 mfrc522(SS_PIN, RST_PIN);
```
Once a card has been read, we iterate over its userid tag to check that it is the same as the uid of one of the allowed cards, in the following way.

```cpp
char cardcontent[1000] = "";
byte letter;
for (byte i = 0; i < mfrc522.uid.size; i++) 
{
    if(mfrc522.uid.uidByte[i] < 0x10){
    strcat(cardcontent, " 0");
    }
    else{
    strcat(cardcontent, " ");
    }
    char prefix[200]="";
    sprintf(prefix, "%02X", mfrc522.uid.uidByte[i]);
    strcat(cardcontent, prefix);
}
```
We first create a cardcontent variable to keep the userid of the card that was scanned.

We look at every byte of the user id which is in hex, and then we sprintf each byte in hex representation to a string, which we then add to the end of cardcontent. Depending on how large the byte is wes either add a 0 infront of its string representation or not.

Depending on whether the the resulting string after reading the card contents matches the stored user id, we proceed to opening the door or not.


### Servo

The servo is connected to pin 21.

The purpose of the servo is to open and close the door as demonstrated in the video in this google drive.

The servo operation is handled by the operate_door() function.

If the door is open and the button has been pressed to close to board, we rotate the servo as shown in the code below, by gradually changing its position, and then set the door to be closed.

```cpp
    for (pos = pos_max; pos >= pos_min; pos -= 1) { // goes from 180 degrees to 0 degrees
      ledcWrite(pwm_channel, pos);    // tell servo to go to position in variable 'pos'
      delay(50);             // waits 15ms for the servo to reach the position
    }
    doorOpen = !doorOpen;
```

If the door is closed and the rfid authorizes access, door gradually opens, by doing the opposite from the above code, instead of pos-=1, we do pos+=1.

### Session Incorporation to website

The initial implementation was to use PHP to create a database to get persistent sessions on the website. Since the class server does not support PHP, the sessions were created using Python with a database. 

Upon sending a get request (first time), the site checks if a session has already been created. If not, the login_form is uploaded to the user for getting input. When the input credentials are verified, the session_db stores the unique id associated with the computer used to log in, username, and password for further access to the website. To get the unique user_id, a universally unique identifier is hashed and stored along with the username and password. Here is the code that hashes the user id:

```cpp
def get_id():
   """
   This function returns the unique id for the computer the user uses
   """
   return hash(uuid.UUID(int=uuid.getnode()))
```

The following function shows how the user information is stored for a particular session of using the website on the server:

```cpp
def get_user_info_from_session():
   """
   This function returns the user's username and password from the session database
   """
   user_info = None
   with sqlite3.connect(session_db) as c:
       c.execute("""CREATE TABLE IF NOT EXISTS session_info (user_hash real, username text, password text);""")
       user_info = c.execute("""SELECT username, password FROM session_info WHERE user_hash = ?;""", (user_hash,)).fetchone()
   return user_info
 ```


The session is deleted when the user logs out of the website. In that case, the database clears the information associated with a particular user_id. This functionality was tested with different usernames and passwords and works as intended. Next thing to work on will be to have a separate page for changing the password.


### Changing Profile Info

Users can log in to the website and change their credentials (password, pincode, etc.).

This is done in `profile.py` as follows:

```python
new_password = request['form']['new_password']
new_pincode = request['form']['pincode']
update_credentials(username, password, {'pincode': new_pincode, 'password': new_password})
```

The `update_credentials` function in `authentication.py` then connects to the database and update the information accordingly:

```python
def update_credentials(username, password, data):
    # data is a dictionary. For now just write data = {"pincode": actual_pincode_value}
    with sqlite3.connect(database) as c:
        object = c.execute("""SELECT * FROM users WHERE username = ? AND password = ?""", (username, password)).fetchone()
        if "pincode" not in data:
            data["pincode"] = object[2]
        c.execute("""UPDATE users SET pincode = ? WHERE username = ? AND password = ?""", (data['pincode'], username, password)).fetchone()
        c.execute("""UPDATE users SET password = ? WHERE username = ? AND password = ?""", (data['password'], username, password)).fetchone()
    return "Password Updated Successfully"
```
