#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"
#include "ofxGui.h"
#include "Ingr.h"
#include "Fps.h"
#ifdef __arm__ // can't include ofxGPIO here again because it's in MPUController.h
    #include "RPiVideoGrabber.h"
    #include "MPUController.h"
#endif
#include "ofxOsc.h"

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

// This app is being built with the rpi in mind (i.e. it can run on both mac and rpi, but the settings are most easily accessed by updating settings.xml, minimal info is draw to screen, etc.)

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void exit();

		void keyPressed(int key);
    
    // ------------------------
    // ------- CONTENT --------
    // ------------------------
    
    // Video grabber for live video
#ifdef __arm__
    RPiVideoGrabber grabber;
#else
    ofVideoGrabber grabber;
#endif
    
    // Video player for recorded video
    ofVideoPlayer player; // remove this
    
    // number of pixels in each frame
    int nPixels;

    
    // ------------------------
    // ------- OP FLOW --------
    // ------------------------
    
    ofxCv::FlowFarneback flow;
    
    bool bPreviousFrame = false; // removes nan and inf from optical flow
    
    ofPixels thisFrame;
    
    void updateBodyRadius(int &radius);
    bool *avgMask;
    
    int nAvgValues;
    
    void drawBody(int x, int y, int w, int h);
    
    // ------------------------
    // --------- IMU ----------
    // ------------------------
    
#ifdef __arm__
    MPUController mpu;
#endif
    
    void updateMPU();
    
    // ------------------------
    // ------ INGREDIENTS -----
    // ------------------------
    
    void efficientCalc();
    
    // confidence in magnitude
    Ingr activity;
    Ingr linearity;
    Ingr reliability;
    Ingr mixture1;
    
    // confidence in direction
    Ingr xComponent;
    Ingr yComponent;
    Ingr xStability;    // prevents left and right tilt -- ROLL
    Ingr yStability;    // prevents front and back tilt -- PITCH
    Ingr xStabilityPost;// second round of processing on the data
    Ingr yStabilityPost;
    // combined confidence
    Ingr mixture2;      // x component
    Ingr mixture3;      // y component
    
    // final movement force
    ofVec2f movementForce;
    
    void drawPredictedMovement(int x, int y, int size);
    
    // ------------------------
    // -------- COMMS ---------
    // ------------------------
    
    ofxOscSender sender;
    
    
    // ------------------------
    // ------- SETTINGS -------
    // ------------------------
    
    vector<ofxPanel> panels;
    int selPanel = 0;
    
    string genPanelFilename = "genSettings.xml";
    string ingrPanelFilename = "ingrSettings.xml";
    
    ofParameterGroup generalParams;
    ofParameter<int> imgW;                  // 256      192     128
    ofParameter<int> imgH;                  // 144      108     72
    ofParameter<bool> bLive;
    ofParameter<bool> bLivePause;
    ofParameter<string> videoName;
    ofParameter<int> desiredFPS;            // of video itself
    
    // may not be necessary -- may significantly reduce fps
    ofParameterGroup cvParams;              // cv before calc opflow
    ofParameter<bool> bNormalize;
    ofParameter<bool> bHistEqualize;
    
    // if polyN = 7, then polySigma = 1.5
    // if polyN = 5, then polySigma = 1.1
    ofParameterGroup flowParams;
    ofParameter<bool> bDoFlow;
    ofParameter<float> pyramidScale;
    ofParameter<int> numLevels;             // 3
    ofParameter<int> windowSize;            // 32
    ofParameter<int> numIterations;         // 1
    ofParameter<int> polyN;                 // 3
    ofParameter<float> polySigma;           // 1.1 ???
    ofParameter<bool> useGaussian;
    ofParameter<bool> bRefreshFlowValues;   // toggle this to update all settings
    void refreshFlowParams();
    
    ofParameterGroup statisticsParams;      // for finding variance and mean
    ofParameter<int> bodyRadius;
    ofParameter<float> maxMagnitude;        // of the flow rays
    ofParameter<int> nSkipPixels; // how many pixels to skip when sampling them from the optical flow

    ofParameterGroup ingrParams;            // only set at startup
    ofParameter<int> nIngrValues;
    ofParameter<bool> bStoreIngrHistory;
    ofParameter<float> outputMovementThreshold; // threshold when movements begin to be output (for example, when movement [0,1] is above 0.1)
    ofParameter<bool> bSkipSmallReadings;   
 
    ofParameterGroup imuParams;
//    ofParameter<bool> bFlipX;
//    ofParameter<bool> bFlipY;
    
    ofParameterGroup renderingParams;
    ofParameter<int> windowW;
    ofParameter<int> windowH;
    ofParameter<int> ingrW;
    ofParameter<int> ingrH;
    ofParameter<float> videoScale;
    ofParameter<bool> bDrawAll;
    ofParameter<bool> bDrawNone;            // must be false to draw anything
    ofParameter<bool> bDrawGui;
    ofParameter<bool> bDrawFPS;
    ofParameter<bool> bDrawMPU;
    ofParameter<bool> bDrawForce;      // print the motion vectors
    ofParameter<bool> bDrawVideo;
    ofParameter<bool> bDrawArrow;
    ofParameter<bool> bDrawCV;
    ofParameter<bool> bDrawFlow;
    // These are only rendering parameters (they don't affect motionForce):
    ofParameter<bool> bDrawPosition;    // draw the integrated position
    ofParameter<float> positionScale;   // scale of each step
    ofParameter<int> nPositions;        // number of vertices in polyline
    ofParameter<int> positionSize;      // length and width of plot
    ofParameter<int> posRotation;       // rotate the impu yaw
    ofParameter<bool> posFlipX;         // flip vector in X
    ofParameter<bool> posFlipY;         // flip vector in Y
    ofParameter<bool> posFlipYaw;
    ofParameter<float> posStrokeWidth;
    void updateDrawingSettings();
    
    ofParameterGroup outputParams;            // output to terminal
    ofParameter<bool> bOutputAscii;
    ofParameter<int> stride;
    ofParameter<float> stretch;
    ofParameter<bool> bOutputFPS;
    ofParameter<bool> bOutputDirection;
    ofParameter<bool> bOutputMPU;
    ofParameter<bool> bOutputForce;
    
    ofParameterGroup oscParams;
    ofParameter<string> host;
    ofParameter<int> port;
    ofParameter<string> messageAddress; // name in the address field of each message
    ofParameter<bool> bSendOsc;
    
    // ------------------------
    // --------- UTILS --------
    // ------------------------
    
    unsigned long prevFrame = 99999999;
    
    float map(float param, float minIn, float maxIn, float minOut, float maxOut, bool bClamp);
    
    Fps videoFPS;
    Fps mpuFPS;
    
    void drawAscii(ofPixels &pixels, int w, int h, int _stride, float _stretch);
    string asciiChar;
    
    // direction between 0 and 360
    // scale between 0 and 1
    // size must be odd
    void drawAsciiDirection(int size, float direction, float scale);
    
    // Drawing the Position
    ofPolyline posLine;
    
    
};
