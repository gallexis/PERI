#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RF24.h"
#include "printf.h"

// unsigned int waitFor(timer, period) 
// Timer pour taches périodique
// arguments :
//  - timer  : numéro de timer entre 0 et MAX_WAIT_FOR_TIMER-1
//  - period : période souhaitée
// retour :
//  - nombre de période écoulée depuis le dernier appel
//
#define MAX_WAIT_FOR_TIMER 16
unsigned int waitFor(int timer, unsigned long period){
  static unsigned long waitForTimer[MAX_WAIT_FOR_TIMER];
  unsigned long newTime = micros() / period;
  int delta = newTime - waitForTimer[timer];
  if ( delta < 0 ) delta += 1 + (0xFFFFFFFF / period);   
  if ( delta ) waitForTimer[timer] = newTime;
  return delta;
}

// Configuration des broches
#define PIN_LUMI 1
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
RF24 radio(9,10); // radio(CE,CS)
byte addresses[][6] = {"0Sens","1Sens","2Sens","3Sens","4Node","5Sens"};

// Variables globales pour la communication inter-taches
byte lumi, lumiFull;

void Lumi (int timer, unsigned long period, byte pin, byte *lumi, byte *lumiFull)
{
   if (!waitFor(timer,period)) return;
   *lumi = map(analogRead(pin),0,1023,0,99);
   *lumiFull = 1;   
}

void SensRF (byte *mess,  byte *full) {
  
  
  if (! (*lumiFull)) return;
  if (!radio.write( mess, sizeof(byte) )){
     Serial.println(F("Now sending !"));
     Serial.println(F("failed!"));
     return;
  }
  *lumiFull=0;
}

void setup() {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  printf_begin();
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(addresses[1]);
  radio.printDetails();
}

void loop() {
  Lumi (0,1000000, PIN_LUMI, &lumi, &lumiFull);  
  SensRF (&lumi, &lumiFull);
}
