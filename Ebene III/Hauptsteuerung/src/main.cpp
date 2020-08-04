//*******************************************************************************//
//                              INITIALISIERUNGSPHASE                            //
//*******************************************************************************//

//******************** PIN-DEFINITION ********************//
#define DRIVE_VELO_CTRL 2
#define RS485_IN_CTRL   22
#define RS485_OUT_CTRL  23
#define RS485_TRX_CTRL  24

//******************** BIBLIOTHEKEN ********************//
// Hallo
#include <Arduino.h>

/* Systemsteuerung */
#include "System.h"
SysControl System;

/* Kommunikation intern des iMARS_lite */
/* Kommunikation über RS485
    - Sensorsteuerung
    - Visualisierungssteuerung
*/
#include "RS485.h"
RS485 RS485(Serial2, RS485_IN_CTRL, RS485_OUT_CTRL, RS485_TRX_CTRL);

/* Kommunikation mit der Fernsteuerung */
/* Kommuinkation über Bluetooth
    - Fernsteuerung/Tablet
*/
#include "Bluetooth.h"
Bluetooth Bluetooth(Serial1);

/* Ansteuerung des Antriebs */
/* Ansteuerung über PWM
     - Steuerung über Servo-Bibliothek
*/
#include "Drive.h"
Drive Drive(DRIVE_VELO_CTRL);

/* Ansteuerung der Lenkung */
/* Ansteuerung über DynamixelShield RS485
     - Belegung des UART-Port 0
*/
#include "Steering.h"
SteerbyWire Steer;


//*******************************************************************************//
//                            INITIALISIERUNGSPHASE                              //
// void setup()                                                                  //
//*******************************************************************************//


void setup() {
  //****** Initialsierung der Variablen ******//
  /* Definieren von Standard-Werte */
  System.initVariables();
  //***** Initialsierung der Schnittstellen ******//
  /* Initialsierung UART-Port 0
      - Kommunikation mit dem Computer "Debugging"
  */
  SerialUSB.begin(115200);
  /* Initialisierung Bluetooth
      - Kommunikation mit dem Tablet
      - [in1] Baudrate: 115200 Bit/s
      - [in2] Timout: 5 ms
  */
  Bluetooth.init(115200, 50);
  /* Initialisierung RS485
      - Kommunikation, iMARS_lite intern
      - [in1] Adresse : "C0" Control Nr.00
      - [in2] Baudrate: 115200 Bit/s
  */
  RS485.init("C0", 115200, 10);
    delay(1000); // Warten, dass alles anderen Teilnehmer erreichbar sind
      
      RS485.transmit("I0", 'I', 'h', 0, 0);
      
      uint8_t *InitValues = RS485.receive();
      if(InitValues[0] == 'I' && InitValues[1] == 'h'){
        System.ActiveSensor[0] = true; 
      }else{System.ActiveSensor[0] = false;}


  //****** Initialsierung der Aktoren ******//
  /* Initialisierung des Antriebs */
  Drive.init();
  /* Initialisierung der Lenkung 
      - Ansteuerung des Dynamixel Motors über das Dynamixel Shield
      - [in1] Winkel: 187 - Ausrichtung Gerade
  */
  Steer.init();
}

//*******************************************************************************//
//                                HAUPTPROGRAMM                                  //
// void loop()                                                                   //
//*******************************************************************************//

