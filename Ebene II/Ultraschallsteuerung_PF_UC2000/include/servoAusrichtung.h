//*******************************************************************************//
//                        SERVOANSTEUERUNG DER US-SENSOREN                       //
//*******************************************************************************//
//
//  - Hier werden die Servos der Ultraschall Sensoren angesteuert 
//  - Desweiteren werden die Entfernungen mit zugehörigern Winkel, auf den Antrieb bezogen, umgerechnet
//
//*******************************************************************************//

#ifndef servoAusrichtung_h
#define servoAusrichtung_h

//#include "Arduino.h"
#include <Servo.h>


class servoAusrichtung
{
private:

    Servo servoR;  
    Servo servoL;
    Servo servoB;

    uint8_t servo_right_front_val = 20;
    uint8_t servo_left_front_val = 145;
    uint8_t servo_right_val, servo_left_val, servo_back_val;  // Sollpos. der Servos
    int8_t Angle_r, Angle_l, Angle_b;             // Ausrichtung aus der Fahrtrichtung -> Antrieb Lenkung 45° = 0°
    float Angle_bog_r, Angle_bog_l, Angle_bog_b; // Winkel in Bogenmaß im bezug auf Antriebsachse

    int32_t changeRange(int32_t, int32_t, int32_t, int32_t, int32_t);

public:

    void init(uint8_t,uint8_t,uint8_t); 
        // Initialisierung der Servos und und anfahren der Ausgangsposition //Pinbelegung: rechts, links, hinten

    void ausrichten();                    
        // richtet Servos in in die gegebene Fahrtrichtung aus

    void activeSensorSteering();
         //Lenkwinkel von Hauptsteuerung
    
    void umrechnen();               
        //übergibt die Entfernungen in in eine 3D Matrix {[y - in Fahrtrichtung] [x - horizontal zur Fahrtrichtung] [h - Höhe]}
        //Bezugspunkt: Achse vorderes Antreibsrad 

    servoAusrichtung(/* args */);
    ~servoAusrichtung();
};

void servoAusrichtung::init(uint8_t servoRPin, uint8_t servoLPin, uint8_t servoBPin){
    
    servoR.attach(servoRPin);
    delay(10);
    servoL.attach(servoLPin);
    delay(10);
    servoB.attach(servoBPin);
    delay(10);
    servoR.write(servo_right_front_val);           //5..35° mitlenken, 20°->Mitte(Vorne) | Hinten 145°
    servoL.write(servo_left_front_val);          //115...145° mitlenken, 130°->Mitte(Vorne) | Hinten 5°
    servoB.write(45);           // -----------???-------------------
};

void servoAusrichtung::ausrichten(){

    /* if (System.DriveDirection == 1)
    {//Rückwärtsfahrt
        //Servos nach hinten drehen
        servoR.write(165);
        servoL.write(15);
        trueAngle_r = 0;
        trueAngle_l = 0;
    } */
    if ( System.DriveDirection == 0)
    {//Vorwärtsfahrt
        //Servos nach vorne drehen
        servoR.write(servo_right_front_val);
        servoL.write(servo_left_front_val);
        servoB.write(45);
    }
    System.DriveDirectionChange = false;
}

void servoAusrichtung::activeSensorSteering(){
    if ((System.Mode[0] == 2 || System.Mode[0] == 1) /* && System.Mode[1] */)
    //Wenn automatikfahrmodus, oder manuelle ausgewählt wurde und dieser aktiviert ist
    {
        if (System.DriveDirectionChange || System.DriveValue[1] == 45)
        {//Bei einem Richtungswechel von der Fahrtrichtung -> 1s warten um US ausrichten zu können
            ausrichten();
        }

        if (System.DriveDirection == 0 && System.DriveValueNew == true) 
        {//bei Vorwärtsfahrt (DriveDirection = 0) und neuem Lenkwinkel
            
            if ((System.DriveValue[1] >= 50 || System.DriveValue[1] <= 40))
            {//Sobald der Lenkwinkel größer als 15° außerhalb der Mitte -> US vorne mitlenken
                servo_right_val = changeRange(System.DriveValue[1], 0, 90, 5, 35);
                servoR.write(servo_right_val);
                
                servo_left_val = changeRange(System.DriveValue[1], 0, 90, 130, 160);
                servoL.write(servo_left_val);
                
            }
            System.DriveValueNew = false;
        }

        else if (System.DriveDirection == 1 && System.DriveValueNew == true) 
        {//bei Rückwärtsfahrt (DriveDirection = 1) und neuem Lenkwinkel
         if ((System.DriveValue[1] >= 55 || System.DriveValue[1] <= 35))
            {//Sobald der Lenkwinkel größer als 10° außerhalb der Mitte  -> US hinten mitlenken
                servo_back_val = changeRange(System.DriveValue[1], 0, 90, 50, 120);
                servoB.write(servo_back_val);
            }
            System.DriveValueNew = false;
        }
    }
}

