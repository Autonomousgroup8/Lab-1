#include <Servo.h>
#include <NewPing.h>

#define TRIGGER_PIN  9
#define ECHO_PIN     9
#define MAX_DISTANCE 200
#define pin_Servo_left 13;
#define pin_Servo_right 12;
#define pin_IR_left 0;
#define pin_IR_right 1;
#define threshold 700;

Servo servo_left;
Servo servo_right;

int IR_left = 0;
int IR_right = 0;

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
	// Read from IR sensors
    IR_left = analogRead(pin_IR_left);
    IR_right = analogRead(pin_IR_right);
    
    if(IR_left < threshold && IR_right < threshold){
		// If no line is detected
		
		StartTime = 0;
      	move_servos(baseSpeed, 0);
		
	}else if (IR_left > threshold && IR_right < threshold) {
		// if line is detected by left side
		
		// if StartTime is not set set it
		if(!StartTime){
			StartTime = millis();
		}
		
		move_servos(baseSpeed, -alpha*(millis() - StartTime));
		
	}else if (IR_left < threshold && IR_right > threshold) {
		// if line is detected by right side
		
		// if StartTime is not set set it
		if(!StartTime){
			StartTime = millis();
		}
		
		move_servos(baseSpeed, alpha*(millis() - StartTime));
		
	}else if(IR_left > threshold && IR_right > threshold){
		// if both detect a line (consider it as no line for now)
		
		StartTime = 0;
		move_servos(baseSpeed, 0);
      // Intersection protocol
    }
}