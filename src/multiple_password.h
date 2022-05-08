#ifndef MULTIPLE_PASSWORD_H
#define MULTIPLE_PASSWORD_H

//#include <WiFi.h>     //Connect to WiFi Network
//#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
//#include <SPI.h>      //Used in support of TFT Display
#include <string.h> //used for some string handling and processing.
//#include <mpu6050_esp32.h>
//#include <math.h>
//#include "button.h"
//#include "text_input.h"
//#include "card_scanner.h"
//#include "door.h"
#include "support_functions.h"

// enum authentification_method
// {
//     GETUSERNAME,
//     AUTHMETHODS,
//     CARDID,
//     CARDBYUSERNAME,
//     PINCODE,
//     PASSWORD,
//     PHRASE
// };

class MultiplePassword
{
private:
    const int RESPONSE_TIMEOUT = 6000;            // ms to wait for response from host
    const int POSTING_PERIOD = 6000;              // periodicity of getting a number fact.
    static const uint16_t IN_BUFFER_SIZE = 1000;  // size of buffer to hold HTTP request
    static const uint16_t OUT_BUFFER_SIZE = 1000; // size of buffer to hold HTTP response
    char request_buffer[IN_BUFFER_SIZE];          // char array buffer to hold HTTP request
    char response_buffer[OUT_BUFFER_SIZE];        // char array buffer to hold HTTP response
    char body[200];                               // for body
    // authentification_method auth = GETUSERNAME;   // default
    char username[6000]="";
    // bool is_auth_valid = false;
    bool password_authenticated = false;
    bool pincode_authenticated = false;
    bool phrase_authenticated = false;
    bool card_authenticated = false;
    bool is_password_needed = false;
    bool is_pincode_needed = false;
    bool is_phrase_needed = false;

public:
    void setup() {
        password_authenticated = false;
        pincode_authenticated = false;
        phrase_authenticated = false;
        card_authenticated = false;
        is_password_needed = false;
        is_pincode_needed = false;
        is_phrase_needed = false;
    }

    /**
     * Sends a GET request with the card id and replaces the username field with the associated username
     */
    void get_username(char *card_id, char* obtained_username) {
        sprintf(body, "?getUsername&card_id=%s", card_id);
        request_buffer[0] = '\0';
        sprintf(request_buffer, "GET https://608dev-2.net/sandbox/sc/team26/server_src/authentication.py%s HTTP/1.1\r\n", body);
        strcat(request_buffer, "Host: 608dev-2.net\r\n\r\n");
        // Serial.println(request_buffer);
        response_buffer[0] = '\0';
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        username[0] = '\0';
        obtained_username[0] = '\0';
        sprintf(username, response_buffer);
        sprintf(obtained_username, response_buffer);
        Serial.printf("GOT USERNAME %s\n", username);
        //sprintf(username, response_buffer); // response should be smth like username if this card_id exists + if a person can access the door
    }

    /**
     * Sends a GET request to the server to get auth methods for username
     * and updates is_pincode_needed and is_password_needed
     */
    void get_auth_methods(char *username) {
        sprintf(body, "?getAuthenticationMethods&username=%s", username);
        request_buffer[0] = '\0';
        sprintf(request_buffer, "GET https://608dev-2.net/sandbox/sc/team26/server_src/authentication.py%s HTTP/1.1\r\n", body);
        strcat(request_buffer, "Host: 608dev-2.net\r\n\r\n");
        // Serial.println(request_buffer);
        memset(response_buffer, 0, OUT_BUFFER_SIZE);
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        Serial.printf("%s\n", response_buffer);
        Serial.printf("%s\n", response_buffer);
        Serial.printf("ok cool");
        parse_auth_methods(response_buffer);
        Serial.printf("gotcha");
        Serial.printf("Pincode %sneeded, password %sneeded\n, voice %sneeded", is_pincode_needed?"": "not ", is_password_needed?"": "not ", is_phrase_needed?"": "not ");
        password_authenticated = !is_password_needed;
        pincode_authenticated = !is_pincode_needed;
        phrase_authenticated = !is_phrase_needed;
        //sprintf(username, response_buffer); // response should be smth like username if this card_id exists + if a person can access the door
    }

    /**
     * Check if the user with this card has access to the door
     */ 
    bool check_access_by_card(char *card_id, int door_id = 1) {
        sprintf(body, "?checkAccess&type=card_id&card_id=%s&door_id=%d", card_id, door_id);
        request_buffer[0] = '\0';
        sprintf(request_buffer, "GET https://608dev-2.net/sandbox/sc/team26/server_src/authentication.py%s HTTP/1.1\r\n", body);
        strcat(request_buffer, "Host: 608dev-2.net\r\n\r\n");
        // Serial.println(request_buffer);
        response_buffer[0] = '\0';
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        parse_auth_methods(response_buffer);
        //sprintf(username, response_buffer); // response should be smth like username if this card_id exists + if a person can access the door
        // if(response_buffer[0] == 'T' && response_buffer[1] == 'r' && response_buffer[2] == 'u' && response_buffer[3] == 'e')
        card_authenticated = strcmp(response_buffer, "True") == 0;
        return card_authenticated;
    }

