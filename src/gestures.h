#ifndef GESTURES_H
#define GESTURES_H

#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h> //Used in support of TFT Display
#include <string.h>  //used for some string handling and processing.
#include <mpu6050_esp32.h>
#include<math.h>
#include "button.h"
#include "support_functions.h"

class Gestures {
private:
    MPU6050* imu; //imu object called, appropriately, imu
    float old_acc_mag;  //previous acc mag
    float older_acc_mag;  //previous prevoius acc mag
    uint8_t gesture_state = 0;
    const uint8_t REST = 0;
    const uint8_t UP = 1;
    const uint8_t DOWN = 2;
    const uint8_t LEFT = 3;
    const uint8_t RIGHT = 4;

    char left_response[22]="";
    char right_response[22]="";
    char up_response[22]="";
    char down_response[22]="";

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

    static const uint16_t IN_BUFFER_SIZE = 3500; //size of buffer to hold HTTP request
    static const uint16_t OUT_BUFFER_SIZE = 100; //size of buffer to hold HTTP response
    char post_request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
    char post_response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

    //float measurements[5000];
    //char str_measurements[6000] = "";
    int iter = 0;
    bool measure;
    char json_body[3000] = "";
    float left_sequence[500];
    float right_sequence[500];
    float up_sequence[500];
    float down_sequence[501];
    char dir_measurements[3000] = "";
    char gesture_sequence[100] = "";

    const uint8_t BUTTON1 = 45; // pin connected to button
    const uint8_t BUTTON2 = 39; // pin connected to button

    Button button1 = Button(BUTTON1), button2 = Button(BUTTON2);

public:
    void setup(MPU6050* imu) {
        this->imu = imu;
        measure = false;
    }

    void reset() {
      measure = false;
      memset(gesture_sequence, 0, sizeof(gesture_sequence));
      memset(json_body, 0, sizeof(json_body));  
    }

    void record(char* gestures) {
        // put your main code here, to run repeatedly:
        
        imu->readAccelData(imu->accelCount);  
        float x, y, z;
        x = imu->accelCount[0] * imu->aRes;
        y = imu->accelCount[1] * imu->aRes;
        z = imu->accelCount[2] * imu->aRes;
        // float acc_mag = sqrt(x * x + y * y + z * z);
        // float avg_acc_mag = 1.0 / 3.0 * (acc_mag + old_acc_mag + older_acc_mag);
        // older_acc_mag = old_acc_mag;
        // old_acc_mag = acc_mag;
        char movement[100] = "";

        averaging_filter(x, left_sequence, 320);    
        averaging_filter(x, right_sequence, 362);
        averaging_filter(y, up_sequence, 463);
        averaging_filter(y, down_sequence, 500);
        
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

        if (input == 2) {
          reset();
        }

        strcpy(gestures, gesture_sequence);
    
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
};
#endif // GESTURES_H
