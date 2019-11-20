#include <Servo.h>
#include <NewPing.h>

#define MAX_DISTANCE 200

#define TRIGGER_PIN  9
#define ECHO_PIN     9
#define pin_Servo_left 13
#define pin_Servo_right 12
#define pin_IR_left 0
#define pin_IR_right 1

#define threshold 700

Servo servo_left;
Servo servo_right;

int IR_left = 0;
int IR_right = 0;
int leftThreshold;
int rightThreshold;

float baseSpeed = 0.5;

const int FilterLength = 5;
float left[FilterLength];
float right[FilterLength];
int FilterIndex = 0;


void move_servos(float speed_left, float speed_right)
{
	speed_left = constrain(speed_left, -1, 1);
	speed_right = constrain(speed_right, -1, 1);
		
    servo_left.write(90+speed_left*90);
    servo_right.write(90+speed_right*90);
}

float avg(float array[], int length){
	float sum = 0;
	
	for(int i = 0; i<length; i++){
		sum += array[i];
	}
	
	return sum/length;
}

void setup() 
{
	Serial.begin(9600);
	servo_left.attach(pin_Servo_left);
	servo_right.attach(pin_Servo_right);
	
	// Setup threshold
	leftThreshold = analogRead(pin_IR_left) + 50;
	rightThreshold = analogRead(pin_IR_right) + 50;
	
	for(int i = 0; i < FilterLength; i++){
		left[i] = baseSpeed;
		right[i] = baseSpeed;
	}
	
	// Init servo motors with 0
	servo_right.write(90);
	servo_right.write(90);
}

void loop()
{    
	// Read from IR sensors
    IR_left = analogRead(pin_IR_left);
    IR_right = analogRead(pin_IR_right);
    
    if(IR_left < leftThreshold && IR_right < rightThreshold){
		// No line detected
		left[FilterIndex] = 1;
		right[FilterIndex] = 1;
		
	}else if (IR_left > leftThreshold && IR_right < rightThreshold) {
		// Line detected left		
	
		left[FilterIndex] = -1;
		right[FilterIndex] = 1;

	}else if (IR_left < leftThreshold && IR_right > rightThreshold) {
		// Line detected right
		
		left[FilterIndex] = -1;
		right[FilterIndex] = 1;
		
	}else if(IR_left > leftThreshold && IR_right > rightThreshold){
		// Line detected both left right
		
		left[FilterIndex] = 0;
		right[FilterIndex] = 0;
    }
	
	move_servos(avg(left, FilterLength), avg(right, FilterLength));
	
	FilterIndex = (FilterIndex + 1) % FilterLength;
}
