#include <NewPing.h>
#include <Servo.h>

#define TRIGGER_PIN  9
#define ECHO_PIN     9
#define MAX_DISTANCE 200

Servo servo_left;
Servo servo_right;

const int pin_Servo_left = 13;       
const int pin_Servo_right = 12;

const int pin_IR_left = 0;       
const int pin_IR_right = 1;

int IR_left = 0;
int IR_right = 0;
int leftThreshold;
int rightThreshold;
int turnleft = 0;
int turnright = 0;

const float alpha = 0.005;
unsigned long StartTime = 0;
float baseSpeed = -0.05;

void move_servos(float baseSpeed, float offset)
{

  float speed_left = baseSpeed +  offset;
  float speed_right = -baseSpeed + offset;

  speed_left = constrain(speed_left, -1, 1);
  speed_right = constrain(speed_right, -1, 1);
      
    servo_left.write(90+speed_left*90);
    servo_right.write(90+speed_right*90);
}

void setup() 
{
  Serial.begin(9600);
  servo_left.attach(pin_Servo_left);
  servo_right.attach(pin_Servo_right);

  IR_left = analogRead(pin_IR_left);
  IR_right = analogRead(pin_IR_right);

  leftThreshold = IR_left + 50;
  rightThreshold = IR_right + 50;
  
  // Init servo motors with 0
  servo_right.write(90);
  servo_right.write(90);
}

void loop()
{    
  // Read from IR sensors
    IR_left = analogRead(pin_IR_left);
    IR_right = analogRead(pin_IR_right);

    //Probeersel sturen
    if(IR_right < rightThreshold){
      turnright = 0;
      }else if(IR_left < leftThreshold){
        turnleft = 0;
        }
  
    
    if(IR_left < leftThreshold && IR_right < rightThreshold){
    // If no line is detected
    StartTime = 0;
    move_servos(baseSpeed, 0);
    
    }else if (IR_left > leftThreshold && IR_right < rightThreshold && turnright == 0) {
    // if line is detected by left side
    turnleft = 1;
    // if StartTime is not set set it
    if(!StartTime){
      StartTime = millis();
    }
    move_servos(baseSpeed, -alpha*(millis() - StartTime));
  
    
    }else if (IR_left < leftThreshold && IR_right > rightThreshold && turnleft == 0) {
    // if line is detected by right side
    turnright = 1;
    // if StartTime is not set set it
    if(!StartTime){
      StartTime = millis();
    }

    move_servos(baseSpeed, alpha*(millis() - StartTime));
 
    
    }else if(IR_left > leftThreshold && IR_right > rightThreshold){
    // if both detect a line (consider it as no line for now)
    
    StartTime = 0;
    move_servos(baseSpeed, 0);
      // Intersection protocol
    }
    Serial.print("Left sensor value: ");
    Serial.print(IR_left);
    Serial.print("; right sensor value: ");
    Serial.print(IR_right);
    Serial.println();
}
