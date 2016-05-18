#include <SPI.h>
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);

byte addresses[][6] = {"0Node","1Node","2Node","3Node","4Node","5Node"};

void setup() {

  Serial.begin(115200);
  printf_begin();
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openReadingPipe(1,addresses[1]);
  radio.startListening();
  
}

void loop() {
  byte light;
    
  if( radio.available()){
     radio.read( &light, sizeof(byte) );             // Get the payload
     Serial.println(light);
   }
}
