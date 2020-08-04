//*******************************************************************************//
//                                R2100 STEUERUNG                                //
//*******************************************************************************//
//                                                                               //
//  Was wird hier gemacht?                                                       //
//  - Steuerung der Datenabfrage des Sensors R2100 der Firma Pepperl+Fuchs       //
//  - Datenkonvertierung von den Rohdaten des Sensors zu Entfernungswerten       //
//  - Es gibt 2 Funktionen:                                                      //
//      -> getdata() : Kommunikation mit dem Sensor - erhalten der Rohdaten      //
//      -> getEchos(): Verarbeitung der Rohdaten                                 //
//  Kommunikation mit dem Sensor                                                 //
//  - Der Sensor wird über RS232 angesteuert                                     //
//  - Kommunikation seitens des Arduino DUE über UART-Port 2 (Pin 16 & 17)       //
//  - Verbindung von Sensor und Arduino über MAX232                              //
//                                                                               //
//                                                                               //
//*******************************************************************************//

#ifndef R2100_h
#define R2100_h

#include "Arduino.h"

class R2100{
  private:  //*** PRIVATE ***//
    //*** ***********Variablen**************** ***//
    Stream *SerialPort;
    HardwareSerial *HardSerial;

    const uint16_t R2100_max_range = 800; /* maximale Reichweite
                                              - nur Softwareseitig
                                              - 800 cm <-> 8 m
                                          */

    /* Werte zur Datenanfrage an den Sensor (Vorgegeben durch P+F Serial Protocol)
       0xde: Receiver ID
       0x01: Sender ID
       0x05: Länge des Frame
       0x59: Befehlt
       0x83: Checksumme
    */                                         
    const uint8_t Request_Distance[5] = {0xde , 0x01 , 0x05 , 0x59 , 0x83};

    /* Zur Zeitsteuerung der Datenabfrage */
    uint32_t Time_before_Request = 0; 
    /* Zur Zeitsteuerung der Datenabfrage */
    uint32_t Time_after_Request  = 0; 
    /* Rohdaten des des Sensors: 50 Bytes */
    uint8_t Response_Distance[50];    

    //*** ************************************ ***//

    //*** Funktion: Erhalt der Entfernungswerte der Echos
    //*  Eingabe-Werte:  - Keine
    //*  Rückgabe-Werte: - bool new_measurement
    //*  - Kommunikation mit Sensors über RS232 (UART-Port 2)
    //*  - Zeitsteuerung zum Auslesen alle 20 ms
    //***
    bool getdata() {
      bool new_measurement = false; /* Rückgabewert: Neue Messwerte vorhanden  */
      /* Zeitsteuerung über Funktion millis()
          Abfrage: ob seit der letzten Messung 20 ms vergangen sind
           - Ergibt sich aus: (Zeit vor Messung) - (Zeit nach Messung)
          Wenn Ja: Abfrage neuer Messwerte
          Wenn Nein: Rückgabe, keine neuen Messwerte
      */
      Time_before_Request = millis();
      if (Time_before_Request - Time_after_Request >= 20) {
        /* Abfrage der Messwerte
            - Senden von 5 Bytes als char
            - Warten bis alle aus dem Ausgangspuffer gesendet wurden (.flush())
        */
        SerialPort->write(Request_Distance[0]);
        SerialPort->write(Request_Distance[1]);
        SerialPort->write(Request_Distance[2]);
        SerialPort->write(Request_Distance[3]);
        SerialPort->write(Request_Distance[4]);
        SerialPort->flush();
        if (SerialPort->available()) {
          /* Einlesen der Antwort des Sensors, auf die Anfrage
             - Es werden 50 Bytes erwartet
             - nachträglich Rückgabewert auf true setzten
          */
          SerialPort->readBytes(Response_Distance, 50);
          new_measurement = true;
        }
      } else {
        return new_measurement = false;
      }
      /* Routine: Nur wenn neue Messwerte erhalten wurden
          - aktueller millis-Wert abspeichern
      */
      Time_after_Request = millis();
      return new_measurement;
    }


  public:
    //*** *************Variablen************** ***//
    uint16_t Distance[11]; /* Entfernungswerte der einzelnen Echos */
    uint8_t Angle[11] = {85, 77, 69, 61, 53, 45, 37, 29, 21, 13,  5};

    /* Relevante Distanz
     *  relevante Distanz des jeweiligen Echos in [cm]
     *  - < rel. Distanz: Hindernis auf der Fahrstrecke
     *  - berechnete Werte: 33,40,52,77 ,151,151,151,77, 52,40,33
     *  - Ist bei Erhöhung der Geschwindigkeit anzupassen (Präventiverkennung)
     *  - = 70,77,89,104,120,120,120,104,89,77,70
     *  - letzte: 46, 56, 73, 108 , 120 , 120, 120, 108, 73, 56, 46
     */
    uint16_t RelevanceDistance[11] = {100,120,140,170,190,200,190,170,140,120,100};

    


    //*** ************************************ ***//

    //*** Funktion: Initialsierung ***//
    //*  - Initialisieren der Schnittstelle
    //*  - Festelegen des UART-Ports für die Kommunikation
    //*  - Baudrate festgelegt auf 115200Bd
    //***
    R2100(HardwareSerial &PORT) {
      HardSerial = &PORT;
      SerialPort = &PORT;
      HardSerial->begin(115200);
    }
    //*** ************************************ ***//

