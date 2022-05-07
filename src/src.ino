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

const uint8_t BUTTON1 = 45; // pin connected to button
const uint8_t BUTTON2 = 39; // pin connected to button
MPU6050 imu;                // imu object called, appropriately, imu

Button button1(BUTTON1), button2(BUTTON2);
TextInputProcessor textInput;
PinInputProcessor pinInput;
CardScanner scanner;
Door door;
MultiplePassword multipass;
SpeechToText speechToText;

// some suggested variables you can use or delete:

enum Stage
{
  LOCKED,
  TAP,
  TEXT,
  PIN,
  UNLOCKED
};

Stage state = LOCKED;
uint8_t post_state = 1; // state of posting
int steps;

/*const uint8_t IDLE = 0;  //change if you'd like
const uint8_t DOWN = 1;  //change if you'd like
const uint8_t POST = 2;  //change if you'd like*/

char output[100], old_output[100];

const int UP = 1, DOWN = 0;
const int REST = 0, PEAK = 1, VALLEY = 2;
const double BASELINE = 11.7;
const float ZOOM = 9.81; // for display (converts readings into m/s^2)...used for visualizing only
int global_steps = 0;
uint8_t LCD_PWM = 0, LCD_CONTROL = 21;
char *username;

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
  if (imu.setupIMU(1))
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

  textInput = TextInputProcessor(BUTTON1);
  pinInput = PinInputProcessor(BUTTON1);
  pinMode(LCD_CONTROL, OUTPUT);
}

void loop()
{
  // CALL char * phrase = speechToText.run();
  // get IMU information:
  // imu.readAccelData(imu.accelCount);

  // float acc_mag = sqrt(x * x + y * y + z * z);
  // float avg_acc_mag = 1.0 / 3.0 * (acc_mag + old_acc_mag + older_acc_mag);
  // older_acc_mag = old_acc_mag;
  // old_acc_mag = acc_mag;
  // char sss[100];
  // sprintf(sss, "X:%f", acc_mag);
  // Serial.println(sss);

  security_system_fsm();
  update_lcd();

  while (millis() - primary_timer < LOOP_PERIOD); // wait for primary timer to increment
  primary_timer = millis();
}

void security_system_fsm()
{
  switch (state)
  {
  case LOCKED:
    state = TAP;
    sprintf(output, "LOCKED");
    textInput = TextInputProcessor(BUTTON1);
    pinInput = PinInputProcessor(BUTTON1);
    break;

  case TAP: // Tap Card
    scanner.loop();
    sprintf(output, "Please Tap Card"); 
    scanner.newcard[0]='\0';
    delay(3000);
    sprintf(scanner.newcard, "73 25 A1 31");  
    //scanner.newcard = "73 25 A1 31"; // hard coding for now
    if (scanner.newcard[0] != '\0')
    {
      multipass.set_auth_method(GETUSERNAME); //0 -- getting username
      Serial.printf("Id tapped: %s\n", scanner.newcard);
      
      // Request username from authentification 
      multipass.post_request_authentification(scanner.newcard);

      Serial.printf("Username: %s\n", multipass.get_username());
      username = multipass.get_username();

      //username = "hayford";
      state = PIN;
      if (strcmp(username, "")){ // username is not empty
        Serial.println("ABOUT TO GET AUTH!!!");
        multipass.set_auth_method(AUTHMETHODS); //leave for later
        multipass.post_request_authentification(); //leave for later
        if (scanner.accessAuthorized)
        {
          Serial.println("access granted");
          scanner.reset();
          state = PIN;
        }
      }
    }
    break;
  case PIN: // Enter text with imu
    // Read imu data
    int16_t data[3];
    output[0] = '\0';
    sprintf(output, "Enter your PIN"); 
    delay(1000); 
    imu.readAccelData(data); // readGyroData(data);
    float x, y, z;
    x = ZOOM * data[0] * imu.aRes;
    y = ZOOM * data[1] * imu.aRes;
    z = ZOOM * data[2] * imu.aRes;

    // Update textInput
    Serial.println(x);
    pinInput.update(x);
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
    multipass.set_auth_method(PINCODE);
    multipass.post_request_authentification(pinInput.getCurrentText(), username);
    if (multipass.is_auth_valid)
    {
      state = TEXT;
      door.open_door();
    }
    break;

  case TEXT: // Enter text with imu
    // Read imu data
    imu.readAccelData(data); // readGyroData(data);
    x = ZOOM * data[0] * imu.aRes;
    y = ZOOM * data[1] * imu.aRes;
    z = ZOOM * data[2] * imu.aRes;

    // Update textInput
    Serial.println(x);
    textInput.update(x);
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

    if (strcmp(textInput.getCurrentText(), "ENTER") == 0)
    {
      state = UNLOCKED;
      door.open_door();
    }
    break;

  case UNLOCKED:
    // If button 2 is pressed, lock the door
    Serial.println("unlocked");
    sprintf(output, "unlocked");
    if (button2.update() != 0)
    {
      sprintf(output, "locking    ");
      Serial.println("locking");
      state = LOCKED;
      door.close_door();
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
    tft.println(output);
    memcpy(old_output, output, sizeof(output));
  }
}
