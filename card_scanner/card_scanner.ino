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
 
void setup() 
{
  Serial.begin(9600);
  SPI.begin();      
  mfrc522.PCD_Init();
  Serial.println("Approximate your card to the reader...");
  Serial.println();

}


void loop() 
{
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
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

  if(!strcmp(newcard, "E0 16 99 1B"))
  {
    Serial.println("Authorized access");
    Serial.println();
    delay(3000);
  }
 
 else{
    Serial.println(" Access denied");
    delay(3000);
  }
} 