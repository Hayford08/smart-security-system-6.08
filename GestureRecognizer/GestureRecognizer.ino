#include <WiFi.h> //Connect to WiFi Network
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h> //Used in support of TFT Display
#include <string.h>  //used for some string handling and processing.
#include <mpu6050_esp32.h>
#include<math.h>


char network[] = "MIT";
char password[] = "";

MPU6050 imu; //imu object called, appropriately, imu
float old_acc_mag;  //previous acc mag
float older_acc_mag;  //previous prevoius acc mag
uint8_t gesture_state = 0;
const uint8_t REST = 0;
const uint8_t UP = 1;
const uint8_t DOWN = 2;
const uint8_t LEFT = 3;
const uint8_t RIGHT = 4;

char left_response[50]="";
char right_response[50]="";
char up_response[50]="";
char down_response[50]="";

char left_request[50]="";
char right_request[50]="";
char up_request[50]="";
char down_request[50]="";

const uint8_t LOOP_PERIOD = 30; //milliseconds
uint32_t primary_timer = 0;
uint32_t posting_timer = 0;
uint32_t step_timer = 0;

const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const int QUERY_PERIOD = 1000; //ms to wait between posting step

const uint16_t IN_BUFFER_SIZE = 10000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 10000; //size of buffer to hold HTTP response
char post_request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char post_response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

float measurements[5000];
char str_measurements[6000] = "";
int iter = 0;
bool measure;
char json_body[6000] = "";
float left_sequence[2000];
float right_sequence[2000];
float up_sequence[2000];
float down_sequence[2000];

enum button_state {S0,S1,S2,S3,S4};
 
class Button{
  public:
  uint32_t S2_start_time;
  uint32_t button_change_time;    
  uint32_t debounce_duration;
  uint32_t long_press_duration;
  uint8_t pin;
  uint8_t flag;
  uint8_t button_pressed;
  button_state state; // This is public for the sake of convenience
  Button(int p = 0) {
  flag = 0;  
    state = S0;
    pin = p;
    S2_start_time = millis(); //init
    button_change_time = millis(); //init
    debounce_duration = 10;
    long_press_duration = 1000;
    button_pressed = 0;
    pinMode(pin, INPUT_PULLUP);
  }
  void read() {
    uint8_t button_val = digitalRead(pin);  
    button_pressed = !button_val; //invert button
  } 
  int update() {
    read();
    flag = 0;
    if (state==S0) {
      if (button_pressed) {
        state = S1;
        button_change_time = millis();
      }
    } else if (state==S1) {
      if (!button_pressed)
        state = S0, button_change_time = millis();
      else if (millis() - button_change_time >= debounce_duration)
        state = S2, S2_start_time = millis();
    } else if (state==S2) {
      if (!button_pressed)
        state = S4, button_change_time = millis();
      else if (millis() - S2_start_time >= long_press_duration)
        state = S3;
    } else if (state==S3) {
      if (!button_pressed)
        state = S4, button_change_time = millis();
    } else if (state==S4) {
      if (!button_pressed && millis() - button_change_time >= debounce_duration) {
        state = S0;
        if (millis() - S2_start_time < long_press_duration)
          flag = 1;
        else
          flag = 2;
      }
      else if (button_pressed && millis() - S2_start_time < long_press_duration)
        state = S2, button_change_time = millis();
      else if (button_pressed)
        state = S3, button_change_time = millis();
    }
    return flag;
  }
};


char gesture_sequence[1000] = "";

const uint8_t BUTTON1 = 45; // pin connected to button
const uint8_t BUTTON2 = 39; // pin connected to button

Button button1(BUTTON1), button2(BUTTON2);

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200); //begin serial comms
  delay(100); //wait a bit (100 ms)
  Wire.begin();
  delay(50); //pause to make sure comms get set up
  if (imu.setupIMU(1)) {
    Serial.println("IMU Connected!");
  } else {
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
  Serial.println("here");
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
  measure = false;
}

