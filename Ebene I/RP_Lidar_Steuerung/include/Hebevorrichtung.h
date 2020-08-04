//*******************************************************************************//
//                          STEUERUNG DER HEBEVORRICHTUNG                        //
//*******************************************************************************//
//  Autoren:    Till Schneider, Lars Gebbeken, Lars Bartholomäus, Fabio Dörries  //
// 
//  Funktion:   Steuerung der Hebevorrichtung
//
//*******************************************************************************//

#ifndef Hebevorrichtung_h
#define Hebevorrichtung_h

#include <Arduino.h>
#include <Servo.h>

//Deklaration Servoobjekt Linearaktuator

Servo Linearmotor;


class Hebevorrichtung{

    public:
    //***** Pin deklarationen *****//
        uint8_t ButtonPin            = 12; // Taster
        uint8_t MotorPin1            = 22; // Motordrehrichtung - Lidar oben
        uint8_t MotorPin2            = 23; // Motordrehrichtung - Deckel oben
        uint8_t SchlittenPositionPin = 7; // Endlagenschalter Lidar oben
        uint8_t LidarPositionPin     = 6; // Endlagenschalter Schlitten
        uint8_t DeckelPositionPin    = 5; // Endlagenschalter Deckel oben
        

    //***** Variablen deklaration *****//

        uint16_t verfahrzeit = 2000;
        bool Schlittenposition = false;

        /*  Initialisierung
         *  
         *  Eingabe - Pins
         *      -   Button:              Taster
         *      -   Motor1:              Motordrehrichtung - Lidar oben
         *      -   Motor2:              Motordrehrichtung - Deckel oben
         *      -   LidarPosition:       Endlagenschalter Lidar oben
         *      -   DeckelPosition:      Endlagenschalter Deckel oben
         *      -   SchlittenPosition:   Endlagenschalter Schlitten
         */     

        void init(){
            Linearmotor.attach(8);
            pinMode(ButtonPin, INPUT_PULLUP);
            pinMode(MotorPin1, OUTPUT);
            pinMode(MotorPin2, OUTPUT);
            pinMode(LidarPositionPin, INPUT_PULLUP);
            pinMode(DeckelPositionPin, INPUT_PULLUP);
            pinMode(SchlittenPositionPin, INPUT_PULLUP);
        }

        //************** Linearmotor Steuerung *******************//

        void SchlittenVerfahren(char dir){
            switch(dir){
                case 'h': // Schlitten hochfahren/ausfahren
                    Linearmotor.writeMicroseconds(2000); // Positionen zwichen 1000(eingefahren) und 2000(ausgefahren) möglich durch PWM
                    delay(verfahrzeit);                     // Wartezeit 3 Sekunden, bis Schlitten hochgefahren
                    Schlittenposition = true;
                break;
                case 'r': // Schlitten runterfahren/einfahren
                    Linearmotor.writeMicroseconds(1000);                 // Positionen zwichen 1000(eingefahren) und 2000(ausgefahren) möglich durch PWM
                    while (digitalRead(SchlittenPositionPin) == 1){}    //Warten bis Korb unten LOW -> gedrückt
                    Schlittenposition = false;
                break;
            }
        }



        //************** Motor Steuerung *******************//


        void LidarDrehen(char dir){
            switch(dir){
                case 'l': // Lidar wird nach oben gedreht
                    while (digitalRead(LidarPositionPin) == HIGH){
                        digitalWrite(MotorPin1, HIGH); 
                        digitalWrite(MotorPin2, LOW);
                    }
                    delay(15);
                    //**** Motor stoppen
                    digitalWrite(MotorPin1, LOW); 
                    digitalWrite(MotorPin2, LOW);
                break;

                case 'd': // Deckel wird nach oben gedreht
                    while (digitalRead(DeckelPositionPin) == HIGH){
                        digitalWrite(MotorPin1, LOW); 
                        digitalWrite(MotorPin2, HIGH); 
                    }
                    //**** Motor stoppen
                    digitalWrite(MotorPin1, LOW); 
                    digitalWrite(MotorPin2, LOW);
                break;
            }

        }

        /* Funktion: Starposition
         *      - Grundposition wird angesteuert
         *          - Deckel oben
         */
        void Starposition() { 
            if(digitalRead(DeckelPositionPin) || Schlittenposition == false){
                SchlittenVerfahren('r');
                LidarDrehen('d');
                SchlittenVerfahren('h');
            }
        }

        /* Funktion: 
         *
         */
        void Fahren(){
            if (Schlittenposition == true && digitalRead(ButtonPin) == LOW){   
                // Wenn der Schlitten in der Grundposition(ausgefahren) und der Button betätigt wird
                //  -> Dann wird verfahren
                
                Serial.print("Deckel \t"); Serial.println(digitalRead(DeckelPositionPin));
                Serial.print("Lidar \t"); Serial.println(digitalRead(LidarPositionPin));
                SchlittenVerfahren('r');

                if(digitalRead(DeckelPositionPin) == LOW && digitalRead(LidarPositionPin) == HIGH) {  
                    // Wenn der Deckel oben
                    Serial.println("Deckel oben");
                    LidarDrehen('l');
                }
                else if (digitalRead(DeckelPositionPin) == HIGH && digitalRead(LidarPositionPin) == LOW){
                    //Wenn der Lidar oben
                    Serial.println("Lidar oben");
                    LidarDrehen('d');
                }
                else if(digitalRead(DeckelPositionPin) == HIGH && digitalRead(LidarPositionPin) == HIGH){
                    Serial.println("Nichts oben");
                    LidarDrehen('d');
                }

                SchlittenVerfahren('h');
            }
            
        }
};

#endif