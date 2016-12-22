#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    // setup the gui
    general.setName("General");
    general.add(camWidth.set("Cam Width", 320, 1, 2000));
    general.add(camHeight.set("Cam Height", 240, 1, 2000));
    general.add(stride.set("Stride", 5, 1, 100));
    general.add(stretch.set("Stretch", 2, 0.0001, 10));
    general.add(desiredFPS.set("Desired FPS", 30, 1, 120));
    
    panel.setup();
    panel.add(general);
    panel.loadFromFile(fileName);
    
    // setup the video grabber
    grabber.setDesiredFrameRate(desiredFPS);
    grabber.initGrabber(camWidth, camHeight);
    
    asciiCharacters =  string("  ..,,,'''``--_:;^^**""=+<>iv%&xclrs)/){}I?!][1taeo7zjLunT#@JCwfy325Fp6mqSghVd4EgXPGZbYkOA8U$KHDBWNMR0Q");

    
}

//--------------------------------------------------------------
void ofApp::update(){

    grabber.update();
    if (grabber.isFrameNew()) {
        // convert it to ascii values
        
        videoFPS.addFrame();
        
        ofPixels &pixels = grabber.getPixels();
        
        stringstream ss;
        
        // adapted from OF ascii video example
        for (int row = 0; row < camHeight; row += stride*stretch){

            ss << "\n";
            for (int col = 0; col < camWidth; col += stride){
                
                // get the pixel and its lightness (lightness is the average of its RGB values)
                float lightness = pixels.getColor(col,row).getLightness();
                
                // calculate the index of the character from our asciiCharacters array
                int character = powf( ofMap(lightness, 0, 255, 0, 1), 2.5) * asciiCharacters.size();
                
                // draw the character at the correct location
                ss << ofToString(asciiCharacters[character]);
            }
        }
        
        // draw it to the terminal
        cout << ss.str() << endl;
        videoFPS.update();
        cout << "\nFPS " << ofToString(videoFPS.getFPS()) << endl;
    }
    
    videoFPS.update();

}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::exit() {
    
    panel.saveToFile(fileName);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