void loop() {
  //* Einlesen und Verarbeiten der Steuerdaten *//
  /*  Auslesen der Bluetoothdaten vom Tablet
      - Steuerbefehle
      - Konfigurationsparameter
  */
  Bluetooth.getOpertionsCommand();

  //* Einlesen und verarbeiten von BUS-Daten *//
  System.ConvertRS485Data(RS485.receive());

  //* Ausführen der Betriebsmodi *//
  /*  Betriebsmodi
        0: parametrier Modus(Start Modus)
        1: manueller Modus
        2: automatik Modus
        3: Folgen Modus
        4: GPS Modus
  */
  switch (System.Mode[0]) {

    //**************************************************************//
    //********************** PARAMETRIER MODUS *********************//
    //** Einstellung von Funktionen
    //**  -> Fahrparameter, Visualisierung
    //**  -> Fahren nicht möglich
    case 0: {
        if (System.ModeNew == true) {
          Drive.writeValue(128);
          Steer.setAngle(45);     // Lenkung wird Geradeaus gerichtet
          System.DriveValueOld[0] = 128;
          System.DriveValueOld[1] = 45;          
          RS485.transmit("I0", 'M', 'p', 0, 0); // Infrarot darf Daten senden
          RS485.transmit("U0", 'M', 'p', 0, 0); // Ultraschall darf nicht Daten senden
          System.ModeNew = false;
        }
        /* Abfrage:: Neue Antriebsparameter vorhanden */
        if (System.ParameterNew[0] == true) {
          Drive.setVelocity(System.Parameter[0][0],System.Parameter[0][1]); // max. FahrGeschwindigkeit
          System.ParameterNew[0] = false;
        }
        /*  Abfrage:: Neue Lenkparameter vorhanden */
        if (System.ParameterNew[1] == true) {
          Steer.setVelocity(System.Parameter[1][0]);
          System.ParameterNew[1] = false;
        }

        if(System.ParameterNew[20] == true){
          RS485.transmit("I0", 'P', 's', 1, System.Parameter[20]);
          System.ParameterNew[20] = false;
        }
        if(System.ParameterNew[21] == true){
          RS485.transmit("I0", 'P', 'r', 2, System.Parameter[21]);
          System.maxRange = (System.Parameter[21][0] << 8) + System.Parameter[21][1];
          System.ParameterNew[21] = false;
        }
        if(System.ParameterNew[22] == true){
          RS485.transmit("I0", 'P', 'l', 2, System.Parameter[22]);
          System.ParameterNew[22] = false;
        }
        if(System.ParameterNew[23] == true){
          RS485.transmit("I0", 'P', 'b', 1, System.Parameter[23]);
          RS485.transmit("U0", 'P', 'b', 1, System.Parameter[23]);
          System.ParameterNew[23] = false;
        }
      } break;



    //**************************************************************//
    //******************** MANUELLER-FAHR MODUS ********************//
    //** Fahren durch Benutzer
    //**   -> Fahren über Joysticks
    //**   -> keine Einstellungen möglich
    case 1: {
        if (System.ModeNew == true) {
          Drive.writeValue(128);  // Antrieb wird zum Stillstand gebremst          
          Steer.setAngle(45);     // Lenkung wird Geradeaus gerichtet
          System.DriveValueOld[0] = 128;
          System.DriveValueOld[1] = 45;          
          RS485.transmit("I0", 'M', 'm', 0, 0); // Infrarot darf nicht Daten senden
          RS485.transmit("U0", 'M', 'm', 0, 0); // Ultraschall darf nicht Daten senden
          System.ModeNew = false;
        }
        /* Abrage:: Neue Fahrdaten*/
        if (System.DriveValueNew == true) {
          // Ansteuern des Antriebs:
          Drive.writeValue(System.DriveValue[0]);
          // Ansteuern der Lenkung: 
          Steer.setAngle(System.DriveValue[1]);  
          uint8_t USonicData[2] = {0, System.DriveValue[1]};
          RS485.transmit("U0", 'S', 's', 2, USonicData);

          System.DriveValueNew = false;
        }
      } break;



    //**************************************************************//
    //******************** AUTOMATIK-FAHR MODUS ********************//
    //** Fahren über Sensorik
    //**    ->
    //**    ->
    case 2: {
        /* Mode-Wechsel Start-Routin */
        if (System.ModeNew == true) {
          Drive.writeValue(128);  // Antrieb wird zum Stillstand gebremst          
          Steer.setAngle(45);     // Lenkung wird Geradeaus gerichtet
          System.DriveValueOld[0] = 128;
          System.DriveValueOld[1] = 45;
          RS485.transmit("I0", 'M', 'a', 1, 0); // Infrarot darf Daten senden
          RS485.transmit("U0", 'M', 'a', 1, 0); // Ultraschall darf  Daten senden
          //if(System.ActiveSensor[0] == false){
            // Wenn kein Sensor-System erkannt wurde dann kann Automatik nicht aktiv sein
          //  System.Mode[1] = 0;
          //}
          System.ModeNew = false;
        }
        switch(System.Mode[1]){
          case 0: /* Automatik Stop */            
          break;
          case 1: /* Automatik Start */

            /* Festlegen der Fahrtrichtung
             *  if(): Abfrage ob Rückwärts 
             *      - Ergebnis: DriveDirection = 1
             *  else if(): Abfrage ob Vorwärts
             *      - Ergebnis: DriveDirection = 0
             */
            if(System.DriveDirection == 0 && System.IRDistance <= System.NearField[0]){
                System.DriveDirection = 1; 
            }else if (System.DriveDirection == 1 && System.IRDistance > System.NearField[1]){
              System.DriveDirection = 0;
            }
  
            /*  Ermittlung der Geschwindigkeit
             *    - Abstand zum Hindernis
             *    - 
             */
            if(System.DriveDirection == 0){
              System.DriveValue[0] = System.changeRange(System.IRDistance, System.NearField[0], System.maxRange, 131, 255);
            } else if(System.DriveDirection == 1){
              System.DriveValue[0] = System.changeRange(System.IRDistance,   0, System.NearField[1], 0,   125);
            }
            /* Ansteuern des Antriebs */
            if(System.DriveValue[0] != System.DriveValueOld[0]){
              Drive.writeValue(System.DriveValue[0]);   
              System.DriveValueOld[0] = System.DriveValue[0];   
            } 

            /*  Ermittlung des Lenkwinkels
             *    - 
             *    - 
             */
            if(System.DriveDirection == 0){
                System.DriveValue[1] = 90 - System.IRAngle;
            } else if(System.DriveDirection == 1){
              System.DriveValue[1] = System.IRAngle;
            }
            /* Ansteuern der Lenkung */
            if(System.DriveValue[1] != System.DriveValueOld[1]){
              Steer.setAngle(System.DriveValue[1]);   
              System.DriveValueOld[1] = System.DriveValue[1];   
            } 
            
          break;
        }


        //* Bestätigung des Erhalts von Daten *//
        if (System.IncomeSensor == 'I'){
          // Wenn Sensordaten eingelesen wurden und nicht der Parametriermodus aktiv ist
          RS485.transmit("I0", 'S', 's', 1, &System.SendTrue);
          System.IncomeSensor = 0;
        }
        if (System.IncomeSensor == 'U'){
          // Wenn Sensordaten eingelesen wurden und nicht der Parametriermodus aktiv ist
          RS485.transmit("U0", 'S', 's', 1, &System.DriveValue[1]);
          System.IncomeSensor = 0;
        }

      } break;
  }//END switch(System.Mode[0])
}//END loop