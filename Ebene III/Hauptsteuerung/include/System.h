//*******************************************************************************//
//                                 SYSTEMSTEUERUNG                               //
//*******************************************************************************//
//  Autor: Florian Harnack
//  - Hier werden die Globalen Variablen deklariert
//  
//
//*******************************************************************************//


#ifndef System_h
#define System_h

class SysControl{
  public:
    //*** FEHLERERKENNUNG ***//
    /* [0]: 
     * [1]:
     * [2]:
     * [3]:
     * [4]:
     * [5]:
     * [6]:
     */
    bool Error[10];


    //*** BEDIENERMODUS ***//
    /*  Mode[2]
     *      [0]: Modus 
     *        0: parametrier Modus(Start Modus)
     *        1: manueller Modus
     *        2: automatik Modus
     *        3: Folgen Modus
     *        4: GPS Modus
     *      [1]: Aktivitätsstatus 
     *        true:   Aktiv   - Fahren möglich
     *        false:  Standby - Fahren nicht möglich
     */  
    uint8_t Mode[2]; 
    bool ModeNew = false;


    //*** PARAMETER ***//
    /*  Parameter[][]
     *    [#][#]: Antriebsystem
     *     0: Antrieb vorne
     *        0: Maximale Fahrgeschwindigkeit Vorwärts
     *        1: Maximale Fahrgeschwindigkeit Rückwärts
     *     1: Lenkung
     *        0: Geschwindigkeit der Lenkung
     *        1: Ausrichtung der Lenkung 
     *   [1#][#]:  
     *   [2#][#]: Sensorsteuerung :: Infrarotsysstem: P+F R2100 
     *     0: Sensorsteuerung :: Sensormodus 
     *     1: Sensorsteuerung :: Relevante Distanz zur ausführung der Modi
     *     2: Visualisierung  :: Maximale Distanz für die Visualisierung
     *     3: Visualisierung  :: LED-Stripe Helligkeit
     *   [3#][#]: Sensorsteuerung :: Ultraschallsysteme: P+F UC2000
     *   [4#][#]: 
     *   [5#][#]:
     *   [6#][#]:
     *   [7#][#]:
     *   [8#][#]:
     *   [9#][#]:
     *  [10#][#]:
     */
    uint8_t Parameter[100][10]; 
    bool ParameterNew[100];
    uint8_t ParameterPointer = 0;


    //*** FAHRDATEN ***//
    /* Fahrdaten: Aktor-Ansteuerung
     *  DriveValue[2]::
     *    [0]: Geschwindigkeitswert  
     *    [1]: Lenkwinkel
     *  DriveValueNew::
     *    Info über neue Werte
     */
    bool DriveValueNew = false;
    uint8_t DriveValue[2];
    uint8_t DriveValueOld[2];

    /* Fahrdaten: Fahrtrichtung
     *  DriveDirection::
     *    0: Vorwärts
     *    1: Rückwärts
     *  DriveDirectionChange::
     *    0: keine Änderung vorhanden
     *    1: Änderung vorhanden
     */
    bool DriveDirection = 0;
    bool DriveDirectionChange = 0;

   uint8_t NearField[2] = {25, 50};

    //*** SENSORDATEN ***//
    /* Sensordaten: Aktive Sensoren
     *  ActiveSensor[4]::
     *    [0]: Infrarot-Sensor      (P+F)
     *    [1]: Ultraschall-Sensoren (P+F)
     *    [2]: Radar-Sensor         (SMi)
     *    [3]: Kamera               (IFM)
     *  True: Sensor da , False: Sensor nicht da
     */ 
    bool ActiveSensor[4];

    /* Sensordaten: Sendender Sensor
     *  IncomeSensor:
     *    'I': Infrarot-Sensor      (P+F)
     *    'U': Ultraschall-Sensoren (P+F)
     *    'R': Radar-Sensor         (SMi)
     *    'K': Kamera               (IFM)
     */
    char IncomeSensor;
    
    /* Sensordaten: Bestätigung für erhaltende Sensordaten
     *  SendTrue[1]:
     *     Wert zum bestätigen von erhaltenen Sensordaten 
     */
    uint8_t SendTrue = 1;

    uint16_t maxRange = 200;

    //*** INFRAROTSENSOR - DATEN ***//
    char IRMode = 'n';
    uint16_t IRDistance = 800;
    uint8_t IRAngle = 45;

    //*** INFRAROTSENSOR - DATEN ***//
    int16_t UDifference = 0;
    uint8_t UDistance = 200;


    int32_t changeRange(int32_t in_value, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
      if(in_value < in_min){in_value = in_min;}
      else if(in_value > in_max){in_value = in_max;}
      return (in_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }   

  //*******************************************************************************//
  //                            VARIABLEN INITIALISIEREN                           //
  //*******************************************************************************//    
    
    void initVariables(){
      ParameterNew[0] = false;
      ParameterNew[1] = false;
      DriveValue[0] = 128;
      DriveValue[1] = 45;
    }


  //*******************************************************************************//
  //                             STEUER-BUS AUSLESEN                               //
  //*******************************************************************************//
  void ConvertRS485Data (uint8_t *Puffer){
    switch (Puffer[0]) {
      case 'P':   //* Paramter *//
      break;
      case 'I':   //* Sensordaten: Infrarotsensor *//
        IncomeSensor = 'I';
            /* Nachricht: [I] [d | h | n | o] [0 | 1] [*]
            *  M: Modus  - Betriebsmodus
            *  [d | h | n | o]::
            *    d: Differenzbildung entgegengesetzter Spuren
            *    h: manueller Modus
            *    n: nächstliegender relevanter Wert
            *    o: Hindernis 2D-Betrachtung
            *  [0 | 1]  
            *    0: Standby
            *    1: in Betrieb
            */        
        if (Puffer[1] == 'd') {       // Differenzbildung entgegengesetzter Spuren
          /* Daten einlesen */
        }
        
        else if (Puffer[1] == 'n') { // nächstliegender relevanter Wert
          /* Daten einlesen */
          IRMode = 'n';
          IRDistance = (Puffer[2] << 8) + Puffer[3]; // High-Byte und Low-Byte zusammensetzen
          IRAngle    = Puffer[4];
        }
        
        else if (Puffer[1] == 'o') { // Hindernis 2D-Betrachtung
          /* Daten einlesen */
        }
      break;


      case 'U':   //* Sensordaten: Ultraschallsensor *//
        IncomeSensor = 'U';

        if(Puffer[1] == 'd'){
          UDifference = (Puffer[2] << 8) + Puffer[3];
          UDistance = Puffer[4];
        }

      break;


      default:  break;
    }//END switch(Puffer[0])
  }//END ConvertRS485Data

};

#endif
