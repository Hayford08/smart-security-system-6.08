# smart-security-system-6.08

# RFID scanning
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


# Servo

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