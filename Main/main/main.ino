#include <NewPing.h>
#include <Servo.h>

#define TRIGGER_PIN  9
#define ECHO_PIN     9
#define MAX_DISTANCE 200
#define Ref_Distance 7

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
Servo servo_left;
Servo servo_right;

const int pin_Servo_left = 13;       
const int pin_Servo_right = 12;

const int pin_IR_left = A0;       
const int pin_IR_right = A1;

int IR_left = 0;
int IR_right = 0;
int turnleft = 0;
int turnright = 0;
int rechtdoor = 0;
const float alpha = 0.1;
const float beta = 0.0002;
unsigned long StartTime = 0;
float baseSpeed = 0.05;

void move_servos(float baseSpeed, float offset)
{

  float speed_left = -baseSpeed +  offset;
  float speed_right = baseSpeed + offset;

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
  
  servo_right.write(90);
  servo_right.write(90);
}

float ACC(){
	int Distance = sonar.ping_cm();
	
	if(Distance > 49 || Distance == 0){
		// Nothing in front
		return baseSpeed;
	}else if (Distance < 50 && Distance > Ref_Distance + 2){
		// Larger than preffered distance
		return baseSpeed + beta*(Distance - Ref_Distance);
	}else if (Distance < Ref_Distance - 2){
		// too small
		return max(baseSpeed - abs(2*beta*(Distance - Ref_Distance)), 0);
	}else{
		// in the window
		return baseSpeed + 0.1*beta*(Distance - Ref_Distance);
	}
}

void loop()
{    
	baseSpeed = ACC();
	
  // Read from IR sensors
    IR_left = digitalRead(pin_IR_left);
    IR_right = digitalRead(pin_IR_right);

    //Probeersel sturen
    if(IR_right == LOW && turnright > 6){
      turnright = 0;
      move_servos(baseSpeed, 1);
      delay(100);
      while(IR_left == HIGH){
        move_servos(baseSpeed, 0);
        IR_left = digitalRead(pin_IR_left);
        }
    }
    if(IR_left == LOW && turnleft > 6){
      turnleft = 0;
      move_servos(baseSpeed, -alpha);
      delay(100);
      while(IR_right == HIGH){
        move_servos(baseSpeed, 0);
        IR_right = digitalRead(pin_IR_right);
        }
    }

    if(IR_right == LOW){
      turnright = 0;
    }
    if(IR_left == LOW){
      turnleft = 0;
    }
    
    if(IR_left == LOW && IR_right == LOW){
    // If no line is detected
    move_servos(baseSpeed, 0);
    rechtdoor++;
    if(rechtdoor>20){
      move_servos(2*baseSpeed, 0);
      }
    
    }else if (IR_left == HIGH && IR_right == LOW) {
      // if line is detected by left side
      turnleft ++;
      rechtdoor = 0;
      // if StartTime is not set set it
      if(turnleft > 10){
      move_servos(baseSpeed, -2*alpha);
      }
      move_servos(baseSpeed, -alpha);
  
    }else if (IR_left == LOW && IR_right == HIGH) {
      // if line is detected by right side
    turnright ++;
    rechtdoor = 0;
      // if StartTime is not set set it 
    if(turnright > 10){
      move_servos(baseSpeed, 2*alpha);
      }
    move_servos(baseSpeed, alpha);
 
    }else if(IR_left == HIGH && IR_right == HIGH && turnright < 6 && turnleft < 6){
     
    // if both detect a line (consider it as no line for now)
    rechtdoor = 0;
    StartTime = 0;
    move_servos(0, 0);
    delay(1000);
    move_servos(baseSpeed,0);
    delay(500);
    // Intersection protocol
    }
    
    Serial.print("LEFT: ");
    Serial.print(turnleft);
    Serial.print(" RIGHT: ");
    Serial.print(turnright);
    Serial.println();
}