void loop() {
  // put your main code here, to run repeatedly:
    
  imu.readAccelData(imu.accelCount);  
  float x, y, z;
  x = imu.accelCount[0] * imu.aRes;
  y = imu.accelCount[1] * imu.aRes;
  z = imu.accelCount[2] * imu.aRes;
  float acc_mag = sqrt(x * x + y * y + z * z);
  float avg_acc_mag = 1.0 / 3.0 * (acc_mag + old_acc_mag + older_acc_mag);
  older_acc_mag = old_acc_mag;
  old_acc_mag = acc_mag;
  char movement[100] = "";

  averaging_filter(x, left_sequence, 320);    
  averaging_filter(x, right_sequence, 362);
  averaging_filter(y, up_sequence, 463);
  averaging_filter(y, down_sequence, 500);
  // averaging_filter(y, up_sequence);
  // averaging_filter(y, down_sequence);
  
  int input = button1.update();
  int input2 = button2.update();
  // TRAINING STARTS HERE
  // if(input == 2){
  //   iter = 0;
  //   memset(measurements, 0, sizeof(measurements));
  //   memset(str_measurements,0, sizeof(str_measurements));
  //   measure = true;  
      

  // }
  // if (input == 1){
  //   Serial.println("here for some weird reason");

  //   measure = false;
  //   sprintf(json_body, "training=true&direction=down&accel_sequence=");
  //   strcat(json_body, str_measurements);
  //   int body_len = strlen(json_body);
  //   Serial.println(json_body);
  //   post_request_buffer[0] = '\0';
  //   post_response_buffer[0] = '\0';
  //   sprintf(post_request_buffer, "POST http://608dev-2.net/sandbox/sc/team26/server_src/gesture_train.py HTTP/1.1\r\n");
  //   strcat(post_request_buffer, "Host: 608dev-2.net\r\n");
  //   strcat(post_request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  //   char content_leng[50]="";
  //   sprintf(content_leng, "Content-Length: %d\r\n\r\n", body_len);
  //   strcat(post_request_buffer, content_leng);
  //   strcat(post_request_buffer, json_body);
  //   do_http_request("608dev-2.net", post_request_buffer, post_response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
  //   post_response_buffer[strlen(post_response_buffer)-1]='\0';  
  //   memset(json_body, 0, sizeof(json_body));
  // }
  // if(measure){
  //   Serial.println("Measuring");
  //   char measurement[10]="";
  //   sprintf(measurement, "%4.2f,", y);
  //   strcat(str_measurements, measurement);
  //   measurements[iter]=y;
  //   iter+=1;

  // }
  // TRAINING ENDS HERE!!
  // Comparing Correlation starts HERE
  if(input ==1){
    measure = false;
    check_gesture(320, left_sequence, "left", left_response);
    check_gesture(362, right_sequence, "right", right_response);
    check_gesture(500, down_sequence, "down", down_response);
    check_gesture(463, up_sequence, "up", up_response);
    // sprintf(gesture_sequence, post_response_buffer);
    sprintf(left_request, "left=%s", left_response);
    sprintf(right_request, "&right=%s", right_response);
    sprintf(up_request, "&up=%s", up_response);
    sprintf(down_request, "&down=%s", down_response);
    strcat(json_body, left_request);
    strcat(json_body, right_request);
    strcat(json_body, up_request);
    strcat(json_body, down_request);
    int body_len = strlen(json_body);
    Serial.println(json_body);
    post_request_buffer[0] = '\0';
    post_response_buffer[0] = '\0';
    sprintf(post_request_buffer, "POST http://608dev-2.net/sandbox/sc/team26/server_src/get_gesture.py HTTP/1.1\r\n");
    strcat(post_request_buffer, "Host: 608dev-2.net\r\n");
    strcat(post_request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
    char content_leng[50]="";
    sprintf(content_leng, "Content-Length: %d\r\n\r\n", body_len);
    strcat(post_request_buffer, content_leng);
    strcat(post_request_buffer, json_body);
    do_http_request("608dev-2.net", post_request_buffer, post_response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);      
    memset(json_body, 0, sizeof(json_body));
    post_response_buffer[strlen(post_response_buffer)-1]='\0';
    
    if(strcmp(post_response_buffer, "-1")!=0){
      strcat(gesture_sequence, post_response_buffer);
      strcat(gesture_sequence, ",");
    }
    Serial.println(gesture_sequence);
  }  
  
  
  if(input2 ==1){
    Serial.println("here?");
    sprintf(json_body, "authenticate_gesture=true&username=dkriezis&gesture_sequence=");    
    strcat(json_body, gesture_sequence);
    int body_len = strlen(json_body);
    Serial.println(json_body);
    post_request_buffer[0] = '\0';
    post_response_buffer[0] = '\0';
    sprintf(post_request_buffer, "POST http://608dev-2.net/sandbox/sc/team26/server_src/authentication.py HTTP/1.1\r\n");
    strcat(post_request_buffer, "Host: 608dev-2.net\r\n");
    strcat(post_request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
    char content_leng[50]="";
    sprintf(content_leng, "Content-Length: %d\r\n\r\n", body_len);
    strcat(post_request_buffer, content_leng);
    strcat(post_request_buffer, json_body);
    do_http_request("608dev-2.net", post_request_buffer, post_response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);      
    memset(gesture_sequence, 0, sizeof(gesture_sequence));
    memset(json_body, 0, sizeof(json_body));
  }
  // CHECKING correlation ends here
  char output[100];
  // sprintf(output, "X:%4.2f,Y:%4.2f, STEPS:%d", acc_mag, avg_acc_mag, count); //render numbers with %4.2 float formatting
  // Serial.println(output); //print to serial for plotting
  //redraw for use on LCD (with new lines):
  //TOTALLY CHANGE WHAT YOU PRINT HERE. %d will place integer into string
  sprintf(output, "%4.2f  \n%4.2f  \n%4.2f  ", x, y, z); //render numbers with %4.2 float formatting
  // Serial.println(output);
  
  // Serial.println(gesture_sequence);
  // while (millis() - primary_timer < LOOP_PERIOD); //wait for primary timer to increment
  primary_timer = millis();
  

}
void averaging_filter(float input, float* stored_values, int array_size) {
    float temp=0.0;
    float current=stored_values[0];
    for(int i=0; i <array_size; i++){
        temp=stored_values[i+1];
        stored_values[i+1]=current;
        current=temp;
    }
    stored_values[0]=input;
}
char dir_measurements[10000] = "";
void check_gesture(int array_size, float* dir_sequence, char* direction, char* response){
    memset(dir_measurements, 0, sizeof(dir_measurements));
    for (int i = 0; i<array_size; ++i){
      char dir_measurement[5] = "";
      sprintf(dir_measurement, "%4.2f,", dir_sequence[i]);
      strcat(dir_measurements, dir_measurement);
    }
    sprintf(json_body, "check=true&direction=%s&accel_sequence=", direction);    
    Serial.println(strlen(dir_measurements));
    strcat(json_body, dir_measurements);
    int body_len = strlen(json_body);
    Serial.println(json_body);
    post_request_buffer[0] = '\0';
    post_response_buffer[0] = '\0';
    sprintf(post_request_buffer, "POST http://608dev-2.net/sandbox/sc/team26/server_src/gesture_train.py HTTP/1.1\r\n");
    strcat(post_request_buffer, "Host: 608dev-2.net\r\n");
    strcat(post_request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
    char content_leng[50]="";
    sprintf(content_leng, "Content-Length: %d\r\n\r\n", body_len);
    strcat(post_request_buffer, content_leng);
    strcat(post_request_buffer, json_body);
    do_http_request("608dev-2.net", post_request_buffer, post_response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
    post_response_buffer[strlen(post_response_buffer)-1]='\0';
    sprintf(response, post_response_buffer);
    memset(json_body, 0, sizeof(json_body));

}

