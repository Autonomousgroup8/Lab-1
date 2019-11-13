#include <Servo.h>

Servo servoLeft;
Servo servoRight;

const int rightServoPin = 13;       
const int leftServoPin = 12;

void move_servos(int speed_left, int speed_right)
{
    servoLeft.write(180 - speed_left);
    servoRight.write(speed_right);
}

void setup() 
{
  Serial.begin(9600);
  servoLeft.attach(leftServoPin);
  servoRight.attach(rightServoPin);
  
  // Stop the servo motors
  servoLeft.write(90);
  servoRight.write(90);
}

int counter = 0;

void loop()
{
  int ThrottleR, ThrottleL;
  //Start driving (Max throttle is 180, 90 is standstill)
  ThrottleL = 180;
  ThrottleR = 180;
  
  //if right sensor detects line, steer left
  //ThrottleL = ThrottleL-5;

    
  //if left sensor detects line, steer right
  //THrottleR = ThrottleR-5;

  //if both sensors detect line, drive straight (do nothing)

  //if both sensors do not detect a line, drive straight (do nothing)

  move_servos(ThrottleL, ThrottleR);
}
