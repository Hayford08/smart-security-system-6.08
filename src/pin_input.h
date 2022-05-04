#ifndef PIN_INPUT_H
#define PIN_INPUT_H

class PinInputProcessor
{
private:
  char current[100], text[100];
  bool valid = false;
  int NUM_COUNT = 10; // 62;
  int counter = 0, ptr = 0;
  float ANG_THRESHOLD = 1.0; // TODO CHANGE THIS
  int last_update = 0;
  int TIMEOUT = 100;
  Button button = Button(0);

public:
  PinInputProcessor(int BUTTON = 0)
  {
    button = Button(BUTTON);
    memset(text, 0, sizeof(text));
    memset(current, 0, sizeof(current));
  }

  char getNum(int id)
  {
    return (char)('0' + id);
  }

  void update(int x)
  {
    if (!valid)
    {
      if (abs(x) > ANG_THRESHOLD && millis() - last_update > TIMEOUT)
      {
        last_update = millis();
        if (x > 0)
          counter++;
        if (x < 0)
          counter--;
        counter = (counter + NUM_COUNT) % NUM_COUNT;
      }
    }
    current[ptr] = getNum(counter);

    int input = button.update();
    if (valid && input)
    {
      valid = false;
      memset(text, 0, sizeof(text));
      memset(current, 0, sizeof(current));
      ptr = 0;
      counter = 0;
      return;
    }
    if (input == 1)
    {
      current[ptr++] = getNum(counter);
      counter = 0;
    }
    else if (input == 2)
    {
      valid = true;
      current[ptr] = 0;
      memcpy(text, current, sizeof(current));
    }
  }

  bool isValid()
  {
    return valid;
  }

  char *getText()
  {
    return text;
  }

  char *getCurrentText()
  {
    return current;
  }

  char getCurrentChar()
  {
    return getNum(counter);
  }
};

#endif // TEXT_INPUT_H
