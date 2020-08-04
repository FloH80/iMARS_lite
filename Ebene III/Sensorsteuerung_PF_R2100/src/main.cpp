//*******************************************************************************//
//                              INITIALISIERUNGSPHASE                            //
//*******************************************************************************//


//***** PIN-DEFINITION *****//
#define RS485_IN_CTRL   22
#define RS485_OUT_CTRL  23
#define RS485_TRX_CTRL  24

//***** BIBLIOTHEKEN *****//
#include "Arduino.h"

/* Systemsteuerung */
#include "System.h"
SysControl System;

/* Kommunikation intern des iMARS_lite */
/* Kommunikation über RS485
    - Sensorsteuerung
    - Visualisierungssteuerung
*/
#include "RS485.h"
RS485 RS485(Serial1, RS485_IN_CTRL, RS485_OUT_CTRL, RS485_TRX_CTRL);
/* Sensoransteuerung */
/* Kommunikation mit dem R2100
    - Auslesen des Sensor
    - Verarbeiten der Daten
*/
#include "R2100.h"
R2100 R2100(Serial2);


//*******************************************************************************//
//                            INITIALISIERUNGSPHASE                              //
// void setup()                                                                  //
//*******************************************************************************//

void setup() {
  //* Initialsierung der Schnittstellen *//
  /* Initialsierung UART-Port 0
      - Kommunikation mit dem Computer "Debugging"
  */
  Serial.begin(115200);
  /* Initialisierung RS485
      - Kommunikation, iMARS_lite intern
      - [in1] Adresse : "C00" Control Nr.00
      - [in2] Baudrate: 115200 Bit/s
  */
  RS485.init("I0", 115200);
  /*
    while(System.Heartbeat == false){
      System.ConvertRS485Data(RS485.receive());
      if(System.Heartbeat == true){
        RS485.transmit("C0", 'I', 'h', 0, 0);
      }
    }
    System.Heartbeat = false;
    delay(5000);
  */
  /* Initialsierung UART-Port 1
      - Kommunikation Visualisierung
  */
  Serial3.begin(115200);
}



//*******************************************************************************//
//                                HAUPTPROGRAMM                                  //
//*******************************************************************************//

void loop() {

  //* Einlesen und verarbeiten von BUS-Daten *//
  System.ConvertRS485Data(RS485.receive());

  /* Abfrage ob neue Messwerte vorhanden sind */
  System.NewSensorValues = R2100.getEchos();


  //* Ausführen der Betriebsmodi *//
  /*  Betriebsmodi
        0: parametrier Modus(Start Modus)
        1: manueller Modus
        2: automatik Modus
        3: Folgen Modus
        4: GPS Modus
  */
  switch(System.Mode[0]){  

    //**************************************************************//
    //********************** PARAMETRIER MODUS *********************//    
    case 0:
      if (System.ModeNew == true) {
        System.ModeNew = false;
      }
      //SENSORMODUS
      if(System.ParameterNew[20] == true){
        System.SensorMode = char(System.Parameter[20][0]);
        System.ParameterNew[20] = false;
      }
      //RELEVANTE SENSOR REICHWEITE
      if (System.ParameterNew[21] == true){
        R2100.initRelevanceDistance((System.Parameter[21][0] << 8) + System.Parameter[21][1]);
        System.ParameterNew[21] = false;
      }
      //LED-REICHWEITE
      if (System.ParameterNew[22] == true){       
        System.initVisualisationDistance((System.Parameter[22][0] << 8) + System.Parameter[22][1]);
        System.ParameterNew[22] = false;
      }
      // LED-HELLIGKEIT
      if (System.ParameterNew[23] == true){
        System.Brightness_LED = System.changeRange(System.Parameter[23][0], 0, 100, 0, 255);
        Serial3.write('B');
        Serial3.write(System.Brightness_LED);
        Serial3.write('*');
        System.ParameterNew[23] = false;
      }

      
      System.EchoVisualisation(R2100.Distance);
    break;




    //**************************************************************//
    //******************** MANUELLER-FAHR MODUS ********************//
    case 1:
      if (System.ModeNew == true) {
        System.ModeNew = false;
      }
      System.EchoVisualisation(R2100.Distance);
    break;



    //**************************************************************//
    //******************** AUTOMATIK-FAHR MODUS ********************//
    //********************** DATENAUSWERTUNG ***********************//
    case 2:
      if (System.ModeNew == true) {
        System.ValueSend = true;
        System.ModeNew = false;
      }

    //***** Auswahl der Sensordatenverarbeitung *****//
      if (System.SensorMode == 'd'){ 
      //***** Bilden der Differenz *****//
        System.SendValue[0] = R2100.DifferentialCalc(System.DifferentialEcho);
      }

      else if(System.SensorMode == 'n'){
      //***** Nächstliegender relevanter Wert *****//
        uint8_t EchoPosition = R2100.ObjektRecognition();
        System.SendValue[0] = highByte(R2100.Distance[EchoPosition]);
        System.SendValue[1] = lowByte(R2100.Distance[EchoPosition]);
        System.SendValue[2] = R2100.Angle[EchoPosition];
        System.SendValueCounter = 3;
      }

      else if(System.SensorMode == 'n'){

      }
  
      System.EchoVisualisation(R2100.Distance);
    break;
  }



  //******************** DATEN SENDEN ********************//
  if (System.NewSensorValues == true && System.ValueSend == true) {
    RS485.transmit("C0", 'I', System.SensorMode, System.SendValueCounter, System.SendValue);
    System.ValueSend = false;
  }



  //******************** VISUALISIERUNG ********************//
  //if(System.Heartbeat == true){
  //  RS485.transmit("C0", 'I', 'i', 0, 0);
  //}

}

