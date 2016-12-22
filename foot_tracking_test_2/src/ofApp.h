#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "Avgr.h"

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
    
    bool bLive = false;
    
    ofVideoGrabber grabber;
		
    ofVideoPlayer player;
    
    string videoName = "tracking_test_1920x1080_15fps.mp4";
    
    int imgW = 256;
    int imgH = 144;
    int nPixels;
    
    ofxCv::FlowFarneback flow;
    
    float flowFraction = 0.2;
    
    ofPixels thisFrame;
    
    
    
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
    ofParameter<float> maxVarianceHeight; // height to normalize variance in graph
    ofParameter<float> meanSmoothingFactor;
    ofParameter<float> varianceSmoothingFactor;
    ofParameter<float> confThresh; // threshold by which values below are considered reliable indicators of movement forward
    ofParameter<float> confMult;
    ofParameter<float> confMax;
    ofParameter<float> confSmoothingFactor;
    ofParameter<float> confPower; // amount by which we separate reliable from unreliable readings (could instead just remove readings below 1 instead, instead of making them smaller
//    ofParameter<float> confTrigger; // value that will trigger movement
    ofParameter<float> angleMask; // remove all measurements beyond this angle in degrees from the mean to recalculate the mean and standard deviation
    ofParameter<float> activityPower; // power to sensitize activity level
    ofParameter<float> variancePower; // power to sensitize variance
    ofParameter<float> mixPower; // power to sensitize mix
    ofParameter<float> activityMax; // smaller than the cap on activity, maxMagnitude
    ofParameter<float> activityMin;
    ofParameter<float> varianceMin;
    ofParameter<float> mixMin;
    ofParameter<float> mixMax;
    
    
    void calcAverageDirections();
    
    void updateBodyRadius(int &radius);
    bool *avgMask;
    
    int nAvgValues;
    float activity;
    vector<float> activities;
    void drawPlot(vector<float> values, float max, int maxNumValues, int x, int y, int w, int h, ofColor color = ofColor(255));
    
    void drawBody(int x, int y, int w, int h);
    
    // histogram
    float *bucketsA; // [0, 360]
    float *bucketsB; // [-180, 180]
    float *repAngleA; // angles that represent the histogram buckets
    float *repAngleB;
    int bucketSize = 5; // should fit into 90 and 360 perfectly: 2, 3, 5, 6, 9, 10, 15
    int nBuckets;
    void drawHistogram(float *_buckets, int _nBuckets, int x, int y, int w, int h);
    float maxSummedHeight;
    
    void calcStatistics(float *values, float *weights, int nValues, float &mean, float &variance);
    
    
    float dirMean, dirVar; // direction mean and variance
    bool bNoPreviousData = true;
    float avgDirMean, avgDirVar; // average
    vector<float> means;
    vector<float> variances;
    vector<float> avgMeans;
    vector<float> avgVariances;
    
    void drawStatistics(vector<float> _means, vector<float> _variances, int x, int y, int w, int h, ofColor color = ofColor(255));
    
    unsigned long prevFrame = 99999999;
    
    float conf;
    vector<float> confs;
    bool bNoPreviousConf = true;
    float avgConf;
    vector<float> avgConfs;
    
    void drawPredictedMovement(int x, int y, int size);
    
    float map(float param, float minIn, float maxIn, float minOut, float maxOut, bool bClamp);
    
    Avgr activityIngr;
    Avgr varianceIngr;
    Avgr confAvgr;
    
    
};
