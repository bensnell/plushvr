#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "Ingr.h"
//#include "RPiVideoGrabber.h"
#include "Fps.h"

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
    
    RPiVideoGrabber grabber;
//    ofVideoGrabber grabber;
    
    ofVideoPlayer player;
    
    int nPixels;
    
    ofPixels pix;

    
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
    ofParameter<int> desiredFPS;
    ofParameter<bool> bLive;
    ofParameter<string> videoName;
    ofParameter<bool> bDrawAll;
    ofParameter<bool> bOutputVideo;
    ofParameter<bool> bOutputDirection;
    ofParameter<bool> bOutputFPS;
//    ofParameter<bool> bNormalizeImage; // could try this
    ofParameter<int> stride;
    ofParameter<float> stretch;
    ofParameter<int> imgW;
    ofParameter<int> imgH;
    
    ofParameterGroup flowParams;
    ofParameter<float> pyramidScale;
    ofParameter<int> numLevels;
    ofParameter<int> windowSize;
    ofParameter<int> numIterations;
    ofParameter<int> polyN;
    ofParameter<float> polySigma;
    ofParameter<bool> useGaussian;
    
    
    
    // ------------------------
    // --------- UTILS --------
    // ------------------------
    
    unsigned long prevFrame = 99999999;
    
    float map(float param, float minIn, float maxIn, float minOut, float maxOut, bool bClamp);
    
    string asciiChar;
    
    Fps videoFPS;
    
    void drawAscii(ofPixels &pixels, int w, int h, int _stride, float _stretch);
    
    // direction between 0 and 360
    // scale between 0 and 1
    // size must be odd
    void drawAsciiDirection(int size, float direction, float scale);
    
};
