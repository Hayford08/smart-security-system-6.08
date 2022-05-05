# Introducing Gestures as a way of unlocking a door

The purpose of this deliverable is to add gestures as a way of unlocking a door.

They way this system is supposed to function is the following. You move your esp in one of the following directions (left, right, up, down), you record accelerations during the movement in some direction, which you then push to the server for identification. After recognizing a certain number of movements you press the second button to send the sequence to authenticate with the server.


# Finding the "default" movement accelerations

To get the default acceleration sequence against which I will compute the correlation I do the following.

First of all I sample the x, y, z acceleration every 30 milliseconds.

Then I have connected a button to pin 45 that is a classy button as implemented in our regular exercise.

A long press of the button starts the recording of the acceleration data. A short press ends the recording and sends the sequence to the server to be stored on a database.

The way I start collecting data is the following. When I detect a long press I set a variable called measure to true.

If measure is true, I add acceleration data to an array called measurements.

```cpp
if(measure){
    char measurement[10]="";
    sprintf(measurement, "%4.2f,", x);
    strcat(str_measurements, measurement);
    measurements[iter]=x;
    iter+=1;
  }
```

If short press, we stop measuring, do post request and reset measurements to empty.

In the json body each time I specify a different direction I change the direction part of the request as mentioned in the code snippet below.

Then I do the post request.

```cpp
 if (input == 1){
    measure = false;
    // manually change direction for each direction I try to learn
    sprintf(json_body, "training=true&direction=right&accel_sequence=");
    strcat(json_body, str_measurements);
    // Doing post request here
```

# Server receives default sequence

In order for the server to distinguish between when it is receiving an acceleration training sequence, or an acceleration sequence it has to compare with I include the "training" or a "check" parameter in the post request.

When the server receives a training sequence, it adds it to the database. First I delete a sequence from that database for the same direction and then I update it with the new sequence.


# Capturing an esp movement

To capture a movement with the esp I do the following. I have an array called left_sequence, right_sequence, ...
That has the length of the default sequence stored in the database for each movement.

I update each of these arrays in a way similar to the running average example. Everytime I sample a new acceleration measurement, I add it to the front of the array, and remove the last element of the array.

When I press a short click I send the current state of each sequence to the server to compute the correlation of each direction gesture with the inputed sequence. More specifically, I send a separate post request for each direction, and get the correlation of the stored sequence with respect to the default sequence.

# Recognizing a gesture through correlation - server sides

The server receives the acceleration sequence as a string of acceleration separated by commas. It removes the trailing comma at the end, and then splits at the commas, reverses the list, because the incoming array is in reverse of the actual measurement sequence, and then computes the correlation, as implemented in our regular exercises.



# Authenticating gesture pattern entered

In the first stage of my program, that worked until Tuesday afternoon, I could recognize only the left movements. I set a correlation threshold to 0.3 and if correlation was larger than 0.3, I classified the gesture as a left movement, and the server returned the string left to the esp which then added it to a gesture_sequence variable.

Then I added a button connected to pin input 39. When I short pressed the button I sent the gesture_sequence string to the server to authenticate. 

More specifically, in our users database, each user has a gesture passcode. I queried the geture passcode for each user, and compared it with the gesture_sequence provided by the esp. If they are the same, then I return true to the esp, false otherwise.

# Complications and issues

The main issue I encountered is the following. Tuesday morning my code was able to somewhat recognize a left gesture.

As a result, I decided to expand my code to recognize a right, up, down movement. However, when I tried to distinguish between left and right movements, the correlations were messed up. I moved the esp to the left, and it showed a higher correlation with the right than with the left sequence.

I gave my code to two TAs to train the sequence for me, in case I did something wrong, and experiment but they couldn't get it to work.

I also talked to Professor Steinmeyer Tuesday night at OH briefly, because he had to go to a group meeting but he couldn't figure out what was wrong. What he recommended, and what I have been working on throughout Wednesday, was sampling acceleration at a faster rate. However, I have been coming across a lot of memory issues, since more sampled data mean larger arrays, so I haven't gotten around to making it work with more data points.

# Gluing it all together 

# Complications in card reading

In trying to to glue everyting together we have come across a very unexpected problem. Sometimes the rfid can recognize a card, other times it doesn't recognize anything. 

[picture of it working]