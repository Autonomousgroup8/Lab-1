#include <NewPing.h>
#include <Servo.h>

#define TRIGGER_PIN  9
#define ECHO_PIN     9
#define MAX_DISTANCE 20
#define MIN_DISTANCE 10
#define SONAR_DISTANCE 200

NewPing sonar(TRIGGER_PIN, ECHO_PIN, SONAR_DISTANCE);
Servo servo_left;
Servo servo_right;

const int pin_Servo_left = 13;       
const int pin_Servo_right = 12;

const int pin_IR_left = 0;       
const int pin_IR_right = 1;

int IR_left = 0;
int IR_right = 0;

const float base_speed = 67.5;
const int idealDistance = (MAX_DISTANCE + MIN_DISTANCE)/2;
const int threshhold = 700;
const float alpha = 0.001;
unsigned long StartTime = 0;
float baseSpeed = 0.5;

void move_servos(float baseSpeed, float offset)
{
  float speed_left = baseSpeed + offset;
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
  
  // Init servo motors with 0
  servo_right.write(90);
  servo_right.write(90);
}

void loop()
{    
  int Distance;
  Distance = sonar.ping_cm();
  if(Distance < MIN_DISTANCE){
    baseSpeed = 90; //Stand still if too close
  }
  else if (Distance > MAX_DISTANCE || Distance == 0){
    baseSpeed = 180; //Maximum speed if too far away
  }
  else{
    baseSpeed = 90 + (Distance - idealDistance + MIN_DISTANCE/2)*9; //Variable speed if inbetween
  }
baseSpeed = -((baseSpeed)/90 - 1) ;
  
  // Read from IR sensors
    IR_left = analogRead(pin_IR_left);
    IR_right = analogRead(pin_IR_right);
    
    if(IR_left < threshhold && IR_right < threshhold){
    // If no line is detected
    
    StartTime = 0;
        move_servos(baseSpeed, 0);
    
  }else if (IR_left > threshhold && IR_right < threshhold) {
    // if line is detected by left side
    
    // if StartTime is not set set it
    if(!StartTime){
      StartTime = millis();
    }
    
    move_servos(baseSpeed, -alpha*(millis() - StartTime));
    
  }else if (IR_left < threshhold && IR_right > threshhold) {
    // if line is detected by right side
    
    // if StartTime is not set set it
    if(!StartTime){
      StartTime = millis();
    }
    
    move_servos(baseSpeed, alpha*(millis() - StartTime));
    
  }else if(IR_left > threshhold && IR_right > threshhold){
    // if both detect a line (consider it as no line for now)
    
    StartTime = 0;
    move_servos(baseSpeed, 0);
      // Intersection protocol
    }
}
