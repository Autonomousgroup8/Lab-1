#include <NewPing.h>
#include <Servo.h>

#define TRIGGER_PIN  9
#define ECHO_PIN     9
#define MAX_DISTANCE 200
#define Ref_Distance 13

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
Servo servo_left;
Servo servo_right;
Servo servo_head;

const int pin_Servo_left = 13;
const int pin_Servo_right = 12;
const int pin_Servo_head = 11;

const int pin_IR_left = A0;
const int pin_IR_right = A1;

int IR_left = 0;
int IR_right = 0;
int turnleft = 0;
int turnright = 0;
int rechtdoor = 0;
const float alpha = 0.15;
const float beta = 0.002;
const float gamma = 10;
unsigned long StartTime = 0;
float baseSpeed = 0.05;
float ACCSpeed = 0.05;
const int FilterLength = 10;
int TurningAverage[FilterLength];
int FilterIndex = 0;

float Average(int Array[], int length) {          //floating average over 10 cycles
  int sum = 0;
  for (int i = 0; i < length; i++) {              //sum all elements of Array
    sum += Array[i];
  }

  return sum / length;                            //calculate average of Array
}

void move_servos(float baseSpeed, float offset)   //mpve servos
{

  float speed_left = -baseSpeed +  offset;
  float speed_right = baseSpeed + offset;

  speed_left = constrain(speed_left, -1, 1);
  speed_right = constrain(speed_right, -1, 1);

  servo_left.write(90 + speed_left * 90);
  servo_right.write(90 + speed_right * 90);
}

void setup()
{
  Serial.begin(9600);
  servo_left.attach(pin_Servo_left);              //attach all servos
  servo_right.attach(pin_Servo_right);
  servo_head.attach(pin_Servo_head);

  servo_left.write(90);
  servo_right.write(90);
}

float ACC() {                                                   //active cruise control algorithm, determines speed
  int Distance = sonar.ping_cm();
  Serial.println(Distance);
  if (Distance > 49 || Distance == 0) {                         // Nothing in front, you are the leader
    return ACCSpeed;
  } else if (Distance < 50 && Distance > Ref_Distance + 5) {    // Larger than preffered distance, gradruately speed up
    return ACCSpeed + beta * (Distance - Ref_Distance);
  } else if (Distance < Ref_Distance - 3) {                     // too small, gradruatly slow down
    return max(ACCSpeed - abs(3 * beta * (Distance - Ref_Distance)), 0);
  } else {                                                      // in the window, only make small adjustments.
    return ACCSpeed + 0.1 * beta * (Distance - Ref_Distance);
  }
}

void loop()
{
  baseSpeed = ACC();                          //determine speed with Active cruise control.
  Serial.print(baseSpeed);
  // Read from IR sensors
  IR_left = digitalRead(pin_IR_left);
  IR_right = digitalRead(pin_IR_right);

  if (IR_right == LOW && turnright > 6) {     //sharp corner protocol right
    turnright = 0;
    move_servos(baseSpeed, 1);
    delay(100);
    while (IR_left == HIGH) {
      move_servos(baseSpeed, 0);
      IR_left = digitalRead(pin_IR_left);
    }
  }
  if (IR_left == LOW && turnleft > 6) {     //sharp corner protocol right
    turnleft = 0;
    move_servos(baseSpeed, -alpha);
    delay(100);
    while (IR_right == HIGH) {
      move_servos(baseSpeed, 0);
      IR_right = digitalRead(pin_IR_right);
    }
  }

  if (IR_right == LOW) {                    //reset consequetive right turns counter
    turnright = 0;
  }
  if (IR_left == LOW) {                    //reset consequetive left turns counter
    turnleft = 0;
  }

  if (IR_left == LOW && IR_right == LOW) {      // If no line is detected
    move_servos(baseSpeed, 0);
    rechtdoor++;
    if (rechtdoor > 20) {                       //increase speed on long straights
      move_servos(2 * baseSpeed, 0);
    }
    TurningAverage[FilterIndex] = 0;           //update average for head direction
  }
  else if (IR_left == HIGH && IR_right == LOW) { // if line is detected by left side
    turnleft ++;
    rechtdoor = 0;
    if (turnleft > 10) {                         //on sharp corners turn faster
      move_servos(baseSpeed, -2 * alpha);
    }
    move_servos(baseSpeed, -alpha);
    TurningAverage[FilterIndex] = -1;           //update average for head direction

  } else if (IR_left == LOW && IR_right == HIGH) {    // if line is detected by right side
    turnright ++;
    rechtdoor = 0;
    if (turnright > 10) {                         //on sharp corners turn faster
      move_servos(baseSpeed, 2 * alpha);
    }
    move_servos(baseSpeed, alpha);
    TurningAverage[FilterIndex] = 1;           //update average for head direction
  } else if (IR_left == HIGH && IR_right == HIGH && turnright < 6 && turnleft < 6) {    // if both detect a line (consider it as no line for now, different in lab2)
    rechtdoor = 0;
    StartTime = 0;
    move_servos(0, 0);
    delay(1000);
    move_servos(baseSpeed, 0);
    delay(500);
    TurningAverage[FilterIndex] = 0;           //update average for head direction
    // Intersection protocol
  }

  FilterIndex = (FilterIndex + 1) % FilterLength; //cycle through filter
  servo_head.write(90 + Average(TurningAverage, FilterLength)*gamma); //turn head in turning direction

  //    Serial.print("LEFT: ");
  //    Serial.print(turnleft);
  //    Serial.print(" RIGHT: ");
  //    Serial.print(turnright);
  //    Serial.println();
}
