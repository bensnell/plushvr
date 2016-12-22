#pragma once

#include "ofMain.h"
#include "Fps.h"

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
    
    ofVideoGrabber grabber;
    
    string asciiCharacters;
    
    int deviceID = 1;
    
    int stride = 5;
    float stretch = 2;
    
    int camWidth = 300;
    int camHeight = 200;
    
    Fps videoFPS;
    
    

};
