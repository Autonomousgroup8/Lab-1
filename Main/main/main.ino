#include <NewPing.h>
#include <Servo.h>

#define TRIGGER_PIN  9
#define ECHO_PIN     9
#define MAX_DISTANCE 200

Servo servo_left;
Servo servo_right;

const int pin_Servo_left = 13;       
const int pin_Servo_right = 12;

const int pin_IR_left = 0;       
const int pin_IR_right = 1;
bool waitMode = false;
bool inMiddle = false;
int IR_left = 0;
int IR_right = 0;

const int threshhold = 700;
const float alpha = 0.001;
unsigned long StartTime = 0;
float baseSpeed = 0.5;

//Zigbee implementation
#define SELF     43
#define PAN_ID           "A008"
#define CHANNEL_ID       "0F"

// some macros needed for the xbee_init function. Do not touch :-).
#define STRING(name) #name
#define TOSTRING(x) STRING(x)


void move_servos(float baseSpeed, float offset)
{
  float speed_left = baseSpeed + offset;
  float speed_right = -baseSpeed + offset;

  speed_left = constrain(speed_left, -1, 1);
  speed_right = constrain(speed_right, -1, 1);
    
    servo_left.write(90+speed_left*90);
    servo_right.write(90+speed_right*90);
}



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


void setup() 
{

   // initialize the XBee wireless module
  xbee_init();
  // initialize the LED light on the Arduino board
  led_init();
  // send something on the wireless network
  Serial.println("This is the XBee - Broadcast program.");
  servo_left.attach(pin_Servo_left);
  servo_right.attach(pin_Servo_right);

  // Init servo motors with 0
  servo_right.write(90);
  servo_right.write(90);
}

void loop()
{    
  if(!waitMode){

  
  // Read from IR sensors
    IR_left = analogRead(pin_IR_left);
    IR_right = analogRead(pin_IR_right);
    
    if(IR_left < threshhold && IR_right < threshhold){
    // If no line is detected
    
    StartTime = 0;
        move_servos(baseSpeed, 0);
    
  }else if (IR_left > threshhold && IR_right < threshhold) {
    // if line is detected by left side
    // if StartTime is not set set it
    if(!StartTime){
      StartTime = millis();
    }
    
    move_servos(baseSpeed, -alpha*(millis() - StartTime));
    
  }else if (IR_left < threshhold && IR_right > threshhold) {
    // if line is detected by right side
    
    // if StartTime is not set set it
    if(!StartTime){
      StartTime = millis();
    }
    
    move_servos(baseSpeed, alpha*(millis() - StartTime));
    
  }else if(IR_left > threshhold && IR_right > threshhold && !inMiddle){
    //We are at an intersection, so we enter waiting mode
    waitMode = true;
    inMiddle = true;
    move_servos(0,0);
    Serial.print(1);
    }
    else if (IR_left > threshhold && IR_right > threshhold && inMiddle){
    inMiddle = false;
    }
  }
  else{
  //Check incoming data
  if(Serial.available()>0){
    int incomingByte = 0;
    incomingByte = Serial.read();
    if (incomingByte == 49){
      waitMode = false;
  }
  }
  }
}
