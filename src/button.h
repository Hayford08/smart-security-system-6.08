#ifndef BUTTON_H
#define BUTTON_H

//enum for button states
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

#endif // BUTTON_H
