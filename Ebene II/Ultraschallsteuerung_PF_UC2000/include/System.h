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
//********** FEHLERERKENNUNG **********//
    /* [0]: 
     * [1]: Lenkung
     * [2]:
     * [3]:
     * [4]:
     * [5]:
     * [6]:
     */
    bool Error[10];


//********** BEDIENERMODUS **********//
    /*  operationMode[2]
     *  [0]: Modus 
     *    0: parametrier Modus(Start Modus)
     *    1: manueller Modus
     *    2: automatik Modus
     *    3: Folgen Modus
     *    4: GPS Modus
     *  [1]: Aktivitätsstatus 
     *    true:   Aktiv   - Fahren möglich
     *    false:  Standby - Fahren nicht möglich
     */  
    uint8_t Mode[2]; 
    bool ModeNew = false;


//********** PARAMETER **********//
    /*  parameter[][]
     *  [][#]:
     *     0: 
     *      0: 
     *      1: 
     *     1: 
     *      0: 
     *      1: 
     *     2:
     *      0:
     *      1:
     */
    uint8_t Parameter[2][2]; 
    bool    ParameterNew[2];



//********** SENSORSTEUERUNG **********//
    /* Modus der Messdatenauswertung
     *  'n':  nächstliegender relevanter Wert
     *  'o':  Hindernis 2D-Betrachtung
     *  'd':  Differenzbildung entgegengesetzter Spuren
     */
    char SensorMode = 'd';

    //*** FAHRDATEN ***//
    /* Fahrdaten: Aktor-Ansteuerung
     *  DriveValue[2]::
     *    [0]: Geschwindigkeitswert  
     *    [1]: Lenkwinkel
     *  DriveValueNew::
     *    Info über neue Werte
     */
    uint8_t DriveValue[2];
    uint8_t DriveValueOld[2] = {0 , 0};
    bool DriveValueNew = true;

    /* Fahrdaten: Fahrtrichtung
     *  DriveDirection::
     *    0: Vorwärts
     *    1: Rückwärts
     *  DriveDirectionChange::
     *    0: keine Änderung vorhanden
     *    1: Änderung vorhanden
     */
    bool DriveDirection = 0;
    bool DriveDirectionOld = 0;
    bool DriveDirectionChange = 0;

    /*** Messdatenverarbeitung ***/

    /* Merker für neue Sensorwerte
     *  True: neue Messwerte vorhanden
     *  False: keinen neuen Messwerte
     */
    bool NewValues = false; 

    /* EchoSpur zur Differenzbildung
     *  - Differenzbildung der beiden Vorderen US Sensoren
     */
    int16_t DifferentialEcho = 0;

    /* EchoSpur zur Differenzbildung
     *  - Differenzbildung der beiden Vorderen US Sensoren
     *  - setzt sich aus dem Delta (lowByte) und der kritischen Distanz zusammen (highByte)
     */
    uint8_t DifferentialKritEcho = 0;

    /* Echo der vorderen  Ultraschallsensorsen
    *   0 - rechts
    *   1 - links
     */
    uint8_t FrontEcho[2];

    /* Echo des hinteren Ultraschallsensors
    *
     */
    uint8_t BackEcho = 0;

    /* 2D Matrix + Höhe
    *   Bezugspunkt: Achse vorderes Antriebsrad 
    *   [rechts , links, hinten]
    *       [x - horizontal zur Fahrtrichtung] [y - in Fahrtrichtung] [h - Höhe]
    *   Objekt wird nur als Punkt betrachtet
     */
    float Distance3DArray[3][3];

    /* US Winkel
        Objekt wird nur als Punkt betrachtet
        Bezugspunkt: Achse vorderes Antriebsrad 
     */
    uint8_t USnormAngel[3] = {45, 45, 45};

    uint8_t USnormAngelKrit;

    /* US normierte Distanz
        Objekt wird nur als Punkt betrachtet
        Bezugspunkt: Achse vorderes Antriebsrad 
     */
    uint8_t USnormDistance[3] = {0, 0, 0};

    uint8_t USnormDistanceKrit;

//********** VISUALISIERUNG **********//

    uint8_t LEDBrighnesss = 15;
    bool NewBrightness = false;
    /* uint8_t Mod = 70;
    uint8_t VisualDistance[11] = {120+Mod, 140+Mod, 150+Mod, 170+Mod, 180+Mod, 180+Mod, 180+Mod, 170+Mod, 150+Mod, 140+Mod, 120+Mod}; */

//********** DATENÜBERTRAGUNG **********//
    /*  Heartbeat
     *   - Wird gesetzt, wenn der Master 'C0' die Anwesenheit abfragt
     */
    bool Heartbeat = false;


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
    uint8_t ValueSend = 0;      /* Merker für Daten-Senden */


    //*** Umrechnungsfunktion ***//

    int32_t changeRange(int32_t in_value, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
      if(in_value < in_min){in_value = in_min;}
      else if(in_value > in_max){in_value = in_max;}
      return (in_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }   
    



    //*******************************************************************************//
    //                             STEUER-BUS AUSLESEN                               //
    //*******************************************************************************//

    void ConvertRS485Data(uint8_t *Puffer) {
        switch (Puffer[0]) {
            case 'U':
            /* Nachricht: [U] [h] [*]
            *  U: Ultraschall - Initialisierung
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
                Mode[1] = Puffer[2];
            ModeNew = true;        
            break;
            case 'P': 
            /*  Nachricht: [P] [ | s] [...] [*]
            *   P: Parameter 
            *    [ | s]::
            *       
            *      s: Sensormodus
            *    [...]::
            *      Parameter-Werte
            */
                if (Puffer[1] == 's'){
                    SensorMode = Puffer[2]; 
                }else if (Puffer[1] == 'n'){
                
                }else if (Puffer[1] == 'o'){
                
                }
                else if (Puffer[1] == 'b'){ //LED Helligkeit
                    LEDBrighnesss = changeRange(Puffer[2], 0, 100, 0, 35);
                    NewBrightness = true;
                    //Serial.print("New LED Brighness");Serial.print("\t");Serial.println(LEDBrighnesss);
                }
            break;
            case 'S':
                if (Puffer[1] == 's') {
                    // Anfrage neuer Daten
                    ValueSend = Puffer[2]; // 0 oder 1
                    DriveValue[1] = Puffer[3];
                    DriveDirection = Puffer[4];
                    //Serial.print("ValueSend\t"); Serial.println(ValueSend);

                    if (DriveValue[1] != DriveValueOld[1])
                    {//Hat sich der Lenkwinkel geändert?
                        DriveValueNew = true;
                        DriveValueOld[1] = DriveValue[1];
                    }
                    if (DriveDirection != DriveDirectionOld)
                    {//Hat sich die Fahrtrichtung geändert?
                        DriveDirectionChange = true;
                        DriveDirectionOld = DriveDirection;
                    }
                }
            break;
            default:  break;
        }
    }

};// ENDE Klasse
#endif




