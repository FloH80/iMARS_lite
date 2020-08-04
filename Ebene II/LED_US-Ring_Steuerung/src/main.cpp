//*******************************************************************************//
//                   Ansteuerung der LED Ringe zur Visualisierung 
//                              der einzelnen Distanzen                                                          
//*******************************************************************************//


#include <Arduino.h>
#include "Adafruit_NeoPixel.h"

#define LEDrPin               10    // LED Signal Pin vorne rechts
#define LEDlPin               11    // LED Signal Pin vorne links
#define LEDbPin               12    // LED Signal Pin vorne hinten

#define numPix                12    // Ring Pixelanzahl

uint8_t LEDBrightness = 15;    // 0-255

Adafruit_NeoPixel ringR(numPix, LEDrPin, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ringL(numPix, LEDlPin, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ringB(numPix, LEDbPin, NEO_GRB + NEO_KHZ800);

uint8_t Echo_UC2000[3] = {200, 200, 200}; //Distanzarray der UC2000 P+F US-Sensoren, für die Visualisierung

uint8_t buffer[4]; //buffer zum Einlesen der seriellen Schnittstelle

boolean getData();
void display_dis();

void setup() {

  Serial.begin(115200);
  Serial.setTimeout(20);

  ringR.begin();
  ringL.begin();
  ringB.begin();

  ringR.show();
  ringL.show();
  ringB.show();
}

void loop() {
  //wenn Daten empfangen, dann Entferunng anzeigen
  if(getData()){
      display_dis();
        //Serial.println(Echo_UC2000[i]);
      
    }
  
}

//Liest die gesendeten Daten, über die Serielle Schnittstelle, vom Arduinoi MEGA "Sensorsteuerung_PF_UC2000" ein
boolean getData() {
  if(Serial.find('U')){                     //Nur ein Char kein String!!!!
    Serial.readBytes(buffer,4);             //nur so viele Bytes einlesen, wie wirklich benötigt
    Serial.print("ECHO");
    for(uint8_t Echo = 0; Echo < 3; Echo++){
      Echo_UC2000[Echo] = buffer[Echo];
      Serial.print("\t");Serial.print(Echo_UC2000[Echo]);
    }
    LEDBrightness = buffer[3];
    Serial.print("\t");Serial.print("HELLIGKEIT: ");Serial.println(LEDBrightness);
   return true; 
  }else{
    return false;
  }
}

//Entferung wird in RGB Farbraum überführt - Farbverlauf von Grün zu Rot 
void display_dis(){
  uint8_t rr, gr, rl, gl, rb, gb;
  uint8_t b = 0; //keinen Blauanteil

  //Farbverlauf rechte LED
  gr = map(Echo_UC2000[0], 0, 200, 0, 255); //Entferunng von cm in 8-Bit Farbe umwnadeln
  rr = map(Echo_UC2000[0], 0, 200, 255, 50);
  uint32_t color_display_r = ringR.Color(rr, gr, b);
  ringR.fill(color_display_r, 0, numPix);
  ringR.setBrightness(LEDBrightness);
  ringR.show();
  /*//zu nah: rot blinken
  if (distance[0]<=7){
    ringR.setBrightness(0);
    ringR.show();
    delay(50);
    uint32_t color = ringR.Color(255, 0, 0);
    ringR.setBrightness(LEDBrightness);
    ringR.fill(color, 0, 12);
    ringR.show();
    delay(150);
  }*/

  //Farbverlauf linke LED
  gl = map(Echo_UC2000[1], 0, 200, 0, 255);
  rl = map(Echo_UC2000[1], 0, 200, 255, 50);
  uint32_t color_display_l = ringL.Color(rl, gl, b);
  ringL.fill(color_display_l, 0, numPix);
  ringL.setBrightness(LEDBrightness);
  ringL.show();

  //Farbverlauf hintere LED
  gb = map(Echo_UC2000[2], 0, 200, 0, 255);
  rb  = map(Echo_UC2000[2], 0, 200, 255, 50);
  uint32_t color_display_b = ringB.Color(rb, gb, b);
  ringB.fill(color_display_b, 0, numPix);
  ringB.setBrightness(LEDBrightness);
  ringB.show();
}