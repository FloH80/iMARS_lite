//*******************************************************************************//
//                                LENKUNGSSTEUERUNG                              //
//*******************************************************************************//

//*******************************************************************************//

#ifndef Steer_h
#define Steer_h

//* Bibliotheken *//
#include "DynamixelShield.h"

DynamixelShield Steering;

class SteerbyWire {
  private:

    const uint8_t DXL_ID = 0;
    const float DXL_PROTOCOL_VERSION = 1.0;

    int32_t changeRange(int32_t in_value, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
        if(in_value < in_min){in_value = in_min;}
        else if(in_value > in_max){in_value = in_max;}
        return (in_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }
    
  public:

    uint16_t offsetAngle = 186;
  
    void init(){
        Steering.begin(115200);
        Steering.setPortProtocolVersion(1.0);
        Steering.ping(DXL_ID);
        Steering.torqueOff(DXL_ID);
        Steering.setOperatingMode(DXL_ID, OP_POSITION);
        Steering.setGoalVelocity(DXL_ID, 25, UNIT_PERCENT);
        Steering.setGoalPosition(DXL_ID, offsetAngle, UNIT_DEGREE);
        Steering.torqueOn(DXL_ID);
    }

    void setVelocity(uint8_t Velocity){
        Steering.setGoalVelocity(DXL_ID, Velocity, UNIT_PERCENT);
    }

    void setAngle(uint16_t angle){
        /* Winkelberechnung
         * offset + 
         */
        angle = offsetAngle + angle - 45;
        System.Error[1] = Steering.setGoalPosition(DXL_ID, angle, UNIT_DEGREE);
    }

};
#endif
