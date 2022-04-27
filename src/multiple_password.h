#include <WiFi.h>     //Connect to WiFi Network
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>      //Used in support of TFT Display
#include <string.h>   //used for some string handling and processing.
#include <mpu6050_esp32.h>
#include <math.h>
#include "button.h"
#include "text_input.h"
#include "card_scanner.h"
#include "door.h"
#include "support_functions.ino"

enum authentification_method
{
    PINCODE,
    CARDID,
    PHRASE
};

class MultiplePassword
{
private:
    const int RESPONSE_TIMEOUT = 6000;     // ms to wait for response from host
    const int POSTING_PERIOD = 6000;       // periodicity of getting a number fact.
    const uint16_t IN_BUFFER_SIZE = 1000;  // size of buffer to hold HTTP request
    const uint16_t OUT_BUFFER_SIZE = 1000; // size of buffer to hold HTTP response
    char request_buffer[IN_BUFFER_SIZE];   // char array buffer to hold HTTP request
    char response_buffer[OUT_BUFFER_SIZE]; // char array buffer to hold HTTP response
    char body[100];                        // for body
    authentification_method auth = CARDID; // default
public:
    void post_request_authentification(char *card_id, char *user_input)
    {
        // GENERATE BODY JSON
        switch (auth)
        {
        case CARDID:
            sprintf(body, "{\"card_id\":\"%s\"}", card_id);
            break;
        case PINCODE:
            sprintf(body, "{\"card_id\":\"%s\",\"pincode\":\"%s\"}", card_id, user_input);
            break;
        case PHRASE:
            sprintf(body, "{\"card_id\":\"%s\",\"phrase\":\"%s\"}", card_id, user_input);
            break;
        default:
            break;
        }
        int body_len = strlen(body); // calculate body length (for header reporting)
        sprintf(request_buffer, "POST https://608dev-2.net/sandbox/sc/team26/server_src HTTP/1.1\r\n");
        strcat(request_buffer, "Host: 608dev-2.net\r\n");
        strcat(request_buffer, "Content-Type: application/json\r\n");
        sprintf(request_buffer + strlen(request_buffer), "Content-Length: %d\r\n", body_len); // append string formatted to end of request buffer
        strcat(request_buffer, "\r\n");                                                       // new line from header to body
        strcat(request_buffer, body);                                                         // body
        strcat(request_buffer, "\r\n");                                                       // new line
        Serial.println(request_buffer);
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
    }
}
