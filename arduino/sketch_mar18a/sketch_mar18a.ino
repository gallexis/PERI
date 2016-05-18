#include <SPI.h>
#include <RF24.h>
#define MAXTIMER 16
long waitForTimer[MAXTIMER];
char buffer[32];
int on = 0;
int reset=0;
int sensorPin = 23; 

int waitFor(int timer, long period){
  long newTime = micros() / period;
  int delta = newTime - waitForTimer[timer];
  if ( delta ) {
    waitForTimer[timer] = newTime;
  }
  return delta;
}

void setup() {
  pinMode(13,OUTPUT);
  pinMode(sensorPin,INPUT);
  Serial.begin(115200);
}

void Led( int *on, int timer, long period, int led) {
  static int val = 1;

  if (!waitFor(timer,period)) return;

  if(*on == 1)
  {
    digitalWrite(led,val);
    val = 1 - val;

  }
  else
    digitalWrite(led,0);

}

void Mess( int *on,int timer, long period, const char * mess) {
  if (!waitFor(timer,period)) return;
    
  if(reset == 1)
  {
    Serial.println(mess);
    reset=0;
  }
}

void GetKbd(char *buffer, int *on)
{
 
 if(Serial.available()==0)
  return;
  
  Serial.readBytes(buffer, 32);

  *on = 1 - *on;
  reset=1;
  
}

void mess2(){
  
}

void GetAn(){
  
}

void loop() {

  Led (&on,0,100000,13);
  Mess(&on,1,1000000,buffer);
  GetKbd(buffer, &on);
  Serial.println(analogRead(sensorPin));
  
}
