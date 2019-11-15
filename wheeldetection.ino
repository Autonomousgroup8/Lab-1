//aflezen van een signaal
//toevoegingen:
//r=3.3cm
//deel 3: 2.1 toevoegen, en die formule pakken
//1: juiste pin: initialiseren(7 en 8)?
//2: pin waarde aflezen en testen
//2.1: afstand bepalen door meting elke 0.1 seconde
//3: formule toevoegen: 2*pi*r->2*3.3*pi=6.6*pi=20.73cm per rondje
//5: kalibreren voor recht rijden

#include <Servo.h>


Servo servo_left;
Servo servo_right;

const int pin_Servo_left = 11;
const int pin_Servo_right = 12;

const int pin_left_wheel=6;
const int pin_right_wheel=8;

int left_wheel=0;
int right_wheel=0;

void setup()
{
  Serial.begin(9600);
  servo_left.attach(pin_Servo_left);
  servo_right.attach(pin_Servo_right);

  //servomotor initialise to zero
  servo_right.write(90);
  servo_left.write(90);
}


void loop()
{
  left_wheel = digitalRead(pin_left_wheel);
  right_wheel = digitalRead(pin_right_wheel);
  servo_left.write(70);
  servo_right.write(110);
  delay(100);
  servo_left.write(90);
  servo_right.write(90);
  delay(1000);
  

  // Write the sensor input values to the debugging channel
  Serial.print("Left wheel value: ");
  Serial.print(left_wheel);
  Serial.print("; right wheel value: ");
  Serial.print(right_wheel);
  Serial.println();
  delay (2000);
  int count=0;
  count++;
  Serial.print(count);
 
}





