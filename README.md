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

### Authentication and Server APIs

All the server APIs that are used by the ESP are handled in `server_src/authentication.py` file. Below is the specification for all these API requests:

```python
#?checkAccess&username=<username>&door_id=<door_id> => returns True if the user has access to the door, False otherwise

#?checkAccess&card_id=<card_id>&door_id=<door_id> => returns True if the user with the given card_id has access to the door, False otherwise

#?getUsername&card_id=<card_id> => returns the username of the user

#?getAuthenticationMethods&username=<username> => returns the list of authentication methods the user is using as: "password=<True/False>\npincode=<True/False>\nvoice=<True/False>"

#?authenticate&username=<username>&password=<password> => returns true if the password matches the username

#?authenticate&username=<username>&pincode=<pincode> => returns true if the pincode matches the username

#?authenticate&username=<username>&voice_phrase=<voice_phrase> => returns true if the voice phrase matches the username
 
# => returns "Unsupported Request" otherwise
```
The main code that handles these requests can be found in `request_handler()` function of the file. This function makes use of other helper functions that help respond to the API queries:

```python
def authenticate_login(username, password): # Given username and password, checks whether the credentials match

def authenticate_pincode(username, pincode): # Given username and pincode, checks whether the credentials match

def authenticate_voice_phrase(username, voice_phrase): # Given username and voice phrase, checks whether the credentials match

def get_credentials(username): # Retrieves all credentials of the user (including password, pincode, etc.)

def retrieve_username(card_id): # Given a card id, returns the username of the user with that card id. Returns None if the card id is not recognized.

def get_authentication_methods(username): # Given a username, returns the list of enabled authentication method of that user in the following format:
# "password=<True/False>\npincode=<True/False>\nv..."

def checkAccess(username, door_id): # Given a door id and a username, checks whether the given user has access to the given door
```

### Changing Profile Info
Users can log in to the website and change their credentials (password, pincode, voice phrase etc.).
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
        ... # for other authentication methods
    return "Password Updated Successfully"
```

### Text Input

This ESP-side class is designated to allow to enter text inputs easier on the board. The idea behind this mechanism is that the user would enter their password/pincode letter by letter and submit the result when done entering by pressing a button. (given to the class contructor).

Choosing each letter is achieved by utilizing the IMU sensor on the board such that the user can scroll through the letters in order based on the acceleration of the board in the X axis. The user can then enter the shown character by pressing a button and then move onto the next character.

```cpp
void update(int x) // Given acceleration x in the X axis, updates the current character. It should repeatedly be called when the user is entering a value.

bool isValid() // Returns true if and only if the user has submitted their response and false otherwise. The user can restart the class by clicking the button.

char *getText() // Returns the submitted response of the user. Only access this if isValid() == true.

char *getCurrentText() // Returns the current response of the user (which is being edited). Used to display the current text that is being typed.

```

### SpeechToText
This ESP-side class is designed to record and recognize passphrases from the user. The API for this class is very straightforward:

```cpp
void setup() // Should be called during the setup time of the ESP. Correctly initializes timer and secure wifi client.

char* run() // Returns a pointer to the string that the user recorded.
```

When `run()` is called, this blocking function starts recording what the user says for 5 seconds (in `record_audio()`). It then sends a request to Google speech recognition API and receives a response (these are all done under the hood in `speech_to_text()`). The resulting recognized phrase is then copied into a string and the pointer to the string is then returned.

The reason behind why this method is implemented as a blocking method is that during the 5 seconds that ESP starts recording the voice phrase, the ESP should not be doing anything else and it should only process the data from the microphone. Hence, the design choice for this function was chosen as blocking.

### MultiplePasswords

This class is designed to handle GET requests to the server. The type of request is exposed to the client by the `authentification_method`
enum.

```cpp
enum authentification_method
{
    GETUSERNAME,
    AUTHMETHODS,
    CARDID,
    CARDBYUSERNAME,
    PINCODE,
    PASSWORD,
    PHRASE
};
```

Here, `GETUSERNAME` gets the username corresponding to the card id. According to the value of an enum variable, the 
`MultiplePasswords` class sends different GET requests to the server, as shown below:

```cpp
case GETUSERNAME:
    sprintf(body, "?getUsername&card_id=%s", user_input);  // gets username from card id
    break;
case AUTHMETHODS:
    sprintf(body, "?getAuthenticationMethods&username=%s", username);       // returns the authentication methods available to the user, in the format "password=<True/False>\npincode=<True/False>\n"
    break;
case CARDID:
    sprintf(body, "?checkAccess&type=card_id&card_id=%s&door_id=%d", user_input, door_id); // returns whether the card_id has access to the door
    break;
case CARDBYUSERNAME:
    sprintf(body, "?checkAccess&type=username&username=%s&door_id=%d", user_input, door_id); // returns whether the user has access to the door
    break;
case PINCODE:
    sprintf(body, "?authenticate&type=pincode&username=%s&pincode=%s", username, user_input); // returns whether the entered pincode is correct
    break;
case PASSWORD:
    sprintf(body, "?authenticate&type=password&username=%s&password=%s", username, user_input); // returns whether the returned password is correct
    break;
case PHRASE:
    sprintf(body, "?authenticate&type=phrase&username=%s&phrase=%s", username, user_input); // returns whether the spoken phrase is correct. Doesn't work yet
    break;
default:
    break;
```
