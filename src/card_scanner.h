/* Code inspired and modifed from the following source:
 * All the resources for this project: https://randomnerdtutorials.com/
 * Modified by Rui Santos
 *
 * Created by FILIPEFLOP
 *
 */

#ifndef CARD_SCANNER_H
#define CARD_SCANNER_H

#include <SPI.h>
#include <MFRC522.h>
#include <string.h>
#include <Arduino.h>
//#define SS_PIN 34
#define RST_PIN 20

class CardScanner
{
private:
    MFRC522 mfrc522;
    const int RED_LED = 2;
    const int GREEN_LED = 3;
    const int BLUE_LED = 4;
    int pinMeasurement = -1;
    bool toClose = false;
    uint8_t buttonPin = 45;
    bool buttonPressed = false;
    char authorizedCard[1000] = "63 E1 B3 31";

public:
    bool accessAuthorized = false;
    char newcard[1000] = "";
    void setup()
    {
        mfrc522 = MFRC522(34, RST_PIN);
        pinMode(buttonPin, INPUT_PULLUP);
        SPI.begin();
        mfrc522.PCD_Init();
        pinMode(RED_LED, OUTPUT);
        pinMode(GREEN_LED, OUTPUT);
        pinMode(BLUE_LED, OUTPUT);
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
        if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
        {
            read_card();
        }
    }

    void reset()
    {
        toClose = false;
        accessAuthorized = false;
    }

private:
    void manage_close_button()
    {
        pinMeasurement = digitalRead(buttonPin);
        if (pinMeasurement == 0)
        {
            buttonPressed = true;
            digitalWrite(BLUE_LED, 0);
            toClose = true;
        }
        else
        {
            digitalWrite(BLUE_LED, 1);
            buttonPressed = false;
        }
    }

    void read_card()
    {
        // Show UID on serial monitor
        Serial.print("UID tag :");
        String content = "";
        char cardcontent[1000] = "";
        byte letter;
        for (byte i = 0; i < mfrc522.uid.size; i++)
        {
            Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
            if (mfrc522.uid.uidByte[i] < 0x10)
            {
                strcat(cardcontent, " 0");
            }
            else
            {
                strcat(cardcontent, " ");
            }
            Serial.print(mfrc522.uid.uidByte[i], HEX);
            char prefix[200] = "";
            sprintf(prefix, "%02X", mfrc522.uid.uidByte[i]);
            strcat(cardcontent, prefix);
        }
        Serial.println(cardcontent);
        Serial.println();
        Serial.print("Message : ");
        content.toUpperCase();
        newcard[0] = '\0';
        memcpy(newcard, &cardcontent[1], strlen(cardcontent) - 1);
        Serial.println(newcard);

        // if (!strcmp(newcard, authorizedCard))
        // {
        //     Serial.println("Authorized access");
        //     Serial.println();
        //     digitalWrite(GREEN_LED, 0);
        //     accessAuthorized = true;
        //     delay(1000);
        //     digitalWrite(GREEN_LED, 1);
        // }

        // else
        // {
        //     Serial.println(" Access denied");
        //     digitalWrite(RED_LED, 0);
        //     delay(3000);
        //     digitalWrite(RED_LED, 1);
        // }
    }
};

#endif // CARD_SCANNER_H
