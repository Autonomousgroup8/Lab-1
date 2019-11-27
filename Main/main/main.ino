//#include <NewPing.h>
#include <Servo.h>

#define TRIGGER_PIN  9
#define ECHO_PIN     9
#define MAX_DISTANCE 200

Servo servo_left;
Servo servo_right;

const int pin_Servo_left = 13;       
const int pin_Servo_right = 12;

const int pin_IR_left = A0;       
const int pin_IR_right = A1;

int incomingByte = 0;
int IR_left = 0;
int IR_right = 0;
int turnleft = 0;
int turnright = 0;
int rechtdoor = 0;
const float alpha = 0.1;
unsigned long StartTime = 0;
float baseSpeed = -0.05;

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
// When the program is *running*, the switch on the wireless proto shield should be in the position 'MICRO'.
// During programming of the Arduino, the switch should be in the position 'USB'.
// It will only work if the XBee module is set to communicate at 9600 baud. If it is not, the module needs to be reprogrammed
// on the USB XBee dongle using the XCTU program.
void xbee_init(void)
{
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

void move_servos(float baseSpeed, float offset)
{
  float speed_left = baseSpeed +  offset;
  float speed_right = -baseSpeed + offset;

  speed_left = constrain(speed_left, -1, 1);
  speed_right = constrain(speed_right, -1, 1);
      
  servo_left.write(90+speed_left*90);
  servo_right.write(90+speed_right*90);
}

void setup() 
{ 
  xbee_init();
  Serial.begin(9600);
  Serial.println("This is the XBee - Broadcast program.");
  
  servo_left.attach(pin_Servo_left);
  servo_right.attach(pin_Servo_right);

  servo_right.write(90);
  servo_right.write(90);
}

void loop()
{    
    IR_left = digitalRead(pin_IR_left);
    IR_right = digitalRead(pin_IR_right);
  if(!waitMode){
//      if(IR_right == LOW && turnright > 6){
//      turnright = 0;
//      move_servos(baseSpeed, alpha);
//      delay(100);
//      while(IR_left == HIGH){
//        move_servos(baseSpeed, 0);
//        IR_left = digitalRead(pin_IR_left);
//        }
//    }
//    if(IR_left == LOW && turnleft > 6){
//      turnleft = 0;
//      move_servos(baseSpeed, -alpha);
//      delay(100);
//      while(IR_right == HIGH){
//        move_servos(baseSpeed, 0);
//        IR_right = digitalRead(pin_IR_right);
//        }
//    }
//    if(IR_right == LOW){
//      turnright = 0;
//    }
//    if(IR_left == LOW){
//      turnleft = 0;
//    }

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
      if(turnleft > 10){
      move_servos(baseSpeed, -2*alpha);
      }
      move_servos(baseSpeed, -alpha);
  
    }else if (IR_left == LOW && IR_right == HIGH) {
      // if line is detected by right side
    turnright ++;
    rechtdoor = 0;
    if(turnright > 10){
      move_servos(baseSpeed, 2*alpha);
      }
    move_servos(baseSpeed, alpha);
 
    }else if(IR_left == HIGH && IR_right == HIGH && crossingsPassed == 0){
    rechtdoor = 0;
    waitMode = true;
    crossingsPassed++;
    move_servos(0, 0);
    //delay(100);
    //erial.println(1);
    Serial.print("Crossing: ");
    Serial.print(crossingsPassed);
    Serial.println();
    // Intersection protocol
    }
        
    else if(IR_left == HIGH && IR_right == HIGH && crossingsPassed > 0){
    // if both detect a line (consider it as no line for now)
    rechtdoor = 0;
    StartTime = 0;
    crossingsPassed++;
    Serial.print("Crossing2: ");
    Serial.print(crossingsPassed);
    Serial.println();
    if(crossingsPassed>10){
      crossingsPassed ==0;
      } ;
    move_servos(baseSpeed,0);
    // Intersection protocol
  }
  }
  else if (waitMode == true){
      if(Serial.available()>0){
        incomingByte = Serial.read();
      if (incomingByte == 49){
        waitMode = false;
        move_servos(baseSpeed,0);
        delay(200);
      }
      }
  }
//  Serial.print("LEFT: ");
//  Serial.print(turnleft);
//  Serial.print(" RIGHT: ");
//  Serial.print(turnright);
//  Serial.println();
  }
