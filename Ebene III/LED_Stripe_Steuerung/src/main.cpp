//*******************************************************************************//
//                              INITIALISIERUNGSPHASE                            //
//*******************************************************************************//

//***** PIN-DEFINITION *****//
#define NEOPIXEL_PIN 5

//***** BIBLIOTHEKEN *****//
/* Kommunikation mit LED-Stripe */
/* Kommunikation über OneWire
   *  - Ansteuern der LED
   *  - Wechseln der Farbe
   * Klassendefinition:: Adafruit_NeoPixel([0],[1],[2])
   *  [0]: Anzahl der Pixel, hier 20
   *  [1]: Steuerpin, hier 5
   *  [2]: [NEO_BRG]+[NEO_KHZ800] 
   *       [NEO_BRG]: .Color(Rot,Blau,Grün)
   *       [NEO_KHZ800]: 800kHz Taktrate 
   */
#include "Adafruit_NeoPixel.h"
Adafruit_NeoPixel Echo_Strip = Adafruit_NeoPixel(20, NEOPIXEL_PIN, NEO_BRG + NEO_KHZ800);

/****Variablen:::Visualisierung****/
bool NewEcho = false;
uint8_t Echo_R2100[11]; //Distanzarray des R2100 P+F Sensors, für die Visualisierung
uint8_t new_angle = false;

uint8_t LED_Brightness = 255;

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  RS485-BUS: Datenaufbereitung
                void get_BUS_data(uint8_t *income_buffer)
  ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

uint8_t getData(){

  uint8_t buffer[64];
      Serial.readBytesUntil('*', buffer, 64);
      if(buffer[0] == 'R'){
        for (uint8_t Echo = 0; Echo < 11; Echo++){Echo_R2100[Echo] = buffer[Echo];}
        return true;
      }
      else if(buffer[0] == 'B'){
        LED_Brightness = buffer[1];
        Serial.println(LED_Brightness);
        return true;
      }
    
  
  else {return false;}
}

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(20);
  Echo_Strip.begin();
  Echo_Strip.show();
  for (uint8_t num = 0; num < Echo_Strip.numPixels(); num++)
  { //macht alle LED's aus
    if (num > Echo_Strip.numPixels())
    {
      num = Echo_Strip.numPixels() - 1;
    }
    Echo_Strip.setPixelColor(num, Echo_Strip.Color(0, 0, 0));
    Echo_Strip.show();
  }
  delay(250);
}

//*******************************************************************************//
//                                HAUPTPROGRAMM                                  //
//*******************************************************************************//

void loop()
{
  if (getData() == true)
  {
    for (byte Echo = 0; Echo <= 10; Echo += 1)
    {
      Echo_Strip.setPixelColor(
        map(Echo, 0, 10, 4, 14),                           
        Echo_Strip.Color(map(Echo_R2100[Echo],   0, 255, LED_Brightness, 0),
                         0,
                         map(Echo_R2100[Echo],   0, 255, 0, LED_Brightness)
                        )
      );
    }
    Echo_Strip.show();
  }
}