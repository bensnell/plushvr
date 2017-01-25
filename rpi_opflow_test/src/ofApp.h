#pragma once

#include "ofMain.h"
#include "Fps.h"
#include "ofxGui.h"
#ifdef __arm__
    #include "RPiVideoGrabber.h"
#endif
#include "ofxCv.h"
#include "ofxOpenCv.h"

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
    
#ifdef __arm__
    RPiVideoGrabber grabber;
#else
    ofVideoGrabber grabber;
#endif
    
    string asciiCharacters;
    
    ofParameter<int> stride;
    ofParameter<float> stretch;
    
    ofParameter<int> camWidth;
    ofParameter<int> camHeight;
    
    Fps videoFPS;
    
    ofParameterGroup general;
    ofxPanel panel;
    string fileName = "settings.xml";
    
    ofParameter<int> desiredFPS;
    
    ofxCv::FlowFarneback fb;
    
    ofParameterGroup flow;
    ofParameter<float> fbPyrScale, fbPolySigma;
    ofParameter<int> fbLevels, fbIterations, fbPolyN, fbWinSize;
    ofParameter<bool> fbUseGaussian, bDoFlow;
    
    ofParameterGroup rendering;
    ofParameter<int> windowW;
    ofParameter<int> windowH;
    ofParameter<bool> bDrawGui;
    ofParameter<bool> bDrawVideo;
    ofParameter<bool> bDrawFlow;
    ofParameter<bool> bDrawFPS;
    ofParameter<bool> bOutputAscii;
    ofParameter<bool> bOutputFPS;

};
