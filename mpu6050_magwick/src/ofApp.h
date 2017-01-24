#pragma once

#include "ofMain.h"
#include "ofxGPIO.h"

#define MPU 0x68

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    I2CBus * bus;

    string temperature;
    
    float ax, ay, az;       // Stores the real accel value in g's
    float gx, gy, gz;       // Stores the real gyro value in degrees per seconds
    float aRes = 2.0/32768.0;
    float gRes = 250.0/32768.0;
    
    void MadgwickQuaternionUpdate(float ax, float ay, float az, float gx, float gy, float gz);
    
    float GyroMeasError = PI * (40.0f / 180.0f);     // gyroscope measurement error in rads/s (start at 60 deg/s), then reduce after ~10 s to 3
    float beta = sqrt(3.0f / 4.0f) * GyroMeasError;  // compute beta
    float GyroMeasDrift = PI * (2.0f / 180.0f);      // gyroscope measurement drift in rad/s/s (start at 0.0 deg/s/s)
    float zeta = sqrt(3.0f / 4.0f) * GyroMeasDrift;  // compute zeta, the other free parameter in the Madgwick scheme usually set to a small or zero value
    float pitch, yaw, roll;
    float deltat = 0.0f;                              // integration interval for both filter schemes
    uint32_t lastUpdate = 0, firstUpdate = 0;         // used to calculate integration interval
    uint32_t Now = 0;                                 // used to calculate integration interval
    float q[4] = {1.0f, 0.0f, 0.0f, 0.0f};            // vector to hold quaternion
		
};
