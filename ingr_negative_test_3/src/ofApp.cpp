#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    mix.setup("Mix", 500, 0, 2, true);
    
    panel.setup();
    panel.setName("Settings");
    panel.add(mix.params);
    panel.loadFromFile("settings.xml");
    
    ofSetFrameRate(10);
}

//--------------------------------------------------------------
void ofApp::update(){
    
    
    if (!bPause) {
        
        float thisVal = (0.9 * cos(ofGetElapsedTimef()) + ofRandom(-.1, .1)) * ((ofRandom(3) < 1.5) ? ofRandom(0.05, 0.3) : 1);
//        cout << thisVal << ", ";
        mix.addRaw(thisVal);

//        mix.addRaw(ofGetFrameNum()%2);
        
//        mix.addRaw(values[counter]);
//        counter++;
//        if (counter == values.size()) bPause = true;
        
//    mix.addRaw(30 * ofNoise(ofGetElapsedTimeMillis()/1000.) + (floor(ofRandom(2))-1) * 2);
        
        
        mix.normalize();
        
        mix.taste();
        

        
        mix.average();
        
        mix.taste();
        
        mix.prior();
        
        mix.doneCooking();
    }

}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(0);
    
    ofPushMatrix();
    ofScale(10, 1);
    mix.draw(0, 0, 200, 200, {}, false);
    ofPopMatrix();
    
    panel.setPosition(ofGetWidth() - panel.getWidth() - 10, 10);
    panel.draw();

}

void ofApp::exit() {
    
    panel.saveToFile("settings.xml");
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == ' ') bPause = !bPause;

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
