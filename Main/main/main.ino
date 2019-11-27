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
int leftThreshold;
int rightThreshold;

int turnleft = 0;
int turnright = 0;

const float alpha = 0.0005;
unsigned long StartTime = 0;
float baseSpeed = -0.05;

//Zigbee implementation
#define SELF     43
#define PAN_ID           "A008"
#define CHANNEL_ID       "0F"
bool waitMode = false;
int crossingsPassed = 0;
int incomingByte = 0;


// some macros needed for the xbee_init function. Do not touch :-).
#define STRING(name) #name
#define TOSTRING(x) STRING(x)

// initialize the LED light on the board
void led_init(void)
{
  // set the pin mode for the LED light pin to OUTPUT
  pinMode(LED_BUILTIN,   OUTPUT);
  // turn the LED off
  digitalWrite(LED_BUILTIN,   LOW);
}


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
  // initialize the LED light on the Arduino board
  led_init();
  Serial.begin(9600);
  // send something on the wireless network
  Serial.println("This is the XBee - Broadcast program.");
  
  servo_left.attach(pin_Servo_left);
  servo_right.attach(pin_Servo_right);

  IR_left = analogRead(pin_IR_left);
  IR_right = analogRead(pin_IR_right);
  
  // Init servo motors with 0
  servo_right.write(90);
  servo_right.write(90);

}

void loop()
{    

  if(!waitMode){
  // Read from IR sensors
    IR_left = analogRead(pin_IR_left);
    IR_right = analogRead(pin_IR_right);               //digitalRead would be preffered.

    //Sharp corner protocol right
    if(IR_right < rightThreshold && turnright > 4){   //right sensor has detected white after sharp turn
      turnright = 0;
      move_servos(baseSpeed, 1);                      //turn a bit further for improved corner handling
      delay(100);
      while(IR_left > leftThreshold){                 //drive forward while left sensor still detects black
        move_servos(baseSpeed, 0);
        IR_left = analogRead(pin_IR_left);
        //Serial.println("Hallo");
        }
    }
    //Sharp corner protocol left
    if(IR_left < leftThreshold && turnleft > 4){
      turnleft = 0;
      move_servos(baseSpeed, -1);
      delay(100);
      while(IR_right > rightThreshold){
        move_servos(baseSpeed, 0);
        IR_right = analogRead(pin_IR_right);
        //Serial.println("Hallo");
        }
    }

    if(IR_right < rightThreshold){
      turnright = 0;
    }
    if(IR_left < leftThreshold){
      turnleft = 0;
    }
    
    if(IR_left < leftThreshold && IR_right < rightThreshold){
    // If no line is detected
    StartTime = 0;
    move_servos(baseSpeed, 0);
    
    }else if (IR_left > leftThreshold && IR_right < rightThreshold) {
      // if line is detected by left side
      turnleft ++;
      // if StartTime is not set set it
      if(!StartTime){
        StartTime = millis();
      }
    move_servos(baseSpeed, -alpha*(millis() - StartTime));

  
    }else if (IR_left < leftThreshold && IR_right > rightThreshold) {
      // if line is detected by right side
<<<<<<< Updated upstream
      turnright ++;
      // if StartTime is not set set it
      if(!StartTime){
        StartTime = millis();
=======
    turnright ++;
    rechtdoor = 0;
      // if StartTime is not set set it 
    if(turnright > 10){
      move_servos(baseSpeed, 2*alpha);
      }
    move_servos(baseSpeed, alpha);
 
    }else if(IR_left == HIGH && IR_right == HIGH && turnright < 6 && turnleft < 6 && crossingsPassed == 0){
     
    // if both detect a line (consider it as no line for now)
    rechtdoor = 0;
    StartTime = 0;
    waitMode = true;
    crossingsPassed++;
    move_servos(0, 0);
    delay(500);
    Serial.print(1);
    // Intersection protocol
    }
    else if(IR_left == HIGH && IR_right == HIGH && turnright < 6 && turnleft < 6 && crossingsPassed > 0){
     
    // if both detect a line (consider it as no line for now)
    rechtdoor = 0;
    StartTime = 0;
   crossingsPassed = (crossingsPassed + 1)%3 ;
      move_servos(baseSpeed,0);
    // Intersection protocol
>>>>>>> Stashed changes
    }
    move_servos(baseSpeed, alpha*(millis() - StartTime));

    }else if(IR_left > leftThreshold && IR_right > rightThreshold && turnright == 0 && turnleft == 0 && crossingsPassed = 0){

     waitMode = true;
    crossingsPassed++;
    move_servos(0,0);
    Serial.print(1);
    }
    else if(IR_left > leftThreshold && IR_right > rightThreshold && turnright == 0 && turnleft == 0&& inMiddle){
      crossingsPassed = (crossingsPassed + 1)%3 ;
      move_servos(baseSpeed,0);
    }
  }
  else{
      if(Serial.available()>0){
      incomingByte = Serial.read();
      if (incomingByte == 49){
      waitMode = false;
  }
  }

    
  }
  
  
//    Serial.print("Left sensor value: ");
//    Serial.print(IR_left);
//    Serial.print("; right sensor value: ");
//    Serial.print(IR_right);
//    Serial.print("; CountLeft: ");
//    Serial.print(turnleft);
//    Serial.print("; CountRight: ");
//    Serial.print(turnright);
//    Serial.println();
    
<<<<<<< Updated upstream
=======
  }
  else if (waitMode == true){
      if(Serial.available()>0){
      incomingByte = Serial.read();
      if (incomingByte == 31){
      waitMode = false;
      move_servos(baseSpeed,0);
      delay(500);
  }
      }}
  
   // Serial.print("LEFT: ");
    //Serial.print(turnleft);
    //Serial.print(" RIGHT: ");
    //Serial.print(turnright);
    //Serial.println();
>>>>>>> Stashed changes
}
