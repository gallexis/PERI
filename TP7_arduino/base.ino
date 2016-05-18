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
RF24 radio(9,10);
byte addresses[][6] = {"0Sens","1Sens","2Sens","3Sens","4Node","5Sens"};

// Variables globales pour la communication inter-taches
byte lumi, lumiFull;

void BaseRF (byte *lumi, byte *lumiFull)
{
    
  if( radio.available()){
     radio.read( lumi, sizeof(byte) );             // Get the payload
     //Serial.println(lumi);
	*lumiFull=1;
   }
}

void Oled1 (byte *mess,  byte *full) {
if (! (*full) ) return;
  *full = 0;
  Serial.println(*mess);
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Lumiere : ");
  display.println(*mess);
  display.display();
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
  radio.openReadingPipe(1,addresses[1]);
  radio.startListening();
  radio.printDetails();
}

void loop() {
  BaseRF(&lumi, &lumiFull);  
  Oled1 (&lumi, &lumiFull);
}
