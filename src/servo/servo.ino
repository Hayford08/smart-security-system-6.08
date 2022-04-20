int pos;
int pos_min = int(0.05*1024);
int pos_max = int(0.1*1024);
int servoPin = 21;
int buttonPin = 45;
int pwm_channel = 0;
bool doorOpen = false;
bool buttonPressed = false;
bool toChange = false;
int pinMeasurement = -1;
void setup() {
  ledcSetup(pwm_channel, 50, 10);
  ledcAttachPin(servoPin, pwm_channel);
  pinMode(buttonPin,INPUT_PULLUP);
}

void loop() {
  pinMeasurement = digitalRead(buttonPin);
  if (pinMeasurement == 0){
    buttonPressed = true;
    toChange = true;
  }
  else{
    buttonPressed = false;
  }
  if(toChange){
    if(!doorOpen){
      for (pos = pos_min; pos <= pos_max; pos += 1) { // goes from 0 degrees to 180 degrees
        // in steps of 1 degree
        ledcWrite(pwm_channel, pos);    // tell servo to go to position in variable 'pos'
        delay(50);             // waits 15ms for the servo to reach the position
      }
    }
    else{
      for (pos = pos_max; pos >= pos_min; pos -= 1) { // goes from 180 degrees to 0 degrees
        ledcWrite(pwm_channel, pos);    // tell servo to go to position in variable 'pos'
        delay(50);             // waits 15ms for the servo to reach the position
      }
    }
    toChange = false;
    doorOpen = !doorOpen;
  }
  
}