void servoAusrichtung::umrechnen(){    
    
    if(System.DriveDirection == 0/* Fahrtrichtung */)
    {/* Vorwertsfahrt*/
        Angle_r = servo_right_val - servo_right_front_val;
        Angle_l = servo_left_val - servo_left_front_val;
        Angle_bog_r = (90 - abs(Angle_r)) * (PI / 180);
        Angle_bog_l = (90 - abs(Angle_l)) * (PI / 180);

        if (System.DriveValue[1] > 45){
            //Rechtsfahrt
            System.Distance3DArray[0][0] =  { 15 + (cos(Angle_bog_r)) * System.FrontEcho[0] };
            System.Distance3DArray[1][0] =  { 15 - (cos(Angle_bog_l)) * System.FrontEcho[1]};
        }
        if (System.DriveValue[1] < 45){
            //Linksfahrt
            System.Distance3DArray[0][0] =   { 15 - (cos(Angle_bog_r)) * System.FrontEcho[0] };
            System.Distance3DArray[1][0] =  { 15 + (cos(Angle_bog_l)) * System.FrontEcho[1] };
        }

        System.Distance3DArray[0][1] =   { (sin(Angle_bog_r)) * System.FrontEcho[0] }; 
        System.Distance3DArray[0][2] =   { 60 - (sin(5 * (PI / 180))) * System.FrontEcho[0] };
        System.Distance3DArray[1][1] =   { (sin(Angle_bog_l)) * System.FrontEcho[1] }; 
        System.Distance3DArray[1][2] =   { 60 - (sin(5 * (PI / 180))) * System.FrontEcho[1] };

        if (System.DriveValue[1] == 45){
            //Geradeausfahrt
            System.Distance3DArray[0][0] = {-15};
            System.Distance3DArray[0][1] = {System.FrontEcho[0]};
            System.Distance3DArray[0][2] = {60 - (sin(5 * (PI / 180))) * System.FrontEcho[0]};

            System.Distance3DArray[1][0] = {15};
            System.Distance3DArray[1][1] = {System.FrontEcho[1]};
            System.Distance3DArray[1][2] = {60 - (sin(5 * (PI / 180))) * System.FrontEcho[1]};
        }

        if (System.Distance3DArray[0][0] <= 0){  //rechter US
            //Gegenstand rechts in Fahrtrichtung
            System.USnormAngel[0] = 90 - atan((System.Distance3DArray[0][1]/abs(System.Distance3DArray[0][0]))*(PI / 180))*(180 / PI);
        }
        if (System.Distance3DArray[0][0] > 0){
            //Gegenstand links in Fahrtrichtung
            System.USnormAngel[0] =   atan((System.Distance3DArray[0][1]/System.Distance3DArray[0][0])*(PI / 180))*(180 / PI);
        }

        if (System.Distance3DArray[1][0] <= 0){  //linker US
            //Gegenstand rechts in Fahrtrichtung
            System.USnormAngel[1] =  atan((System.Distance3DArray[1][1]/abs(System.Distance3DArray[1][0]))*(PI / 180))*(180 / PI);
        }
        if (System.Distance3DArray[1][0] > 0){
            //Gegenstand links in Fahrtrichtung
            System.USnormAngel[1] = 90 - atan((System.Distance3DArray[1][1]/System.Distance3DArray[1][0])*(PI / 180))*(180 / PI);
        }

        System.USnormDistance[0] = System.Distance3DArray[0][1] / sin(System.USnormAngel[0] * (PI / 180));
        System.USnormDistance[1] = System.Distance3DArray[1][1] / sin(System.USnormAngel[1] * (PI / 180));

        if (System.USnormDistance[0] < System.USnormDistance[1]){
            //Kritische normierte Entfernung
            System.USnormDistanceKrit = System.USnormDistance[0];
            System.USnormAngelKrit =  System.USnormAngel[0];
        }else{
            System.USnormDistanceKrit = System.USnormDistance[1];
            System.USnormAngelKrit =  System.USnormAngel[1];
        }
        

        //Serial.print(System.FrontEcho[0]);Serial.print("\t");Serial.println(System.FrontEcho[1]);

        Serial.print(System.USnormDistance[0]);Serial.print("\t");Serial.print(System.USnormAngel[0]);Serial.print("\t");Serial.print(System.FrontEcho[0]);Serial.print("\t");
        Serial.print(System.USnormDistance[1]);Serial.print("\t");Serial.println(System.USnormAngel[1]);

        /* for(int i = 0; i<2; i++){
            for(int u = 0; u < 3; u++){
                Serial.print(System.Distance3DArray[i][u]);Serial.print("\t");
            }
            Serial.println();
        } */

    }else if(System.DriveDirection == 1){
        /* Rueckwertsfahrt*/
    }
    
}

int32_t servoAusrichtung::changeRange(int32_t in_value, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
  if (in_value < in_min) {
    in_value = in_min;
  } else if (in_value > in_max) {
    in_value = in_max;
  }
  return (in_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

servoAusrichtung::servoAusrichtung(/* args */)
{
}

servoAusrichtung::~servoAusrichtung()
{
}




#endif