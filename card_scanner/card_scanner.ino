 /* Code inspired and modifed from the following source:
 * All the resources for this project: https://randomnerdtutorials.com/
 * Modified by Rui Santos
 * 
 * Created by FILIPEFLOP
 * 
 */
 
#include <SPI.h>
#include <MFRC522.h>
#include <string.h>
#define SS_PIN 34
#define RST_PIN 20
MFRC522 mfrc522(SS_PIN, RST_PIN);

const int RED_LED = 2;
const int GREEN_LED = 3;
const int BLUE_LED = 4;
int pos;
int pos_min = int(0.05*1024);
int pos_max = int(0.1*1024);
int servoPin = 21;
int buttonPin = 45;
int pwm_channel = 0;
bool doorOpen = false;
bool buttonPressed = false;
bool toClose = false;
int pinMeasurement = -1;
bool accessAuthorized = false;
char authorizedCard[1000] = "63 E1 B3 31";

void setup() 
{
  Serial.begin(115200);
  ledcSetup(pwm_channel, 50, 10);
  ledcAttachPin(servoPin, pwm_channel);
  pinMode(buttonPin,INPUT_PULLUP);
  SPI.begin();      
  mfrc522.PCD_Init();
  pinMode(RED_LED,OUTPUT);
  pinMode(GREEN_LED,OUTPUT);
  pinMode(BLUE_LED,OUTPUT);
  digitalWrite(RED_LED, 1);
  digitalWrite(BLUE_LED, 1);
  digitalWrite(GREEN_LED, 1);
  Serial.println("Approximate your card to the reader...");
  Serial.println();  
}


void loop() 
{
  manage_close_button();
  // Look for new cards
  if(mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()){
    read_card();
  }
} 

void operate_door(){
  if(toClose && doorOpen){
    for (pos = pos_max; pos >= pos_min; pos -= 1) { // goes from 180 degrees to 0 degrees
      ledcWrite(pwm_channel, pos);    // tell servo to go to position in variable 'pos'
      delay(50);             // waits 15ms for the servo to reach the position
    }
    doorOpen = !doorOpen;
  }
  if(accessAuthorized && !doorOpen){
    for (pos = pos_min; pos <= pos_max; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      ledcWrite(pwm_channel, pos);    // tell servo to go to position in variable 'pos'
      delay(50);             // waits 15ms for the servo to reach the position
    }
    doorOpen = !doorOpen;
  }
  toClose = false;
}

void manage_close_button(){
  pinMeasurement = digitalRead(buttonPin);
  if (pinMeasurement == 0){
    buttonPressed = true;
    digitalWrite(BLUE_LED, 0);
    toClose = true;
    operate_door();
  }
  else{
    digitalWrite(BLUE_LED, 1);
    buttonPressed = false;
  }
}

void read_card(){
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  char cardcontent[1000] = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     if(mfrc522.uid.uidByte[i] < 0x10){
       strcat(cardcontent, " 0");
     }
     else{
       strcat(cardcontent, " ");
     }
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     char prefix[200]="";
     sprintf(prefix, "%02X", mfrc522.uid.uidByte[i]);
     strcat(cardcontent, prefix);
  }
  Serial.println(cardcontent);
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  char newcard[1000]="";
  memcpy(newcard, &cardcontent[1], strlen(cardcontent)-1);
  Serial.println(newcard);

  if(!strcmp(newcard, authorizedCard))
  {
    Serial.println("Authorized access");
    Serial.println();
    digitalWrite(GREEN_LED, 0);
    accessAuthorized = true;
    operate_door();
    accessAuthorized = false;
    delay(1000);
    digitalWrite(GREEN_LED, 1);
  }
 
  else{
    Serial.println(" Access denied");
    digitalWrite(RED_LED, 0);
    delay(3000);
    digitalWrite(RED_LED, 1);
  }
}