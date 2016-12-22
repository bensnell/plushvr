#pragma once

#include "ofMain.h"
#include "Fps.h"
#include "ofxGui.h"

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
    
    ofVideoGrabber grabber;
    
    string asciiCharacters;
    
    ofParameter<int> deviceID;
    
    ofParameter<int> stride;
    ofParameter<float> stretch;
    
    ofParameter<int> camWidth;
    ofParameter<int> camHeight;
    
    Fps videoFPS;
    
    ofParameterGroup general;
    ofxPanel panel;
    string fileName = "settings.xml";
    
    ofParameter<int> desiredFPS;
    
    

};
