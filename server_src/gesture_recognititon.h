#ifndef GESTURE_RECOGNITION_H
#define GESTURE_RECOGNITION_H

#include <WiFi.h> //Connect to WiFi Network
#include <string.h>  //used for some string handling and processing.
#include <mpu6050_esp32.h>
#include<math.h>

Class GestureRecognizer
{
    private:
        MPU6050 imu; //imu object called, appropriately, imu
        float old_acc_mag;  //previous acc mag
        float older_acc_mag;  //previous prevoius acc mag
        uint8_t gesture_state = 0;
        const uint8_t REST = 0;
        const uint8_t UP = 1;
        const uint8_t DOWN = 2;
        const uint8_t LEFT = 3;
        const uint8_t RIGHT = 4;

const uint8_t LOOP_PERIOD = 75; //milliseconds
    public:
    
}


#endif // CARD_SCANNER_H
