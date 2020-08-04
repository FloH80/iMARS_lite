#include <Arduino.h>

#include "System.h" /* Systemsteuerung */
SysControl System;

#include <RS485.h>

#include <servoAusrichtung.h>

/**********RS BUS Anschlüsse***********/
#define com_ctrl_Pin_in       22   //gelb
#define com_ctrl_Pin_out      23   //gelb
#define tx_rx_ctrl_Pin        24   //orange

RS485 Rs485(Serial2, com_ctrl_Pin_in, com_ctrl_Pin_out, tx_rx_ctrl_Pin);

/**********SERVO Anschlüsse***********/
#define servoRPin  A7  //rechts
#define servoLPin  A6  //links
#define servoBPin  A4  //hinten

servoAusrichtung myServos;

/* Anschluss der Enfernungssensonren an den Analogpins*/
#define disPin1               A8   //rechts
#define disPin2               A9 //links
#define disPin3               A10   //hinten

uint8_t distance[3];
uint8_t distanceOld[3] = {0, 0, 0};                    // auf 0-200cm -> erstes Byte ist die Ausrichtung

void read_sensor(); //liest die Spannugn von den US-Sensoren ein, welche über die Widerstände abfällt 
void display_dis(); //schickt die Entfernung über die serielle Schnittstelle zum Nano zur Visualisierung auf den LED Ringen
void send_brighness();

void setup() {

  Serial.begin(115200);

  Rs485.init("U0", 115200);

  /* Initialisierung RS485
      - Kommunikation, iMARS_lite intern
      - [in1] Adresse : "C00" Control Nr.00
      - [in2] Baudrate: 115200 Bit/s
  */
  /* Rs485.init("U0", 115200);
    do{
      System.ConvertRS485Data(Rs485.receive());
      }while(System.Heartbeat == false);  */

  myServos.init(servoRPin, servoLPin, servoBPin); //R  //L //B

  Serial1.begin(115200);      //UART für die Übertragung der Entfernung an den Nano
}

void loop() {
  //* Einlesen und verarbeiten von BUS-Daten *//
  
  System.ConvertRS485Data(Rs485.receive());

  //Serial.print(System.DriveValue[1]);Serial.println("\t Hauptprogramm");

  read_sensor();
  
  
  switch(System.Mode[0]){  

    //**************************************************************//
    //********************** PARAMETRIER MODUS *********************//    
    case 0:
      if (System.ModeNew == true) {
        Serial.println("Parameter Modus");
        System.ModeNew = false;
      }


    break;

    //**************************************************************//
    //******************** MANUELLER-FAHR MODUS ********************//
    case 1:
      if (System.ModeNew == true) {
        Serial.println("Manueller Modus");
        System.DriveValue[0] = 128;
        System.DriveValue[1] = 45;
        System.ValueSend = 0;
        System.ModeNew = false;
      }       
      //******************** SERVOS AUSRICHTEN ********************//
      myServos.activeSensorSteering();

      myServos.umrechnen();



    break;

    //**************************************************************//
    //******************** AUTOMATIK-FAHR MODUS ********************//
    //********************** DATENAUSWERTUNG ***********************//
    case 2:
      if (System.ModeNew == true) {
        Serial.println("Automatischer Modus");
        System.DriveValue[0] = 128;
        System.DriveValue[1] = 45;
        System.ValueSend = 1;
        System.ModeNew = false;
      }

    //***** Auswahl der Sensordatenverarbeitung *****//
      switch (System.SensorMode) {

      case 'd':
        //***** Bilden der Differenz *****//
        if (System.DriveDirectionChange) myServos.ausrichten();
        // bei Richtungswechsel Servos ausrichten
        myServos.activeSensorSteering();
        // Sensoren mitlenken
        System.SendValue[0] = highByte(System.DifferentialEcho);
        System.SendValue[1] = lowByte(System.DifferentialEcho);
        System.SendValue[2] = System.DifferentialKritEcho;
        //System.SendValue[3] = System.BackEcho;
        System.SendValueCounter = 3;
        
        break;

        case 'n':
        //***** Nächstliegender relevanter Wert *****//
        if (System.DriveDirectionChange) myServos.ausrichten();
        // bei Richtungswechsel Servos ausrichten
        myServos.activeSensorSteering();
        myServos.umrechnen();
        //normiert, Nullpunkt Antriebsache.
        System.SendValue[0] = System.USnormDistanceKrit;
        System.SendValue[1] = System.USnormAngelKrit;
        break;

        case 'o': 
        //***** Hindernis 2D-Betrachtung *****//
        if (System.DriveDirectionChange) myServos.ausrichten();
        // bei Richtungswechsel Servos ausrichten
        myServos.activeSensorSteering();
        myServos.umrechnen();
        //Winkel und Enternung bezogen auf Antriebsachse
        System.SendValue[0] = System.USnormDistance[0];
        System.SendValue[1] = System.USnormDistance[1];
        System.SendValue[2] = System.USnormAngel[0];
        System.SendValue[3] = System.USnormAngel[1];
        //Matrix. normiert, Nullpunkt Antriebsache. Fahrtichtung rechts: negativ. Fahrrichtung links: positiv
        /* System.SendValue[0] = System.Distance3DArray[0][0];
        System.SendValue[1] = System.Distance3DArray[0][1];
        System.SendValue[2] = System.Distance3DArray[1][0];
        System.SendValue[3] = System.Distance3DArray[1][1]; */
        break;
      }
    break;
  }
  
  

  //******************** DATEN SENDEN ********************//
  if ( /* System.NewValues == true   &&*/  System.ValueSend == 1) {
    Rs485.transmit("C0", 'U', System.SensorMode, System.SendValueCounter, System.SendValue);
    Serial.print((System.SendValue[0]<<8)+System.SendValue[1]);
    Serial.print("\t");Serial.println(System.SendValue[2]);
    System.ValueSend = 0;
  }
  
  if (System.NewValues) display_dis();
}


