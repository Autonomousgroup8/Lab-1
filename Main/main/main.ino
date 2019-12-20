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

int ForwardTime[10] = {200, 500, 1000, 1500, 2000, 3000, 4000, 5000, 7500, 10000};

int incomingByte = 0;
int IR_left = 0;
int IR_right = 0;
float baseSpeed = 0.05;
float correction = 0.025;
float ACCSpeed = 0.05;
int driveTime = 0;
int beta = 0.002;
int communication = 0;
int curTime = 0;
char ID = 0;
char Direction;
char DurationChar;
int Duration = 0;
int startTime = 0;
bool commandExcecuted = false;
int passedTime = 0;
int test;
//Variables for communication
int iter = 0;
char receivedID = 0;
char endMarker = '\n';
const byte numChars = 32;
char receivedChars[numChars];

//Zigbee implementation
#define SELF     53
#define PAN_ID           "A008"
#define CHANNEL_ID       "0F"

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
  if (Serial.available() > 0) { //A message is available
    int startBit = Serial.read();
    delay(5);
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

int determineDuration(char DurationChar) {
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

void loop()
{
  communication = getMessage();
  curTime = millis();
  if (communication == 2) { //Relevant message, listen, Save received chars in new string
    ID = receivedID;
    if (SELF == ID) {   //check if message is for you
      Direction = receivedChars[0];
      Serial.print("Richting");
      Serial.print(Direction);
      DurationChar = receivedChars[1]; 
      Duration = ForwardTime[DurationChar-48];
      Serial.print(Duration);
      startTime = curTime;
      commandExcecuted = false;
    }
  }
  passedTime = curTime - startTime;

  if (passedTime < Duration && commandExcecuted != true) {
    switch (Direction) {
      case 'F':
        Serial.print("Forward");
        move_servos(baseSpeed, 0);
        baseSpeed = ACC();                          //determine speed with Active cruise control.

        IR_left = digitalRead(pin_IR_left);         // Read from IR sensors
        IR_right = digitalRead(pin_IR_right);

        if (IR_left == LOW && IR_right == LOW) {      // If no line is detected
          move_servos(baseSpeed, correction);
        } else if (IR_left == HIGH && IR_right == LOW) { // if line is detected by left side
          move_servos(baseSpeed, -0.05);
        } else if (IR_left == LOW && IR_right == HIGH) {    // if line is detected by right side
          move_servos(baseSpeed, 0.05);
        }
        break;
      case 'B':
        Serial.print("Break");
        move_servos(0, 0);
        break;
      case 'R':
        Serial.print("Right");
        move_servos(0, 0.05);
        break;
      case 'L':
        Serial.print("Left");
        move_servos(0, -0.05);
        break;
    }
  }  else {
    move_servos(0, 0);
    //stuur bericht dat hij de volgende berekening verwacht
  }
}
