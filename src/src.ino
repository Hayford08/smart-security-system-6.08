#include <WiFi.h>     //Connect to WiFi Network
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>      //Used in support of TFT Display
#include <string.h>   //used for some string handling and processing.
#include <mpu6050_esp32.h>
#include <math.h>
#include "button.h"
#include "text_input.h"
#include "pin_input.h"
#include "card_scanner.h"
#include "door.h"
#include "multiple_password.h"
#include "speech_to_text.h"
#include "gestures.h"

TFT_eSPI tft = TFT_eSPI(); // Invoke library, pins defined in User_Setup.h

char network[] = "EECS-Lounges";
char password[] = "";
/* Having network issues since there are 50 MIT and MIT_GUEST networks?. Do the following:
    When the access points are printed out at the start, find a particularly strong one that you're targeting.
    Let's say it is an MIT one and it has the following entry:
   . 4: MIT, Ch:1 (-51dBm)  4:95:E6:AE:DB:41
   Do the following...set the variable channel below to be the channel shown (1 in this example)
   and then copy the MAC address into the byte array below like shown.  Note the values are rendered in hexadecimal
   That is specified by putting a leading 0x in front of the number. We need to specify six pairs of hex values so:
   a 4 turns into a 0x04 (put a leading 0 if only one printed)
   a 95 becomes a 0x95, etc...
   see starting values below that match the example above. Change for your use:
   Finally where you connect to the network, comment out
     WiFi.begin(network, password);
   and uncomment out:
     WiFi.begin(network, password, channel, bssid);
   This will allow you target a specific router rather than a random one!
*/
uint8_t channel = 1;                                 // network channel on 2.4 GHz
byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; // 6 byte MAC address of AP you're targeting.

const uint8_t LOOP_PERIOD = 10; // milliseconds
uint32_t primary_timer = 0;
uint32_t posting_timer = 0;
uint32_t step_timer = 0;
float x, y, z; // variables for grabbing x,y,and z values
const char USER[] = "random";

// Some constants and some resources:
// const int RESPONSE_TIMEOUT = 6000;     // ms to wait for response from host
// const int POSTING_PERIOD = 6000;       // periodicity of getting a number fact.
// const uint16_t IN_BUFFER_SIZE = 1000;  // size of buffer to hold HTTP request
// const uint16_t OUT_BUFFER_SIZE = 1000; // size of buffer to hold HTTP response
// char request_buffer[IN_BUFFER_SIZE];   // char array buffer to hold HTTP request
// char response_buffer[OUT_BUFFER_SIZE]; // char array buffer to hold HTTP response

const int TRANSITION_TIMEOUT = 2000;
uint32_t transition_timer = millis();

const uint8_t BUTTON1 = 45; // pin connected to button
const uint8_t BUTTON2 = 39; // pin connected to button
MPU6050* imu = new MPU6050;                // imu object called, appropriately, imu

Button button1(BUTTON1), button2(BUTTON2);
TextInputProcessor textInput;
PinInputProcessor pinInput;
CardScanner scanner;
Door door;
MultiplePassword multipass;
SpeechToText speechToText;
Gestures gestures;

// some suggested variables you can use or delete:

enum Stage
{
  LOCKED,
  TAP,
  PIN,
  TEXT,
  SPEECH,
  GESTURE,
  UNLOCKED
};

Stage state = LOCKED;
uint8_t post_state = 1; // state of posting
int steps;

/*const uint8_t IDLE = 0;  //change if you'd like
const uint8_t DOWN = 1;  //change if you'd like
const uint8_t POST = 2;  //change if you'd like*/

char output[100], old_output[100];
char stage[20] = "locked";

const int UP = 1, DOWN = 0;
const int REST = 0, PEAK = 1, VALLEY = 2;
const double BASELINE = 11.7;
const float ZOOM = 9.81; // for display (converts readings into m/s^2)...used for visualizing only
int global_steps = 0;
uint8_t LCD_PWM = 0, LCD_CONTROL = 21;
char username[100];