    /**
     * Check if the user with this username has access to the door
     */ 
    bool check_access_by_username(char *username, int door_id = 1) {
        sprintf(body, "?checkAccess&type=username&username=%s&door_id=%d", username, door_id);
        request_buffer[0] = '\0';
        sprintf(request_buffer, "GET https://608dev-2.net/sandbox/sc/team26/server_src/authentication.py%s HTTP/1.1\r\n", body);
        strcat(request_buffer, "Host: 608dev-2.net\r\n\r\n");
        // Serial.println(request_buffer);
        response_buffer[0] = '\0';
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        parse_auth_methods(response_buffer);
        //sprintf(username, response_buffer); // response should be smth like username if this card_id exists + if a person can access the door
        // if(response_buffer[0] == 'T' && response_buffer[1] == 'r' && response_buffer[2] == 'u' && response_buffer[3] == 'e')
        card_authenticated = strcmp(response_buffer, "True") == 0;
        return card_authenticated;
    }

    /**
     * Check if the given pincode is correct
     */ 
    bool authenticate_by_pincode(char *username, char* pincode) {
        sprintf(body, "?authenticate&type=pincode&username=%s&pincode=%s", username, pincode);
        request_buffer[0] = '\0';
        sprintf(request_buffer, "GET https://608dev-2.net/sandbox/sc/team26/server_src/authentication.py%s HTTP/1.1\r\n", body);
        strcat(request_buffer, "Host: 608dev-2.net\r\n\r\n");
        // Serial.println(request_buffer);
        response_buffer[0] = '\0';
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        parse_auth_methods(response_buffer);
        //sprintf(username, response_buffer); // response should be smth like username if this card_id exists + if a person can access the door
        // if(response_buffer[0] == 'T' && response_buffer[1] == 'r' && response_buffer[2] == 'u' && response_buffer[3] == 'e')
        if(strcmp(response_buffer, "True") == 0)
            pincode_authenticated = true; // somehow
        return pincode_authenticated;
    }

    /**
     * Check if the given password is correct
     */ 
    bool authenticate_by_password(char *username, char* password) {
        sprintf(body, "?authenticate&type=password&username=%s&password=%s", username, password);
        request_buffer[0] = '\0';
        sprintf(request_buffer, "GET https://608dev-2.net/sandbox/sc/team26/server_src/authentication.py%s HTTP/1.1\r\n", body);
        strcat(request_buffer, "Host: 608dev-2.net\r\n\r\n");
        // Serial.println(request_buffer);
        response_buffer[0] = '\0';
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        parse_auth_methods(response_buffer);
        //sprintf(username, response_buffer); // response should be smth like username if this card_id exists + if a person can access the door
        // if(response_buffer[0] == 'T' && response_buffer[1] == 'r' && response_buffer[2] == 'u' && response_buffer[3] == 'e')
        if(strcmp(response_buffer, "True") == 0)
            password_authenticated = true; // somehow
        return password_authenticated;
    }

    /**
     * Check if the phrase is correct
     */ 
    bool authenticate_by_phrase(char *username, char* phrase) {
        sprintf(body, "?authenticate&type=phrase&username=%s&phrase=%s", username, phrase);
        request_buffer[0] = '\0';
        sprintf(request_buffer, "GET https://608dev-2.net/sandbox/sc/team26/server_src/authentication.py%s HTTP/1.1\r\n", body);
        strcat(request_buffer, "Host: 608dev-2.net\r\n\r\n");
        // Serial.println(request_buffer);
        response_buffer[0] = '\0';
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        parse_auth_methods(response_buffer);
        //sprintf(username, response_buffer); // response should be smth like username if this card_id exists + if a person can access the door
        // if(response_buffer[0] == 'T' && response_buffer[1] == 'r' && response_buffer[2] == 'u' && response_buffer[3] == 'e')
        if(strcmp(response_buffer, "True") == 0)
            phrase_authenticated = true; // somehow
        return phrase_authenticated;
    }

    /**
     * Return true if the door can be opened after having done the necessary auths
     */ 
    bool can_open_door() {
        return password_authenticated && pincode_authenticated && phrase_authenticated;
    }

    void parse_auth_methods(char *auth_methods) {
        // password=True\npincode=True\nvoice=True\n
        Serial.printf("auth_methods:%s\n", auth_methods);
        char * token = strtok(auth_methods, "password=");
        Serial.printf("token:%s\n", token);
        // loop through the string to extract all other tokens
        char* password = strtok(NULL, "\npincode=");
        Serial.printf("password:%s\n", password);
        char* pincode = strtok(NULL, "\nvoice=");
        Serial.printf("pincode:%s\n", pincode);
        char* phrase = strtok(NULL, "\n");
        Serial.printf("phrase:%s\n", phrase);

        if (strcmp(password, "True") == 0) {
            is_password_needed = true;
        } else {
            is_password_needed = false;
        }
        if (strcmp(pincode, "True") == 0) {
            is_pincode_needed = true;
        } else {
            is_pincode_needed = false;
        }
        if (strcmp(phrase, "True") == 0) {
            is_phrase_needed = true;
        } else {
            is_phrase_needed = false;
        }
    }
};

#endif // MULTIPLE_PASSWORD_H
