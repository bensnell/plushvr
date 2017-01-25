//
//  MPUController.h
//  mpu6050_magwick_2
//
//  Created by Ben Snell on 1/24/17.
//
//

#ifdef __arm__

#ifndef __mpu6050_magwick_2__MPUController__
#define __mpu6050_magwick_2__MPUController__

#include "ofMain.h"
#include "ofxGPIO.h"

#define MPU 0x68

class MPUController {
    
public:
    
    MPUController();
    
    // comms bus
    I2CBus * bus;
    
    // setup comms
    void setup();
    
    // update sensor values
    void update();
    
    // Raw values
    float ax, ay, az;       // Stores the real accel value in g's
    float gx, gy, gz;       // Stores the real gyro value in degrees per seconds
    string temperature;
    
    // resolution
    float aRes = 2.0/32768.0;
    float gRes = 250.0/32768.0;
    
    // really good sensor fusion algorithm
    void MadgwickQuaternionUpdate(float ax, float ay, float az, float gx, float gy, float gz);
    
    float GyroMeasError = PI * (40.0f / 180.0f);     // gyroscope measurement error in rads/s (start at 60 deg/s), then reduce after ~10 s to 3
    float beta = sqrt(3.0f / 4.0f) * GyroMeasError;  // compute beta
    float GyroMeasDrift = PI * (2.0f / 180.0f);      // gyroscope measurement drift in rad/s/s (start at 0.0 deg/s/s)
    float zeta = sqrt(3.0f / 4.0f) * GyroMeasDrift;  // compute zeta, the other free parameter in the Madgwick scheme usually set to a small or zero value
    float deltat = 0.0f;                              // integration interval for both filter schemes
    uint32_t lastUpdate = 0, firstUpdate = 0;         // used to calculate integration interval
    uint32_t Now = 0;                                 // used to calculate integration interval
    float q[4] = {1.0f, 0.0f, 0.0f, 0.0f};            // vector to hold quaternion
    
    // final values
    float pitch, yaw, roll;
    
    float getPitch() { return pitch };
    float getYaw() { return yaw };
    float getRoll() { return roll };
    
    // get string value to debug
    string getStringValues(int precision = 0);
    
};



#endif /* defined(__mpu6050_magwick_2__MPUController__) */

#endif