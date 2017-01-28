#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    mix.setup("Mix", 500, 0, 10, true);
    
    panel.setup();
    panel.setName("Settings");
    panel.add(mix.params);
    panel.loadFromFile("settings.xml");
}

//--------------------------------------------------------------
void ofApp::update(){
    
    mix.addRaw(30 * ofNoise(ofGetElapsedTimeMillis()/1000.) + (floor(ofRandom(2))-1) * 2);
    
    mix.difference();
    
    mix.normalize();
    
    mix.taste();
    

    
    mix.average();
    
    mix.sensitize();
    
    mix.doneCooking();

}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(0);
    
    mix.draw(0, 0, 500, 200, {}, false);
    
    panel.setPosition(ofGetWidth() - panel.getWidth() - 10, 10);
    panel.draw();

}

void ofApp::exit() {
    
    panel.saveToFile("settings.xml");
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
