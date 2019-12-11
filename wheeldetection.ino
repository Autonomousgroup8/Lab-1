#include <Servo.h>


Servo servo_left;
Servo servo_right;

const int pin_Servo_left = 13;
const int pin_Servo_right = 12;

const int pin_left_wheel=8;
const int pin_right_wheel=7;

int left_wheel;
int right_wheel;
int leftcount=1;
int rightcount=1;

int gamma=0;
int delta=0;
int tl=0;
int tr=0;
float prev_time_links=0;
float prev_time_rechts=0;
float tijd_links=0;
float tijd_rechts=0;
float left_speed=0;
float right_speed=0;
float speed;
float average=0;
float prev_average=0;
int a=0;
float average_left=0;
float average_right=0;
float left_av=0;
float right_av=0;
float top=0;
int x=0;
int count=0;
int uitkomst;
int som;




void setup() 

{
  Serial.begin(9600);
  servo_left.attach(pin_Servo_left);
  servo_right.attach(pin_Servo_right);

  servo_right.write(90);
  servo_left.write(90);
}

void loop()
{

  count++;
 left_wheel=digitalRead(pin_left_wheel);
 right_wheel=digitalRead(pin_right_wheel);
 servo_left.write(80);
 servo_right.write(100);


if(left_wheel==1)
{
  if(gamma==0)
  {
    
    Serial.print("Leftcount: ");
    Serial.print(leftcount);
    prev_time_links=tijd_links;
    tijd_links=millis();
    tl=tijd_links-prev_time_links;
    //Serial.print("tl");
    //Serial.println(tl);
    left_speed=5.18/tl;
    
    if(left_speed>10000)
    {
      left_speed=0;
      leftcount--;
      Serial.print("; mismatch");
    }
    Serial.print("; Left wheel speed: ");
    Serial.print(left_speed, 4);
    Serial.print("m/s");

    //gemiddelde linkerwielsnelheid berekenen
   Serial.print(average_left);
    average_left=average_left+left_speed;
    Serial.print("; average_left, hoort elke keer groter te worden: ");
    Serial.print(average_left);
    
    left_av=average_left/leftcount; // dit is het gemiddelde
    Serial.print("; left_av");
    Serial.print(left_av);
    
    gamma=1;
    leftcount++;
  }

}
else if(left_wheel==0)
{
  gamma=0;
}


if(right_wheel==1)
{
  if(delta==0)
  {
    
    Serial.print("; Rightcount: ");
    Serial.print(rightcount);
    prev_time_rechts=tijd_rechts;
    tijd_rechts=millis();
    tr=tijd_rechts-prev_time_rechts;
    //Serial.print("tr");
    //Serial.println(tr);
    right_speed=5.18/tr;
    if(right_speed>10000)
    {
      right_speed=0;
      rightcount--;
      Serial.print("; mismatch");
    }
    Serial.print("; Right wheel speed: ");
    Serial.print(right_speed, 4);
    Serial.print("m/s");

    //gemiddelde rechterwielsnelheid berekeken
    
    average_right=average_right+right_speed;
    Serial.print("; average right, hoort elke keer groter te worden: ");
    Serial.print(average_right);
    right_av=average_right/rightcount;  //dit is het gemiddelde!!
   Serial.print("; right_av");
   Serial.println(right_av);
    
    
    delta=1;
    rightcount++;
  }
}
else if(right_wheel==0)
{
  delta=0;
}

//gemiddelde uitrekenen+++++++++++++++++++++++++++++++++
//gemiddelde van het langzaamste wiel gepakt
if(millis()>a*1000)
{
  prev_average=average;
  if(left_av>right_av)
  {
  Serial.println();
  Serial.print("de gemiddelde snelheid is: ");
  Serial.println(right_av, 4);
  }
  else if(left_av<right_av)
  {
    Serial.println();
  Serial.print("De gemiddelde snelheid is: ");
  Serial.println(left_av, 4);
  }
  

  
  
  a++;
  left_av=0;
  right_av=0;
  rightcount=1;
  leftcount=1;
  average_left=0;
  average_right=0;
}



/*
//topsnelheid geven+++++++++++++++++++++++++++++++++++++
if(average>prev_average)
{
  top=average;
  Serial.print("new topspeed: ");
  Serial.println(top);
  
}
*/


//kalibreren++++++++++++++++++++++++++++++++++++++++++++
/*if(x==0)
{
  if(right_speed!=left_speed)
  {
    Serial.println("ongelijk!");
    int variabel=left_speed/right_speed;
    uitkomst=som*variabel;
    Serial.println("gelukt!");
    x++;
  }
}

if(count>10000)
{
  x=0;
}
*/


//aantekeningen

 //code hier beginnen.

}
