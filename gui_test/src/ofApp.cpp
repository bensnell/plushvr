#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    settings.setName("Sets");
    settings.add(value.set("value", 1, 0, 10));
    
    panel.setup();
    panel.add(settings);
    
    value.addListener(this, &ofApp::updateValue);
    
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    
    panel.draw();

}

//--------------------------------------------------------------
void ofApp::updateValue(float &_value) {
    
    cout << "New value passed: " << _value << "\t\tParameter value: " << value << endl;
    
    cout << "on this frame: " << ofGetFrameNum() << endl;
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
