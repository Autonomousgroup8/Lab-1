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

const int pin_left_wheel = 8;
const int pin_right_wheel = 7;
const int pin_Servo_left = 13;
const int pin_Servo_right = 12;
const int pin_Servo_head = 11;

const int pin_IR_left = A0;
const int pin_IR_right = A1;

int left_wheel = 0;
int right_wheel = 0;
int leftCount = 0;
int rightCount = 0;
int leftWait = 0;
int rightWait = 0;

int serial = 0;
int incomingByte = 0;
int IR_left = 0;
int IR_right = 0;
int turnleft = 0;
int turnright = 0;
int rechtdoor = 0;
int prevCross = 0;
int communication = 0;
const float alpha = 0.3;
const float beta = 0.002;
float baseSpeed = 0.05;
float ACCSpeed = 0.05;
int headTurn = 0;
int Slave = 0;
int trash = 0;
char ID = 6;
//Variables for communication
int iter = 0;
int receivedID = 0;
char endMarker = '\n';
const byte numChars = 8;
char receivedChars[numChars];
char Direction; //Forward, left, right
int Duration; //Distance in number of blocks of grid
char DurationChar;
int curTime;
int startTime;
int passedTime;
bool commandExcecuted = false;
int driveTime;

//Zigbee implementation
#define SELF     0 //set to 0,1 or 2, different per robot
#define PAN_ID           "A008"
#define CHANNEL_ID       "0D"
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

int getMessage() {
  //Enter messages using the format 'xIDMessage', where x is either 0 for debug or 1 for real message, ID is the ID of the sending robot and then the Message
  if (Serial.available() > 0) {               //A message is available
    int startBit = Serial.read();
    switch (startBit) {
      case 48 :                           //48 = ascii for 0, debug message
        while (Serial.read() >= 0) { }
        return 1;                       //return a 1 that indicates that it is a debug message
        break;
      case 49:                            //49 = ascii for 1, this message is relevant
        receivedID = Serial.read();
        for (int j = 0; j < numChars; j++) {
          receivedChars[j] = ' ';
        }
        char tempChar;
        iter = -1;
        while (Serial.available() > 0) {
          tempChar = Serial.read();
          iter++;
          if (tempChar != endMarker && iter < numChars) {
            receivedChars[iter] = tempChar;
          }
          else {
            break;
          }
        }
        receivedChars[iter + 1] = '\0';
        for (int k = 0; k < iter + 1; k++) {
          Serial.print(receivedChars[k]);
        }
        return 2;                           //Return 2 because it is a relevant message.
        break;
      default:
        while (Serial.read() >= 0) { }
        return 404;                     //Return 404 in case of an error.
        break;
    }
  }
  else {
    return 0; // No message was available
  }
}

int determineDuration(char DurationChar){
  if (DurationChar == 0) {
    return 0;
  }
  else {
    driveTime = (DurationChar - '0') * 500;
    return driveTime;
  }
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
    return max(ACCSpeed - abs(3 * beta * (Distance - Ref_Distance)) + 0.01, 0.01) - 0.01;
  } else {                                                      // in the window, only make small adjustments.
    return ACCSpeed + 0.1 * beta * (Distance - Ref_Distance);
  }
}

void loop() {
  communication = getMessage();
  curTime = millis();
  if (communication == 2) {
    //Relevant message, listen, Save received chars in new string
    ID = receivedChars[1];
    if (SELF == ID) {   //check if message is for you
      Direction = receivedChars[2];
      DurationChar = receivedChars[3];
      Duration = determineDuration(DurationChar);
      startTime = curTime;
      commandExcecuted = false;
    }
  }
  passedTime = curTime - startTime;

  if (passedTime < Duration && commandExcecuted != true) {
    switch (Direction) {
      case 'F':
        move_servos(baseSpeed, 0);
        break;
      case 'B':
        move_servos(-baseSpeed, 0);
        break;
      case 'R':
        move_servos(0, -1);
        break;
      case 'L':
        move_servos(0, 1);
        break;
    }
  }
  else {
    move_servos(0, 0);
    //stuur bericht dat hij de volgende berekening verwacht
  }
  //  baseSpeed = ACC();                          //determine speed with Active cruise control.

  // Read from IR sensors
  IR_left = digitalRead(pin_IR_left);
  IR_right = digitalRead(pin_IR_right);

  if (IR_left == LOW && IR_right == LOW) {      // If no line is detected
    rechtdoor++;
    //    move_servos(baseSpeed, 0);
    if (baseSpeed > 0.03) {
      headTurn = 0;
    }
  } else if (IR_left == HIGH && IR_right == LOW) { // if line is detected by left side
    turnleft ++;
    rechtdoor = 0;
    if (turnleft > 3) {
      headTurn = -45;
    } else {
      headTurn = -10;
    }
    move_servos(0, 0);
    commandExcecuted = true;
    //    move_servos(baseSpeed, 1);
  } else if (IR_left == LOW && IR_right == HIGH) {    // if line is detected by right side
    turnright ++;
    rechtdoor = 0;
    if (turnright > 3) {
      headTurn = 45;
    } else {
      headTurn = 10;
    }
    move_servos(0, 0);
    commandExcecuted = true;
    //    move_servos(baseSpeed, -1);
  } else if (IR_left == HIGH && IR_right == HIGH) { // 2 lines detected at same time
    rechtdoor = 0;
    move_servos(0, 0);
    commandExcecuted = true;
  }
  servo_head.write(90 + headTurn); //turn head in turning direction
  delay(100);
}
