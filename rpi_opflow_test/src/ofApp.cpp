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
    
    flow.setName("Optical Flow");
    flow.add(bDoFlow.set("Do Optical Flow", true));
    flow.add(fbPyrScale.set("fbPyrScale", .5, 0, .99));
    flow.add(fbLevels.set("fbLevels", 4, 1, 8));
    flow.add(fbIterations.set("fbIterations", 2, 1, 8));
    flow.add(fbPolyN.set("fbPolyN", 7, 5, 10));
    flow.add(fbPolySigma.set("fbPolySigma", 1.5, 1.1, 2));
    flow.add(fbUseGaussian.set("fbUseGaussian", false));
    flow.add(fbWinSize.set("winSize", 32, 4, 64));
    
    rendering.setName("Rendering");
    rendering.add(windowW.set("Window Width", 1024, 1, 2000));
    rendering.add(windowH.set("Window Height", 768, 1, 2000));
    rendering.add(bDrawGui.set("Draw Gui", true));
    rendering.add(bDrawVideo.set("Draw Video", false));
    rendering.add(bDrawFlow.set("Draw Flow", false));
    rendering.add(bDrawFPS.set("Draw FPS", false));
    rendering.add(bOutputAscii.set("Output Ascii Video", false));
    rendering.add(bOutputFPS.set("Output FPS", true));
    
    panel.setup();
    panel.add(general);
    panel.add(flow);
    panel.add(rendering);
    panel.loadFromFile(fileName);
    
    // set the shape of the window
    ofSetWindowShape(windowW, windowH);
    
    // setup the video grabber
#ifndef __arm__
    grabber.setDeviceID(1);
#endif
    grabber.setDesiredFrameRate(desiredFPS);
    grabber.initGrabber(camWidth, camHeight);
    
    asciiCharacters =  string("  ..,,,'''``--_:;^^**""=+<>iv%&xclrs)/){}I?!][1taeo7zjLunT#@JCwfy325Fp6mqSghVd4EgXPGZbYkOA8U$KHDBWNMR0Q");

    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    grabber.update();
    
    if (grabber.isFrameNew()) {
        
        videoFPS.addFrame();
        
        if (bOutputAscii) {
            // convert it to ascii values
            // adapted from OF ascii video example
            stringstream ss;
            ofPixels &pixels = grabber.getPixels();
            
            for (int row = 0; row < camHeight; row += stride*stretch){

                ss << "\n";
                for (int col = 0; col < camWidth; col += stride){
                    
                    // get the pixel and its lightness (lightness is the average of its RGB values)
                    float lightness = 255 - pixels.getColor(col,row).getLightness();
                    
                    // calculate the index of the character from our asciiCharacters array
                    int character = powf( ofMap(lightness, 0, 255, 0, 1), 2.5) * asciiCharacters.size();
                    
                    // draw the character at the correct location
                    ss << ofToString(asciiCharacters[character]);
                }
            }
            
            // draw it to the terminal
            cout << ss.str() << "\n" << endl;
        }
        
        if (bDoFlow) {
            
            fb.setPyramidScale(fbPyrScale);
            fb.setNumLevels(fbLevels);
            fb.setWindowSize(fbWinSize);
            fb.setNumIterations(fbIterations);
            fb.setPolyN(fbPolyN);
            fb.setPolySigma(fbPolySigma);
            fb.setUseGaussian(fbUseGaussian);
            
            fb.calcOpticalFlow(grabber);
        }

        videoFPS.update();
        
        if (bOutputFPS) {
            cout << "Video FPS: " << ofToString(videoFPS.getFPS()) << "\t\tApp FPS: " << ofGetFrameRate() << endl;
        }
    }
    
    videoFPS.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    if (bDrawVideo) grabber.draw(0, 0);
    
    if (bDrawFlow) {
        if (bDrawVideo) {
            ofPushStyle();
            ofSetColor(0);
            ofDrawRectangle(0, camHeight, camWidth, camHeight);
            ofPopStyle();
            fb.draw(0, camHeight);
        } else {
            ofPushStyle();
            ofSetColor(0);
            ofDrawRectangle(0, 0, camWidth, camHeight);
            ofPopStyle();
            fb.draw(0, 0);
        }
    }
    
    if (bDrawFPS) {
        stringstream ss;
        ss << "Video FPS: " << ofToString(videoFPS.getFPS()) << "\t";
        ss << "App FPS: " << ofToString(ofGetFrameRate());
        ofDrawBitmapStringHighlight(ss.str(), 10, 20);
    }
    
    if (bDrawGui) {
        panel.setPosition(ofGetWidth() - panel.getWidth(), 0);
        panel.draw();
    }

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
