#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "Ingr.h"

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
    
    bool bLive = false;
    
    ofVideoGrabber grabber;
    
    ofVideoPlayer player;
    string videoName = "tracking_test_1920x1080_30fps.mp4";
    
    int imgW = 256;
    int imgH = 144;
    int nPixels;
    
    // ------------------------
    // ------- OP FLOW --------
    // ------------------------
    
    ofxCv::FlowFarneback flow;
    
    float flowFraction = 0.2;
    
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
    // ------ INGREDIENTS -----
    // ------------------------
    
    Ingr var;
    Ingr dir;
    Ingr act;
    Ingr mix;
    
    void drawPredictedMovement(int x, int y, int size);
    
    
    // ------------------------
    // ------- SETTINGS -------
    // ------------------------
    
    ofxPanel panel;
    string settingsFilename = "settings.xml";
    
    ofParameterGroup general;
    ofParameter<bool> bDrawImage;
    ofParameter<bool> bDrawFlow;
    ofParameter<int> bodyRadius;
    ofParameter<float> maxMagnitude;
    ofParameter<bool> bNormalizeHistogram;
    ofParameter<float> histogramScale; // for a normalized histogram
    ofParameter<int> nStats; // # stats kept in graph
    ofParameter<float> angleMask; // remove all measurements beyond this angle in degrees from the mean to recalculate the mean and standard deviation
    
    
    // ------------------------
    // --------- UTILS --------
    // ------------------------
    
    unsigned long prevFrame = 99999999;
    
    float map(float param, float minIn, float maxIn, float minOut, float maxOut, bool bClamp);
    
};
