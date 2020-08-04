//*******************************************************************************//
//                                ANTRIEBSSTEUERUNG                              //
//*******************************************************************************//

//*******************************************************************************//

#ifndef Drive_h
#define Drive_h

//* Bibliotheken *//
#include "Servo.h" 

/* Festlegen der Servo-Klasse */
Servo DriveController; /* Ansteuerung des Antriebs 
                        *  - Funktion writeMicroseconds
                        */

class Drive {
  private:

  int32_t changeRange(int32_t in_value, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    if(in_value < in_min){in_value = in_min;}
    else if(in_value > in_max){in_value = in_max;}
    return (in_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }
    
  public:
  uint8_t PWM_Pin = 2;
  uint8_t VelocityMax[2];

  Drive(uint8_t Pin){
    PWM_Pin = Pin;
  }
  
  void init(){
    DriveController.attach(PWM_Pin);
    VelocityMax[0] = 255;
    VelocityMax[1] = 0;
  }

    void setVelocity(uint8_t VelocityMaximalForward, uint8_t VelocityMaximalBackward){
      VelocityMax[0] = changeRange(VelocityMaximalForward, 0 ,100, 128, 255);
      VelocityMax[1] = changeRange(VelocityMaximalBackward, 0 , 100, 128, 0);
    }

  void writeValue(uint8_t velocity){
    if(velocity == 128){
      /* Halte Anweisung bei 128
       * - unabghängig von maxima
       */
      DriveController.writeMicroseconds(1500);
    }else if(velocity > 128){
      velocity = min(velocity,VelocityMax[0]);
      uint16_t forward = changeRange(velocity, 128, 255, 1500, 2000);
      DriveController.writeMicroseconds(forward);
    }else if(velocity < 128){
      velocity = max(velocity,VelocityMax[1]);
      uint16_t backward = changeRange(velocity, 0, 128, 1000, 1500);
      DriveController.writeMicroseconds(backward);
    }
  }

};
#endif
