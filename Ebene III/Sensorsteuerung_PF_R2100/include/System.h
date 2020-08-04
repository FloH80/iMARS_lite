//*******************************************************************************//
//                                 SYSTEMSTEUERUNG                               //
//*******************************************************************************//
//  Autor: Florian Harnack
//
//  - Hier werden die Globalen Variablen deklariert
//  - Hier werden allgemeine Steuerungsfunktionen definiert  
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


//********** SENSORSTEUERUNG **********//
    /* Modus der Messdatenauswertung
     *  'n':  nächstliegender relevanter Wert
     *  'o':  Hindernis 2D-Betrachtung
     *  'd':  Differenzbildung entgegengesetzter Spuren
     */
    char SensorMode = 'n';

    /*** Messdatenverarbeitung ***/

    /* Merker für neue Sensorwerte
     *  True: neue Messwerte vorhanden
     *  False: keinen neuen Messwerte
     */
    bool NewSensorValues = false;

    /* EchoSpur zur Differenzbildung
     *  - Differenzbildung erfolgt anhand gegenüberliegender Echos
     *  - Wert kann zwischen 0 und 4 liegen
     */
    uint8_t DifferentialEcho = 0;

    //*** Umrechnungsfunktion ***//

    int32_t changeRange(int32_t in_value, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
      if(in_value < in_min){in_value = in_min;}
      else if(in_value > in_max){in_value = in_max;}
      return (in_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }   
    

  //*******************************************************************************//
  //                                VISUALISIERUNG                                 //
  //*******************************************************************************//

    //********** VARIABLEN **********//

    uint8_t     CloseRange          = 25;
    uint16_t     VisualDistance[11]  = {100,120,140,170,190,200,190,170,140,120,100};
    uint8_t     Brightness_LED      = 255;  

    //********** FUNKTIONEN **********//


    void initVisualisationDistance(uint16_t VisualisationRange){
      int16_t Factor = VisualisationRange - VisualDistance[5];
      Serial.print(VisualisationRange);Serial.print("\t");Serial.println(Factor);
      for(uint8_t i = 0; i < 11; i++){
        VisualDistance[i]  = VisualDistance[i] + Factor;
      }
    }
    
    void EchoVisualisation(uint16_t Distance[11]) {
        if(NewSensorValues == true){
            Serial3.write('R');
            for (uint8_t Echo = 0; Echo <= 10; Echo += 1) {
                Serial3.write(changeRange(Distance[Echo], CloseRange, VisualDistance[Echo], 0, 255));
            }
            Serial3.write('*');
        }
    }

    //*******************************************************************************//
    //                               KOMMUNIKATION                                   //
    //*******************************************************************************//


    //********** VARIABLEN **********//
    /*  Heartbeat
     *   - Wird gesetzt, wenn der Master 'C0' die Anwesenheit abfragt
     */
    bool Heartbeat = 0;

    /* Senden der Sensordaten
     *  SensorValue[64]:
     *  - Inhaltliche Anordnung ändert sich je nach Auswerte-Methode
     *  SensorValCnt:
     *  - Zähler für den Puffer
     *  SensorValueSend:
     *  - Merker für das Daten-Senden
     */
    uint8_t SendValue[64];          /* Puffer max 64 Byte - max. Empfangspuffer */
    uint8_t SendValueCounter = 0;   /* Zähler für den SensorValue-Puffer */
    bool    ValueSend = false;      /* Merker für Daten-Senden */

    void ConvertRS485Data(uint8_t *Puffer) {
        
        switch (Puffer[0]) {
            case 'I':
            /* Nachricht: [I] [h] [*]
            *  I: Infrarot - Initialisierung
            *  [h]:: Heartbeat
            *    h: parametrier Modus(Start Modus)
            */            
                if(Puffer[1] == 'h'){
                    Heartbeat = true;
                }
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
                if(Puffer[1] == 'p'){
                    Mode[0] = 0;
                }else if(Puffer[1] == 'm'){
                    Mode[0] = 1;
                }else if(Puffer[1] == 'a'){
                    Mode[0] = 2;
                }else if(Puffer[1] == 'f'){
                    Mode[0] = 3;
                }else if(Puffer[1] == 'g'){
                    Mode[0] = 4;
                }
            
            ModeNew = true;        
            break;
            case 'P': 
                Serial.write(Puffer[0]); Serial.print("\t"); Serial.write(Puffer[1]);Serial.print("\t");
                /*  Nachricht: [P] [b | l | r | s] [...] [*]
                 *   P: Parameter 
                 *    [ | s]::
                 *      s: Sensormodus
                 *      r: Relevante Sensorreichweite
                 *      l: LED-Entfernung
                 *      b: LED-Helligkeit
                 *    [...]::
                 *      Parameter-Werte
                 */
                if(Puffer[1] == 's'){
                    /* Parameter-Werte
                     *  [0]: ['d']: Differenzbildung                
                     *  [0]: ['n']: nächstliegender Relevanter Wert 
                     *  [0]: ['o']: 2D-Objekterkennung              
                     */
                    Parameter[20][0] = Puffer[2];
                    ParameterNew[20] = true;
                }
                else if(Puffer[1] == 'r'){
                    Parameter[21][0] = Puffer[2];  // High Byte
                    Parameter[21][1] = Puffer[3];  // Low Byte
                    ParameterNew[21] = true;
                     Serial.print(Parameter[21][0]); Serial.print("\t"); Serial.println(Parameter[21][1]);
                }

                else if(Puffer[1] == 'l'){
                    Parameter[22][0] = Puffer[2];  // High Byte
                    Parameter[22][1] = Puffer[3];  // Low Byte
                    ParameterNew[22] = true;
                    Serial.print(Parameter[22][0]); Serial.print("\t"); Serial.println(Parameter[22][1]);
                }

                else if(Puffer[1] == 'b'){
                    Parameter[23][0] = Puffer[2];
                    ParameterNew[23] = true;
                    Serial.println(Parameter[23][0]);
                }


            break;
            case 'S':
            /*  Nachricht: [S] [s] [...] [*]
             *   S: Sensor
             *      [s]:: senden
             *       0: darf nicht senden
             *       1: darf senden
             */   
                if (Puffer[1] == 's') {
                    if(Puffer[2] == 0){
                        ValueSend = false;
                    }
                    else if (Puffer[2] == 1){
                        ValueSend = true;
                    }
                }
            break;
            default:  break;
        }
    }

};// ENDE Klasse
#endif