void read_sensor() 
{//Sensor auslesen in mm und in cm umrechnen
  //rechts
  distance[0] = (((analogRead(disPin1) * 0.488) - 100) / 0.2) / 10; //  500mV/2^10 Bits  =  0,488mV/Bit --- 1...5V 100...2000mm 400mV/2000mm = 0.2 ---
  //if (distance[0] < 0) distance[0] = 0;
  System.FrontEcho[0] = distance[0];
  //links
  distance[1] = (((analogRead(disPin2) * 0.488) - 100) / 0.2) / 10;
  //if (distance[1] < 0) distance[1] = 0;
  System.FrontEcho[1] = distance[1];
  //hinten
  distance[2] = (((analogRead(disPin3) * 0.488) - 100) / 0.2) / 10;
  //if (System.BackEcho < 0) System.BackEcho = 0;
  System.BackEcho = distance[2];

  System.DifferentialEcho = distance[0] - distance[1];

  if(distance[0]<distance[1]) System.DifferentialKritEcho = distance[0];
  if(distance[1]<distance[0]) System.DifferentialKritEcho = distance[1];

  if (distanceOld[0] != distance[0] || distanceOld[1] != distance[1] /* || distanceOld[2] != distance[2] */ || System.NewBrightness)
  {//wenn sich die Entfernungswerte geändert haben 
    System.NewValues = true;
    for (int u = 0; u < 3 ; u++) distanceOld[u] = distance[u];
  }
  //Serial.print(System.FrontEcho[0]);Serial.print("\t");Serial.println(System.FrontEcho[1]);
}

void display_dis() 
{//Printet die Entfernung in den seriellen Port 1, um mit dem Nano zu kommunizieren
  Serial1.print('U');
  Serial1.write(distance[0]);
  Serial1.write(distance[1]);
  /* Serial1.write(System.BackEcho); */Serial1.write(0);
  Serial1.write(System.LEDBrighnesss);
  System.NewValues = false;
  System.NewBrightness = false;
  //Serial.print(distance[0]);Serial.print("\t");Serial.println(distance[1]);
}