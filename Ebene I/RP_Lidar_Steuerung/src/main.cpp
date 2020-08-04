//*******************************************************************************//
//                              INITIALISIERUNGSPHASE                            //
//*******************************************************************************//

//******************** PIN-DEFINITION ********************//


//********************* BIBLIOTHEKEN *********************//


#include "Hebevorrichtung.h"
Hebevorrichtung Lift;

//*********************** VARIABLEN **********************//

uint32_t i = 0;

void setup() {
  Serial.begin(115200);
  Lift.init();
  Lift.Starposition();
  Serial.println("Start Lift");
}

void loop() {
  Lift.Fahren();
}