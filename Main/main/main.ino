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
int sum = 0;
int average = 0;
float headpos = 0;



int sign(int x) {
  if (x < 0) return -1;
  else if (x > 0) return 1;
  else return 0;
}

float movingAverageTurn(int turn) {

  if (turn = !0) {
    sum++;
  }
  else {
    sum = sum - sign(sum);
  }
  average = sum / 10;
  return average;
}

void move_servos(float baseSpeed, float offset)
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
  servo_left.attach(pin_Servo_left);
  servo_right.attach(pin_Servo_right);
  servo_head.attach(pin_Servo_head);

  servo_left.write(90);
  servo_right.write(90);
}

float ACC() {
  int Distance = sonar.ping_cm();
  Serial.println(Distance);
  if (Distance > 49 || Distance == 0) {
    // Nothing in front, you are the leader
    return ACCSpeed;
  } else if (Distance < 50 && Distance > Ref_Distance + 5) {
    // Larger than preffered distance, gradruately speed up
    return ACCSpeed + beta * (Distance - Ref_Distance);
  } else if (Distance < Ref_Distance - 3) {
    // too small, gradruatly slow down
    return max(ACCSpeed - abs(3 * beta * (Distance - Ref_Distance)), 0);
  } else {
    // in the window, only make small adjustments.
    return ACCSpeed + 0.1 * beta * (Distance - Ref_Distance);
  }
}

void loop()
{
  baseSpeed = ACC();
  Serial.print(baseSpeed);

  // Read from IR sensors
  IR_left = digitalRead(pin_IR_left);
  IR_right = digitalRead(pin_IR_right);

  //Probeersel sturen
  if (IR_right == LOW && turnright > 6) {
    turnright = 0;
    move_servos(baseSpeed, 1);
    delay(100);
    while (IR_left == HIGH) {
      move_servos(baseSpeed, 0);
      IR_left = digitalRead(pin_IR_left);
    }
  }
  if (IR_left == LOW && turnleft > 6) {
    turnleft = 0;
    move_servos(baseSpeed, -alpha);
    delay(100);
    while (IR_right == HIGH) {
      move_servos(baseSpeed, 0);
      IR_right = digitalRead(pin_IR_right);
    }
  }

  if (IR_right == LOW) {
    turnright = 0;
  }
  if (IR_left == LOW) {
    turnleft = 0;
  }

  if (IR_left == LOW && IR_right == LOW) {
    // If no line is detected
    move_servos(baseSpeed, 0);
    rechtdoor++;
    if (rechtdoor > 20) {
      move_servos(2 * baseSpeed, 0);
    }
    headpos = movingAverageTurn(0);
  }
  else if (IR_left == HIGH && IR_right == LOW) {
    // if line is detected by left side
    turnleft ++;
    rechtdoor = 0;
    // if StartTime is not set set it
    if (turnleft > 10) {
      move_servos(baseSpeed, -2 * alpha);
    }
    move_servos(baseSpeed, -alpha);
    headpos = movingAverageTurn(-1);

  } else if (IR_left == LOW && IR_right == HIGH) {
    // if line is detected by right side
    turnright ++;
    rechtdoor = 0;
    // if StartTime is not set set it
    if (turnright > 10) {
      move_servos(baseSpeed, 2 * alpha);
    }
    move_servos(baseSpeed, alpha);
    headpos = movingAverageTurn(1);
  } else if (IR_left == HIGH && IR_right == HIGH && turnright < 6 && turnleft < 6) {

    // if both detect a line (consider it as no line for now)
    rechtdoor = 0;
    StartTime = 0;
    move_servos(0, 0);
    delay(1000);
    move_servos(baseSpeed, 0);
    delay(500);
    headpos = movingAverageTurn(0);
    // Intersection protocol
  }
  servo_head.write(90+headpos*gamma);

  //    Serial.print("LEFT: ");
  //    Serial.print(turnleft);
  //    Serial.print(" RIGHT: ");
  //    Serial.print(turnright);
  //    Serial.println();
}