void setup()
{
  tft.init();                              // init screen
  tft.setRotation(2);                      // adjust rotation
  tft.setTextSize(1);                      // default font size
  tft.fillScreen(TFT_WHITE);               // fill background
  tft.setTextColor(TFT_PURPLE, TFT_WHITE); // set color of font to green foreground, black background
  Serial.begin(115200);                    // begin serial comms
  delay(100);                              // wait a bit (100 ms)
  Wire.begin();
  delay(50); // pause to make sure comms get set up
  if (imu->setupIMU(1))
  {
    Serial.println("IMU Connected!");
  }
  else
  {
    Serial.println("IMU Not Connected :/");
    Serial.println("Restarting");
    ESP.restart(); // restart the ESP (proper way)
  }

  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
  {
    Serial.println("no networks found");
  }
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      Serial.printf("%d: %s, Ch:%d (%ddBm) %s ", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "");
      uint8_t *cc = WiFi.BSSID(i);
      for (int k = 0; k < 6; k++)
      {
        Serial.print(*cc, HEX);
        if (k != 5)
          Serial.print(":");
        cc++;
      }
      Serial.println("");
    }
  }
  delay(100); // wait a bit (100 ms)

  // if using regular connection use line below:
  WiFi.begin(network, password);
  // if using channel/mac specification for crowded bands use the following:
  // WiFi.begin(network, password, channel, bssid);

  uint8_t count = 0; // count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count < 12)
  {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected())
  { // if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    delay(500);
  }
  else
  { // if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }

  scanner.setup();
  door.setup();
  speechToText.setup();
  multipass.setup();
  gestures.setup(imu);

  sprintf(output, "LOCKED");

  textInput = TextInputProcessor(BUTTON1);
  pinInput = PinInputProcessor(BUTTON1);
  pinMode(LCD_CONTROL, OUTPUT);
}

void loop() {
  update_lcd();
  security_system_fsm();

  while (millis() - primary_timer < LOOP_PERIOD); // wait for primary timer to increment
  primary_timer = millis();
}