    void initRelevanceDistance(uint16_t maxRelevanceRange){
      uint16_t Factor = maxRelevanceRange - RelevanceDistance[5];
      for(uint8_t i = 0; i < 11; i++){
        RelevanceDistance[i]  = RelevanceDistance[i] + Factor;
      }
    }

    //*** Funktion: Umrechenen der Sensorwerte ***//
    //*  - Umwandeln der Sensordaten von:
    //*     -> HIGH/LOW-Byte <-> Entfernungswert
    //*  - Umrechnen der Werte in cm
    //*  - Ausgeben der Entfernungswerte
    uint16_t getEchos() {
      uint8_t Byte = 4; /* zur Verschiebung der zuverarbeitenden Bytes des Sensors */
      if (getdata() == true) {
        //Serial.println("Neue Messung");
        for (uint8_t Echo = 0 ; Echo < 11; Echo ++) {
          /* Umrechnung der Daten des Sensors zu Entfernungswerten
              - Beginnt ab Byte 4 (erste 4 Bytes = Header)
              - Distanzwert besteht aus 2 Byte
                -> LOW(1) & HIGH(2) Byte
              - Nach Distanzbytes folgen 2 Echo-Bytes
          */
          Distance[Echo] = 0; /* Löschen des alten Distanzwertes */
          Distance[Echo] = ((Response_Distance[Byte + 1] << 8) + Response_Distance[Byte]) / 10; /*Ausgabe in mm, Umrechnung in cm */
          Byte += 4; /* Springt zum nächsten Distanzwert */
          if (Distance[Echo] > R2100_max_range || Distance[Echo] < 1) {
            /* Begrenzung des maximalen Wertes auf 8 meter */
            Distance[Echo] = R2100_max_range;
          }       //Serial.print(Distance[Echo]);Serial.print("\t");Serial.print(Angle[Echo]);Serial.print("\t");
        }         //Serial.println();
        return true;
      } 
      
      else {
        return false;
      }
    }
    //*** ************************************ ***//

    //***** *********************************************************** *****//
    //*****               DATENAUSWERTUNG DER SENSORDATEN               *****//
    //***** *********************************************************** *****//

    //*** ****Funktion: Differenzbildung**** ***//
    //*  Bilden der Differenz zwischen den außenliegenden Echos
    //*   -> Einfluss auf die Fahrtrichtung: rechts, links
    //*
    //*   Eingabe-Werte:  - EchoPos;
    //*   Rückgabe-Werte: - diff; Differenz in Einheitswerten +/-
    int8_t DifferentialCalc(uint8_t EchoPos) {
      int16_t diff = (Distance[EchoPos] - Distance[10 - EchoPos]);
      return System.changeRange(diff, -600, 600, -128, 128);
    }
    //*** ************************************ ***//

    //*** *****Funktion: Objekterkennung****** ***//
    //*  Eingabe-Werte:  - relevance_distance[11]: Distanz ab welcher der Nahbereich beginnt
    //*  Rückgabe-Werte: - object_inside_near_field
    //*
    //*  Es wird immer dem nahesten Objekt ausgewichen!
    uint8_t ObjektRecognition() {
      bool recognite = false;      /* Merker ob ein Objekt erkannt wurde */
      uint8_t recognition_note[2]; /* Merker für den Ausweichwinkel */
      recognition_note[0] = 0;
      recognition_note[1] = 0;
      /* Prüfen: Liegen gemessene Objekt im relevanten Bereich
          - rel. Bereich: Messbereich der auf der Fahrstrecke liegt
      */
      for (uint8_t Echo = 0; Echo < 11; Echo += 1) {
        /* Abfrage: Echo-Wert im relevanten Bereich
            - Ja  : Echo-Nr wird gespeichert
            - Nein: Nächster Wert wirtd verglichen
        */
        if (Distance[Echo] <= RelevanceDistance[Echo]) {
          /* Abfrage: bereits ein Wert im relevanten bereich
              - Nein : wird im ersten speicher gespeichert
              - Ja : wird im zweiten speicher gespeichert
          */
          if (recognite == false) {
            /* Position des Überienstimmenden Wertes aus Array Distance[] wird gespeichert */
            recognition_note[0] = Echo;
            recognite = true;
          } else if (recognite == true) {
            recognition_note[1] = Echo;
            /* Abfrage: ist der Distanzwert von [0] > [1]:
                - Ja: [0] = [1] ([1] wird auf [0] geschrieben)
                - Nein: Es passiert nichts, geringste Entfernung bereist auf [0]
            */
            if (Distance[recognition_note[0]] > Distance[recognition_note[1]]) {
              recognition_note[0] = recognition_note[1];
            }
          }
        } /* Ende Abfrage der Relevanz */
      }   /* Ende Echo-Aufzählung */

      /* Abfrage: Hindernis erkannt
         - Nein: geradeaus weiterfahren
         - Ja: Prüfen ob geradeaus liegt
      */
      if (recognite == false) {
        recognition_note[0] = 5;
      } else if (recognition_note[0] == 5) {
        /* Falls Hindernis mittig liegt
           Abfrage: Auf welcher Seite ist ist die Entfernung am größten
            -> Ausweiche in die Richutng mit größter Entfernung
        */
        if (Distance[4] > Distance[6]) {
          recognition_note[0] = 4;
        } else {
          recognition_note[0] = 6;
        }
      }
      return recognition_note[0];
    }
    //*** ************************************ ***//
};
#endif
