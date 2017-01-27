#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"
#include "ofxGui.h"
#include "Ingr.h"
#include "Fps.h"
#ifdef __arm__
    #include "RPiVideoGrabber.h"
    #include "ofxGPIO.h"
    #include "MPUController.h"
#endif

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
    ofVideoPlayer player;
    
    // number of pixels in each frame
    int nPixels;

    
    // ------------------------
    // ------- OP FLOW --------
    // ------------------------
    
    ofxCv::FlowFarneback flow;
    
    ofPixels thisFrame;
    
    void calcAverageDirections();
    
    void updateBodyRadius(int &radius);
    bool *avgMask;
    
    int nAvgValues;
    
    void drawBody(int x, int y, int w, int h);
    
    void calcStatistics(float *values, float *weights, int nValues, float &mean, float &variance);
    
    // ------------------------
    // ------- HISTOGRAM ------
    // ------------------------
    
    float *bucketsA; // [0, 360]
    float *bucketsB; // [-180, 180]
    float *repAngleA; // angles that represent the histogram buckets
    float *repAngleB;
    int bucketSize = 5; // should fit into 90 and 360 perfectly: 2, 3, 5, 6, 9, 10, 15
    int nBuckets;
    void drawHistogram(float *_buckets, int _nBuckets, int x, int y, int w, int h);
    float maxSummedHeight;
    

    
    // ------------------------
    // --------- IMU ----------
    // ------------------------
    
#ifdef __arm__
    MPUController mpu;
#endif
    
    void updateMPU();
    
    void removeTiltEffects();
    
    // ------------------------
    // ------ INGREDIENTS -----
    // ------------------------
    
    Ingr var;
    Ingr dir;
    Ingr act;
    Ingr mix;
    Ingr mpuX;  // pitch -- Y change
    Ingr mpuY;  // roll -- X change
    Ingr dir2;
    Ingr mix2;
    Ingr ext; // extremeties of angles
    Ingr mix3;
    
    void drawPredictedMovement(int x, int y, int size);
    
    
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
    ofParameter<float> angleMask; // remove all measurements beyond this angle in degrees from the mean to recalculate the mean and standard deviation

    ofParameterGroup ingrParams;            // only set at startup
    ofParameter<int> nIngrValues;
    ofParameter<bool> bStoreIngrHistory;
    
    ofParameterGroup imuParams;
    ofParameter<bool> bFlipX;
    ofParameter<bool> bFlipY;
    
    ofParameterGroup renderingParams;
    ofParameter<int> windowW;
    ofParameter<int> windowH;
    ofParameter<bool> bNormalizeHistogram;  // for visualization
    ofParameter<float> histogramScale;      // for a normalized histogram
    ofParameter<int> ingrW;
    ofParameter<int> ingrH;
    ofParameter<float> videoScale;
    ofParameter<bool> bDrawAll;
    ofParameter<bool> bDrawNone;            // must be false to draw anything
    ofParameter<bool> bDrawGui;
    ofParameter<bool> bDrawFPS;
    ofParameter<bool> bDrawMPU;
    ofParameter<bool> bDrawVideo;
    ofParameter<bool> bDrawArrow;
    ofParameter<bool> bDrawCV;
    ofParameter<bool> bDrawFlow;
    ofParameter<bool> bDrawHistogram;
    void updateDrawingSettings();
    
    ofParameterGroup outputParams;            // output to terminal
    ofParameter<bool> bOutputAscii;
    ofParameter<int> stride;
    ofParameter<float> stretch;
    ofParameter<bool> bOutputFPS;
    ofParameter<bool> bOutputDirection;
    ofParameter<bool> bOutputMPU;
    
    
    
    // ------------------------
    // --------- UTILS --------
    // ------------------------
    
    unsigned long prevFrame = 99999999;
    
    float map(float param, float minIn, float maxIn, float minOut, float maxOut, bool bClamp);
    
    
    Fps videoFPS;
    
    void drawAscii(ofPixels &pixels, int w, int h, int _stride, float _stretch);
    string asciiChar;
    
    // direction between 0 and 360
    // scale between 0 and 1
    // size must be odd
    void drawAsciiDirection(int size, float direction, float scale);
    
    
};
