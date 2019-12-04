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

int incomingByte = 0;
int IR_left = 0;
int IR_right = 0;
int turnleft = 0;
int turnright = 0;
int rechtdoor = 0;
int prevCross = 0;
const float alpha = 0.3;
const float beta = 0.002;
float baseSpeed = 0.05;
float ACCSpeed = 0.05;
int headTurn = 0;

//Zigbee implementation
#define SELF     43
#define PAN_ID           "A008"
#define CHANNEL_ID       "0F"
bool waitMode = false;
int crossingsPassed = 0;

// some macros needed for the xbee_init function. Do not touch :-).
#define STRING(name) #name
#define TOSTRING(x) STRING(x)

// the xbee_init function initializes the XBee Zigbee module
void xbee_init(void) {
  Serial.begin(9600);                         // set the baud rate to 9600 to match the baud rate of the xbee module
  Serial.flush();                             // make sure the buffer of the serial connection is empty
  Serial.print("+++");                        // sending the characters '+++' will bring the XBee module in its command mode (see https://cdn.sparkfun.com/assets/resources/2/9/22AT_Commands.pdf)
  delay(2000);                                // it will only go in command mode if there is a long enough pause after the '+++' characters. Wait two seconds.
  Serial.print("ATCH " CHANNEL_ID "\r");      // set the channel to CHANNEL_ID
  Serial.print("ATID " PAN_ID "\r");          // set the network PAN ID to PAN_ID
  Serial.print("ATMY " TOSTRING(SELF) "\r");  // set the network ID of this module to SELF
  Serial.print("ATDH 0000\rATDL FFFF\r");     // configure the modue to broadcast all messages to all other nodes in the PAN
  Serial.print("ATCN\r");                     // exit command mode and return to transparent mode, communicate all data on the serial link onto the wireless network
}

void move_servos(float baseSpeed, float offset) {
  float speed_left = -baseSpeed +  offset;
  float speed_right = baseSpeed + offset;

  speed_left = constrain(speed_left, -1, 1);
  speed_right = constrain(speed_right, -1, 1);

  servo_left.write(90 + speed_left * 90);
  servo_right.write(90 + speed_right * 90);
}

void setup()
{
  xbee_init();
  Serial.begin(9600);
  Serial.println("This is the XBee - Broadcast program.");
  servo_left.attach(pin_Servo_left);
  servo_right.attach(pin_Servo_right);
  servo_head.attach(pin_Servo_head);
  servo_left.write(90);
  servo_right.write(90);
}

float ACC() {                                                   //active cruise control algorithm, determines speed
  int Distance = sonar.ping_cm();
  //  Serial.println(Distance);
  if (Distance > 49 || Distance == 0) {                         // Nothing in front, you are the leader
    return ACCSpeed;
  } else if (Distance < 50 && Distance > Ref_Distance + 5) {    // Larger than preffered distance, gradruately speed up
    return ACCSpeed + beta * (Distance - Ref_Distance);
  } else if (Distance < Ref_Distance - 3) {                     // too small, gradruatly slow down
    return max(ACCSpeed - abs(3 * beta * (Distance - Ref_Distance))+0.01, 0.01) - 0.01;
  } else {                                                      // in the window, only make small adjustments.
    return ACCSpeed + 0.1 * beta * (Distance - Ref_Distance);
  }
}

void loop()
{
  if (!waitMode) {
    baseSpeed = ACC();                          //determine speed with Active cruise control.
    Serial.println(baseSpeed);

    // Read from IR sensors
    IR_left = digitalRead(pin_IR_left);
    IR_right = digitalRead(pin_IR_right);

    if (IR_left == LOW && IR_right == LOW) {      // If no line is detected
      rechtdoor++;
      if (prevCross == 1) {
        crossingsPassed++;
        prevCross = 0;
      }

      if (rechtdoor > 80) {                       //increase speed on long straights
        move_servos(2 * baseSpeed, 0);
        if (baseSpeed > 0.03) {
          headTurn = 0;
        }
      } else {
        move_servos(baseSpeed, 0);
      }
      //update average for head direction
    } else if (IR_left == HIGH && IR_right == LOW) { // if line is detected by left side
      turnleft ++;
      rechtdoor = 0;
      if (turnleft > 10) {                         //on sharp corners turn faster
        move_servos(baseSpeed, -2 * alpha);
      } else {
        move_servos(baseSpeed, -alpha);
      }
      if (turnleft > 3) {
        headTurn = -45;
      } else {
        headTurn = -10;
      }

    } else if (IR_left == LOW && IR_right == HIGH) {    // if line is detected by right side
      turnright ++;
      rechtdoor = 0;
      if (turnright > 10) {                         //on sharp corners turn faster
        move_servos(baseSpeed, 2 * alpha);
      } else {
        move_servos(baseSpeed, alpha);
      }

      if (turnright > 3) {
        headTurn = 45;
      } else {
        headTurn = 10;
      }
    } else if (IR_left == HIGH && IR_right == HIGH) {
      //If I am first robot wait 10 seconds at line. If I am not the first 'head' robot continue driving
      rechtdoor = 0;
      //waitMode = true;
      prevCross = 1;
      move_servos(baseSpeed, 0);
    }
  } else if (waitMode == true) {
    if (Serial.available() > 0) {
      incomingByte = Serial.read();
      if (incomingByte == 55) {
        waitMode = false;
        move_servos(baseSpeed, 0);
        delay(200);
      }
    }
  }
  servo_head.write(90 + headTurn); //turn head in turning direction
}
