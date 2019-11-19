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

const float alpha = 0.001;
unsigned long StartTime = 0;
unsigned long uncertainTime = 0;
float baseSpeed = 0.5;

const int speedFilterLength = 10;
const int steeringFilterLength = 10;
float speed[speedFilterLength];
float steering[steeringFilterLength];
int speedIndex = 0;
int steeringIndex = 0;

void move_servos(float baseSpeed, float offset)
{
	float speed_left = baseSpeed + offset;
	float speed_right = -baseSpeed + offset;

	speed_left = constrain(speed_left, -1, 1);
	speed_right = constrain(speed_right, -1, 1);
		
    servo_left.write(90+speed_left*90);
    servo_right.write(90+speed_right*90);
}

float avg(float array[], int length){
	int sum = 0;
	
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
	
	for(int i = 0; i < steeringFilterLength; i++){
		speed[i] = baseSpeed;
	}
	
	for(int i = 0; i < speedFilterLength; i++){
		steering[i] = 0;
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
		speed[speedIndex] = baseSpeed;
		steering[steeringIndex] = 0;
		
	}else if (IR_left > leftThreshold && IR_right < rightThreshold) {
		// Line detected left		
	
		speed[speedIndex] = 0.05;
		steering[steeringIndex] = -1;

	}else if (IR_left < leftThreshold && IR_right > rightThreshold) {
		// Line detected right
		
		speed[speedIndex] = 0.05;
		steering[steeringIndex] = 1;
		
	}else if(IR_left > leftThreshold && IR_right > rightThreshold){
		// Line detected both left right
		
		speed[speedIndex] = baseSpeed;
		steering[steeringIndex] = 0;
    }
	
	move_servos(avg(speed, speedFilterLength), avg(steering, steeringFilterLength));
	
	speedIndex = (speedIndex + 1) % speedFilterLength;
	steeringIndex = (steeringIndex + 1) % steeringFilterLength;
}