void security_system_fsm() {
  switch (state) {
  case LOCKED: {
    if (millis() - transition_timer < TRANSITION_TIMEOUT) break;
    state = TAP;
    sprintf(stage, "locked");
    sprintf(output, "");
    textInput = TextInputProcessor(BUTTON1);
    pinInput = PinInputProcessor(BUTTON1);
    transition_timer = millis();
    }
    break;

  case TAP: {// Tap Card
    if (millis() - transition_timer < TRANSITION_TIMEOUT) break;
    sprintf(stage, "Card Tap"); 
    sprintf(output, "Please Tap Card"); 
    //scanner.newcard[0]='\0';
    //delay(3000);
    scanner.loop();
    //sprintf(scanner.newcard, "73 25 A1 31"); // comment this line out  
    // scanner.newcard = "73 25 A1 31"; // hard coding for now
    if (scanner.newcard[0] != '\0') {
//      Serial.printf("Id tapped: %s\n", scanner.newcard);
//      multipass.get_username(scanner.newcard, username);
//      Serial.printf("Username: %s\n", username);
      multipass.get_username(scanner.newcard, username);
      Serial.printf("Username: %s\n", username);

      //sprintf(username, "hayford");
//      Serial.printf("k%sk", username);
      //username = "hayford";
      if (username[0] != '\0') { // username is not empty
        if (! multipass.check_access_by_username(username, 78)) {
          sprintf(output, "Access Denied, \n%s", username);
          state = LOCKED;
          break;
        }

        state = PIN;

        sprintf(output, "Hi, %s\n", username);
        
        Serial.println("ABOUT TO GET AUTH METHODS!!!");
        
        multipass.get_auth_methods(username);
        // if (scanner.accessAuthorized) {
        //   Serial.println("access granted");
        //   scanner.reset();
        //   state = PIN;
        // }
        Serial.println("Going to pin");
        transition_timer = millis();
        scanner.reset();
      }
    }
    }
    break;
  case PIN:{ // Enter text with imu
    //state = GESTURE;
    //break;
    if (! multipass.is_pincode_needed) {
      state = TEXT;
      break;
    }
    if (millis() - transition_timer < TRANSITION_TIMEOUT) break;
    sprintf(stage, "Pincode");
    // Read imu data
    int16_t data[3];
    //output[0] = '\0';
    //sprintf(output, "Enter your PIN"); 
    //delay(1000); 
    imu->readAccelData(data); // readGyroData(data);
    float x, y, z;
    x = ZOOM * data[0] * imu->aRes;
    y = ZOOM * data[1] * imu->aRes;
    z = ZOOM * data[2] * imu->aRes;

    // Update textInput
    Serial.println(x);
    pinInput.update(x);
    char prev_output[100];
    strcpy(prev_output, output);
    if (pinInput.isValid())
    {
      sprintf(output, "%s     ", pinInput.getText());
    }
    else
    {
      sprintf(output, "%s     ", pinInput.getCurrentText());
    }

    Serial.println(pinInput.isValid());
    Serial.println(pinInput.getCurrentText());

    // If the text is a hardcoded "enter", unlock the door

    // char lower[100];
    // to_lower(textInput.getCurrentText(), lower);
    // multipass.set_auth_method(PINCODE);
    if (strcmp(prev_output, output) != 0) {
    bool result = multipass.authenticate_by_pincode(username, pinInput.getCurrentText());
    if (result) {
      state = TEXT;
      transition_timer = millis();
    }
    }
    }
    break;

  case TEXT: { // Enter text with imu
    if (! multipass.is_password_needed) {
      state = SPEECH;
      break;
    }
    if (millis() - transition_timer < TRANSITION_TIMEOUT) break;
    sprintf(stage, "Password");
    // Read imu data
    int16_t data[3];
    imu->readAccelData(data); // readGyroData(data);
    x = ZOOM * data[0] * imu->aRes;
    y = ZOOM * data[1] * imu->aRes;
    z = ZOOM * data[2] * imu->aRes;

    // Update textInput
    Serial.println(x);
    textInput.update(x);
    char prev_output[100];
    strcpy(prev_output, output);
    if (textInput.isValid())
    {
      sprintf(output, "%s     ", textInput.getText());
    }
    else
    {
      sprintf(output, "%s     ", textInput.getCurrentText());
    }

    Serial.println(textInput.isValid());
    Serial.println(textInput.getCurrentText());

    // If the text is a hardcoded "enter", unlock the door

    // char lower[100];
    // to_lower(textInput.getCurrentText(), lower);
    if (strcmp(prev_output, output) != 0) {
    char lower[100];
    to_lower(textInput.getCurrentText(), lower);
    bool result = multipass.authenticate_by_password(username, lower);
    if (result) {
      state = SPEECH;
      transition_timer = millis();
      //door.open_door();
    }
    }
    }
    break;
  case SPEECH: {
    if (! multipass.is_phrase_needed) {
      state = GESTURE;
      break;
    }
    if (millis() - transition_timer < TRANSITION_TIMEOUT) break;
    sprintf(stage, "Speech");
    sprintf(output, "Push button 1");

    if (button1.update()) {
      char word_out[50];
      speechToText.run(word_out);
      char word_out_lower[50];
      to_lower(word_out, word_out_lower);
      word_out_lower[strlen(word_out_lower)-1] = '\0';
      bool result = multipass.authenticate_by_phrase(username, word_out_lower + 1);
  
      if (result) {
        state = GESTURE;
        // Remove following lines
        state = UNLOCKED;
        door.open_door();
        transition_timer = millis();
      }
    }
    }  
    break;

  case GESTURE:
    if (! multipass.is_gesture_needed) {
      state = UNLOCKED;
      door.open_door();  
    }
    if (millis() - transition_timer < TRANSITION_TIMEOUT) break;
    sprintf(stage, "Gesture");
    //state = UNLOCKED;  
    char result_gestures[100];
    gestures.record(result_gestures);
    if (strlen(result_gestures) != 0)
      result_gestures[strlen(result_gestures)-1] = '\0';

    sprintf(output, result_gestures);

    if (button2.update()) {
      bool result = multipass.authenticate_by_gesture(username, result_gestures);
      if (result) {
        state = UNLOCKED;
        door.open_door();
        transition_timer = millis();
      }
      gestures.reset();
    }
    break;

  case UNLOCKED: {
    if (millis() - transition_timer < TRANSITION_TIMEOUT) break;
    // If button 2 is pressed, lock the door
    Serial.println("unlocked");
    sprintf(stage, "unlocked");
    sprintf(output, "");
    if (button2.update() != 0)
    {
      sprintf(output, "locking    ");
      Serial.println("locking");
      state = LOCKED;
      door.close_door();
      transition_timer = millis();
    }
    }
    break;
  }
}

void update_lcd()
{
  if (strcmp(output, old_output))
  {
    tft.fillScreen(TFT_WHITE); // fill background
    tft.setCursor(0, 0, 4);
    tft.println(stage);
    tft.setCursor(0, 25, 4);
    tft.println(output);
    memcpy(old_output, output, sizeof(output));
  }
}
