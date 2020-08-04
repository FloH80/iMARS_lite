//*******************************************************************************//
//                                    BLUETOOTH                                  //
//*******************************************************************************//

//*******************************************************************************//

#ifndef Bluetooth_h
#define Bluetooth_h

class Bluetooth{
  private: //*** PRIVATE ***//
    Stream *mySerial;
    HardwareSerial *hs;


    int32_t changeRange(int32_t in_value, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
        if(in_value < in_min){in_value = in_min;}
        else if(in_value > in_max){in_value = in_max;}
        return (in_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

  public: //*** PUBLIC ***//

//*******************************************************************************//
//                                  VARIABLEN                                    //
//*******************************************************************************//

 
  /* Visualisierungs Aktivierungsmerker (Bluetooth)
   *  [0]: R2100-Werte (Sensor 1)
   *  [1]: R2100-Werte (Sensor 2)
   *  [2]: Fahrgeschwindigkeit & Lenkwinkel
   *  [3]: Ultraschall
   *  [4]: RP-Lidar
   */
    bool Visualization[10]; 
  
//*******************************************************************************//
//                          INITALISIERUNG KLASSE                                //
//*******************************************************************************//
//  - Initaliserung der Bluetooth-Klasse
//  - Paramter[in]  &port
//                  SerialPort: Serial, Serial1, Serial2, Serial3 
//*******************************************************************************//

    Bluetooth(HardwareSerial &port){
      hs = &port;
      mySerial = &port;
    }
//*******************************************************************************//
//                           INITALISIERUNG UART                                 //
//*******************************************************************************//
//  - Initaliserung der Verbindung zwischen dem Ardino und dem Bluetooth-Modul
//  - Paramter[in]  Baudrate in Bd
//                    Übertragungsgeschwindigkeit UART 
//  - Paramter[in]  Timeout in ms
//                    Wartezeit auf Daten 
//*******************************************************************************//

    void init(int32_t BAUDRATE, uint8_t Timeout){      
      if(BAUDRATE > 115200){
        /* Baudratenbegrenzung
         * Größer 115200Bd wird vom Arduino nicht unterstützt
         */
        BAUDRATE = 115200;
      }  
      /* Initialisierung der UART-Schnittstelle */
      
      hs->begin(BAUDRATE);
      /* Festlegen der Wartezeit
       *  Timeout in ms 
       *  Dauer, wie lange auf eingehende Daten gewartet wird
       */
      mySerial->setTimeout(Timeout);
    }
//*******************************************************************************//
//                             BLUETOOTH AUSLESEN                                //
//*******************************************************************************//
//  - Auslesen der übertagenen Daten des Bluetooth-Moduls
//  - Datensatz besteht aus min. 3 Einheiten
//  - Software Protokoll: 
//        'Buchstabe' - 'Buchstabe' - Werte(einzelne Bytes) - '*' 
//    -      Index    -  Subindex   - Daten [1...20]        - Ende
//
//*******************************************************************************//

    void getOpertionsCommand(){
      uint8_t puffer[24];               // Puffer für Bluetooth-Daten 
      for(uint8_t i = 0; i < 24; i++){  // Puffer-Werte Nullen für Fehlerfindung
        puffer[i] = 0;
      }
      /* while()::durchläuft den ganzen UART-Puffer */
      while(mySerial->available() >= 4){
        mySerial->readBytesUntil('*', puffer ,24);
        switch(puffer[0]){
          case 'D':
            /* Nachricht: [D] [d|s|v] [0...255 (v-Wert) | 0...90 (s-Wert)] [*]
            *  D: Drive    - Fahrvorgabe
            *  d: drive    - Geschwindigkeits- und Lenkwinkelvorgabe
            *  s: steering - Lenkwinkelvorgabe 
            *  v: velocity - Geschwindigkeitsvorgabe
            */
              if(puffer[1] == 'd'){
                System.DriveValue[0] = puffer[2];  /* Geschwindigkeitswert */
                System.DriveValue[1] = puffer[3];  /* Lenkwinkel */
              }else if(puffer[1] == 's'){
                System.DriveValue[1] = puffer[2];  /* Geschwindigkeitswert */
              }else if(puffer[1] == 'v'){
                System.DriveValue[0] = puffer[2];  /* Lenkwinkel */
              }
              System.DriveValueNew = true;  
          break;
          case 'M':
            /* Nachricht: [M] [p | m | a | f | g] [0 | 1] [*]
            *  M: Modus  - Betriebsmodus
            *  [p | m | a | f | g]::
            *    p: parametrier Modus(Start Modus)
            *    m: manueller Modus
            *    a: automatik Modus
            *    f: Folgen Modus
            *    g: GPS Modus
            *  [0 | 1]  
            *    0: Standby
            *    1: in Betrieb
            */
            if(puffer[1] == 'p'){
              System.Mode[0] = 0;
              System.Mode[1] = puffer[2];
            }else if(puffer[1] == 'm'){
              System.Mode[0] = 1;
              System.Mode[1] = puffer[2];
            }else if(puffer[1] == 'a'){
              System.Mode[0] = 2;
              System.Mode[1] = puffer[2];
            }else if(puffer[1] == 'f'){
              System.Mode[0] = 3;
              System.Mode[1] = puffer[2];
            }else if(puffer[1] == 'g'){
              System.Mode[0] = 4;
              System.Mode[1] = puffer[2];
            }
            System.ModeNew = true; 
          break; 
          case 'P':
          /*  Nachricht: [P] [v | s] [...] [*]
           *   P: Parameter 
           *    [v | s]::
           *      v: drive    - Antriebsparameter 
           *      s: steering - Lenkparameter
           *    [...]::
           *      Parameter-Werte
           */
          if(puffer[1] == 'd'){
            if(puffer[2] == 'v'){
              System.Parameter[0][0] = puffer[3];
              System.Parameter[0][1] = puffer[4];
              System.ParameterNew[0] = true;
            }else if(puffer[2] == 's'){
              System.Parameter[1][0] = puffer[3];
              System.ParameterNew[1] = true;
            }
          }
          /*  Nachricht: [P] [i] [r | s | l | b] [...] [*]
           *   P: Parameter 
           *    [i]::
           *      v: Infrarot-Sensor
           *    [r | s]::
           *      r: range      - Relevante Reichweite des Infrarotsensors Spur-Mitte 
           *      s: sensormode - Sensordaten-Auswertungsmodus
           *      l: led range  - Entfernung für die Visualisierung des Infrarotsensors
           *      b: brightness - Helligkeit des LED-Stripes
           *    [...]::
           *      Parameter-Werte
           */
            else if(puffer[1] == 'i'){
              if(puffer[2] == 's'){
                System.Parameter[20][0] = puffer[3];
                System.ParameterNew[20] = true;
              }
              else if(puffer[2] == 'r'){
                System.Parameter[21][0] = puffer[3];  // High Byte
                System.Parameter[21][1] = puffer[4];  // Low Byte
                System.ParameterNew[21] = true;
              }
              else if(puffer[2] == 'l'){
                System.Parameter[22][0] = puffer[3];  // High Byte
                System.Parameter[22][1] = puffer[4];  // Low Byte
                System.ParameterNew[22] = true;
              }
              else if(puffer[2] == 'b'){
                System.Parameter[23][0] = puffer[3];
                System.ParameterNew[23] = true;
              }          
            }
          break; 
        }
      }


    }// END getOpertionsCommand()
};
#endif
