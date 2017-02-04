#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::setup(){
    
    // -------------- SETUP GUI ---------------
    
    generalParams.setName("General Params");
    generalParams.add(imgW.set("Image Width", 128, 1, 2000));
    generalParams.add(imgH.set("Image Height", 72, 1, 2000));
    generalParams.add(bLive.set("Live", true));
    generalParams.add(bLivePause.set("Pause Live", false));
    generalParams.add(videoName.set("Video Name", "tracking_test_128x72_30fps.mp4"));
    generalParams.add(desiredFPS.set("Desired FPS", 60, 1, 120));
    
    cvParams.setName("CV Params");
    cvParams.add(bNormalize.set("Normalize", false));
    cvParams.add(bHistEqualize.set("Hist Equalization", false));

    // these settings work for sure on 256 x 144, but may not on lower resolutions or slower systems
    flowParams.setName("Optical Flow Params");
    flowParams.add(bDoFlow.set("Do Op Flow", true));
    flowParams.add(pyramidScale.set("Pyramid Scale", 0.5, 0, 10));
    flowParams.add(numLevels.set("Num Levels", 4, 1, 10));
    flowParams.add(windowSize.set("Window Size", 10, 1, 50));
    flowParams.add(numIterations.set("Num Iterations", 2, 1, 10));
    flowParams.add(polyN.set("Poly N", 7, 1, 20));
    flowParams.add(polySigma.set("Poly Sigma", 1.5, 0.01, 10));
    flowParams.add(useGaussian.set("Use Gaussian", false));
    flowParams.add(bRefreshFlowValues.set("Refresh Params", false));

    statisticsParams.setName("Statistics Params");
    statisticsParams.add(bodyRadius.set("Body Radius", 38, 1, 200));
    statisticsParams.add(maxMagnitude.set("Max Mag", 10, 1, 30));
    statisticsParams.add(nSkipPixels.set("Skipped Pixels", 1, 1, 20));
    
    ingrParams.setName("Ingredient Params");
    ingrParams.add(nIngrValues.set("Num Values", 300));
    ingrParams.add(bStoreIngrHistory.set("Store History", true));
    ingrParams.add(outputMovementThreshold.set("Output Threshold", 0.1, 0, 1));
    ingrParams.add(bSkipSmallReadings.set("Skip Small Readings", false));    

    imuParams.setName("IMU Params");
//    imuParams.add(bFlipX.set("Flip X Roll", false));
//    imuParams.add(bFlipY.set("Flip Y Pitch", false));
    
    renderingParams.setName("Rendering Params");
    renderingParams.add(windowW.set("Window Width", 1024, 1, 2000));
    renderingParams.add(windowH.set("Window Height", 768, 1, 2000));
    renderingParams.add(ingrW.set("Ingr Width", 500, 1, 1000));
    renderingParams.add(ingrH.set("Ingr Height", 150, 1, 500));
    renderingParams.add(videoScale.set("Video Scale", 2, 0.1, 5));
    renderingParams.add(bDrawAll.set("Draw All", false));
    renderingParams.add(bDrawNone.set("Draw None", false));
    renderingParams.add(bDrawGui.set("Draw Gui", false));
    renderingParams.add(bDrawFPS.set("Draw FPS", true));
    renderingParams.add(bDrawMPU.set("Draw MPU", false));
    renderingParams.add(bDrawVideo.set("Draw Video", true));
    renderingParams.add(bDrawArrow.set("Draw Arrow", true));
    renderingParams.add(bDrawCV.set("Draw CV", false));
    renderingParams.add(bDrawFlow.set("Draw Flow", false));

    outputParams.setName("Output Params");
    outputParams.add(bOutputAscii.set("Output Ascii Video", false));
    outputParams.add(stride.set("Stride", 5, 1, 100));
    outputParams.add(stretch.set("Stretch", 2, 0.001, 10));
    outputParams.add(bOutputFPS.set("Output FPS", false));
    outputParams.add(bOutputDirection.set("Output Direction", false));
    outputParams.add(bOutputMPU.set("Output MPU", false));
    
    
    activity.setup("Activity - avg magntiude", 200, 0, 5);
    linearity.setup("Linearity - dYaw inverted", 200, 0, 5);
    reliability.setup("Reliability - extreme tilt inverted", 200, 0, 90);
    mixture1.setup("Mixture1 - magnitude conf", 200, 0, 1);
    xComponent.setup("X Component - of direction", 200, 0, 1, true); // true for two sided distribution (i.e. [-1, 1])
    yComponent.setup("Y Component - of direction", 200, 0, 1, true);
    xStability.setup("X Stability - dRoll inverted", 200, 0, 12, true); // maybe hi should be lower
    yStability.setup("Y Stability - bPitch inverted", 200, 0, 12, true);
    xStabilityPost.setup("X Stability Postprocessing", 200, 0, 1, true);
    yStabilityPost.setup("Y Stability Postprocessing", 200, 0, 1, true);
    mixture2.setup("Mixture 2 - x direction", 200, 0, 1, true);
    mixture3.setup("Mixture 3 - y direction", 200, 0, 1, true);
    
    // setup panels
    
    panels.push_back(*(new ofxPanel()));
    panels.push_back(*(new ofxPanel()));
    
    panels[0].setup();
    panels[0].setName("General Settings");
    panels[0].add(generalParams);
    panels[0].add(cvParams);
    panels[0].add(flowParams);
    panels[0].add(statisticsParams);
    panels[0].add(ingrParams);
    panels[0].add(renderingParams);
    panels[0].add(outputParams);
    panels[0].loadFromFile(genPanelFilename);
    
    panels[1].setup();
    panels[1].setName("Ingr Settings");
    panels[1].add(activity.params);
    panels[1].add(linearity.params);
    panels[1].add(reliability.params);
    panels[1].add(mixture1.params);
    panels[1].add(xComponent.params);
    panels[1].add(xStability.params);
    panels[1].add(xStabilityPost.params);
    panels[1].add(mixture2.params);
    panels[1].add(yComponent.params);
    panels[1].add(yStability.params);
    panels[1].add(yStabilityPost.params);
    panels[1].add(mixture3.params);
    panels[1].loadFromFile(ingrPanelFilename);
    
    // update settings from those loaded
    
    bRefreshFlowValues = false;
    bLivePause = false;
    
    // set the shape of the window
    ofSetWindowShape(windowW, windowH);
    
    // update the number of values stored
    activity.setNumValues(nIngrValues);
    linearity.setNumValues(nIngrValues);
    reliability.setNumValues(nIngrValues);
    mixture1.setNumValues(nIngrValues);
    xComponent.setNumValues(nIngrValues);
    yComponent.setNumValues(nIngrValues);
    xStability.setNumValues(nIngrValues);
    yStability.setNumValues(nIngrValues);
    xStabilityPost.setNumValues(nIngrValues);
    yStabilityPost.setNumValues(nIngrValues);
    mixture2.setNumValues(nIngrValues);
    mixture3.setNumValues(nIngrValues);
    
    activity.bStoreHistory = bStoreIngrHistory;
    linearity.bStoreHistory = bStoreIngrHistory;
    reliability.bStoreHistory = bStoreIngrHistory;
    mixture1.bStoreHistory = bStoreIngrHistory;
    xComponent.bStoreHistory = bStoreIngrHistory;
    yComponent.bStoreHistory = bStoreIngrHistory;
    xStability.bStoreHistory = bStoreIngrHistory;
    yStability.bStoreHistory = bStoreIngrHistory;
    xStabilityPost.bStoreHistory = bStoreIngrHistory;
    yStabilityPost.bStoreHistory = bStoreIngrHistory;
    mixture2.bStoreHistory = bStoreIngrHistory;
    mixture3.bStoreHistory = bStoreIngrHistory;
    
    
    // -------------- SETUP VIDEO ----------------
    
    if (bLive) {
#ifndef __arm__
        grabber.setDeviceID(1);
#endif
        grabber.setDesiredFrameRate(desiredFPS);
        grabber.initGrabber(imgW, imgH);
    } else {
        player.load(videoName);
        player.setLoopState(OF_LOOP_NORMAL);
        player.setPaused(true);
    }
    
    // ------------ SETUP FLOW --------------
    
    refreshFlowParams();
    
    thisFrame.allocate(imgW, imgH, OF_PIXELS_RGB);
    
    bodyRadius.addListener(this, &ofApp::updateBodyRadius);
    nPixels = imgW * imgH;
    avgMask = new bool[nPixels];
    int _bodyRadius = bodyRadius;
    updateBodyRadius(_bodyRadius);

    
    asciiChar =  string("  ..,,,'''``--_:;^^**""=+<>iv%&xclrs)/){}I?!][1taeo7zjLunT#@JCwfy325Fp6mqSghVd4EgXPGZbYkOA8U$KHDBWNMR0Q");
    
    // ---------------- SETUP IMU -----------------
    
#ifdef __arm__
    mpu.setup();
#endif


}

//--------------------------------------------------------------
void ofApp::update(){
    
    if (bLive) {
        // if it's paused, don't take in any new video
        if (!bLivePause) grabber.update();
    } else {
        player.update();
    }
    
    // update the mpu and mix its ingredient
    updateMPU();

    // update settings
    if (bRefreshFlowValues) refreshFlowParams();
    updateDrawingSettings();
    
    // determine if we have a new frame
    if (bLive ? grabber.isFrameNew() : (player.getCurrentFrame() != prevFrame)) {
        if (!bLive) prevFrame = player.getCurrentFrame();

        videoFPS.addFrame();
        
        if (bDoFlow) {
            // do optical flow (and optionally cv)
            if (bLive) {
                // live video
                if (bHistEqualize || bNormalize) {
                    // do cv
                    thisFrame = grabber.getPixels();
                    if (bNormalize) ofxCv::normalize(thisFrame);
                    if (bHistEqualize) ofxCv::equalizeHist(thisFrame);
                    flow.calcOpticalFlow(thisFrame);
                } else {
                    // don't do cv
                    flow.calcOpticalFlow(grabber);
                }
            } else {
                // recorded video
                player.getPixels().resizeTo(thisFrame);
                if (bNormalize) ofxCv::normalize(thisFrame);
                if (bHistEqualize) ofxCv::equalizeHist(thisFrame);
                flow.calcOpticalFlow(thisFrame);
            }
            
            if (bPreviousFrame) {
                // Do all the important calculations
                efficientCalc();
            } else {
                bPreviousFrame = true;
            }
        }
        
        // Draw to the terminal
        if (bOutputAscii) drawAscii(bLive ? grabber.getPixels() : player.getPixels(), imgW, imgH, stride, stretch);
        if (bOutputDirection) drawAsciiDirection(13, ofRadToDeg(atan2(movementForce.y, movementForce.x)), movementForce.length());
        
        // output the frame rate
        videoFPS.update();
        mpuFPS.update();
        if (bOutputFPS) {
            stringstream ss;
            ss << std::fixed << std::setprecision(2) << "Video FPS:\t" << ofToString(videoFPS.getFPS()) << "\tApp FPS\t" << ofToString(ofGetFrameRate()) << "\tMPU FPS:\t" << ofToString(mpuFPS.getFPS());
            cout << ss.str() << endl;
        }
    }
    videoFPS.update();
    mpuFPS.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    if (bDrawNone && !bDrawGui) return;
    
    ofBackground(0);
    
    int px = 0;
    int py = 0;
    int width = 0;
    
    if (bDrawFPS) {
        stringstream ss;
        ss << "Video FPS: " << ofToString(videoFPS.getFPS()) << "\t";
        ss << "App FPS: " << ofToString(ofGetFrameRate()) << "\t";
        ss << "MPU FPS: " << ofToString(mpuFPS.getFPS());
        ofDrawBitmapStringHighlight(ss.str(), 0, py+20);
        
        py += 25;
    }
#ifdef __arm__
    if (bDrawMPU) {
        ofDrawBitmapStringHighlight(mpu.getStringValues(), 0, py+20);
        py += 25;
    }
#endif
    if (bDrawVideo) {
        
        // draw video
        if (bLive) {
            grabber.draw(px, py, imgW*videoScale, imgH*videoScale);
        } else {
            player.draw(px, py, imgW*videoScale, imgH*videoScale);
        }
        
        if (bDrawArrow) {
        // draw arrow on top
            drawPredictedMovement(px + imgW*videoScale/2, py + imgH*videoScale/2, min(imgW*videoScale/2, imgH*videoScale/2));
        }

        py += imgH*videoScale;
        width = max(width, imgW*videoScale);
        
    }
    if (bDrawArrow && !bDrawVideo) {
        
        drawPredictedMovement(px + imgW*videoScale/2, py + imgH*videoScale/2, min(imgW*videoScale/2, imgH*videoScale/2));
        
        py += imgH*videoScale;
        width = max(width, imgW*videoScale);
    }
    if (bDrawCV) {
        if (py + imgH*videoScale > ofGetHeight()) {
            px = width;
            py = 0;
        }
        
        ofImage tmpImg;
        tmpImg.getPixels() = thisFrame;
        tmpImg.update();
        tmpImg.draw(px, py, imgW*videoScale, imgH*videoScale);
        
        py += imgH*videoScale;
        width = max(width, imgW*videoScale);
    }
    if (bDrawFlow) {
        if (py + imgH*videoScale > ofGetHeight()) {
            px = width;
            py = 0;
        }
        
        flow.draw(px, py, imgW*videoScale, imgH*videoScale);
        drawBody(px, py, imgW*videoScale, imgH*videoScale);
        
        py += imgH*videoScale;
        width = max(width, imgW*videoScale);
    }
    if (bDrawGui) {
            
        panels[selPanel].setPosition(ofGetWidth() - panels[selPanel].getWidth() - 10, panels[selPanel].getPosition().y);
        panels[selPanel].draw();
    }
}

//--------------------------------------------------------------
void ofApp::updateDrawingSettings() {
    
    if (bDrawNone) {
        bDrawAll = false;
        
        bDrawFPS = false;
        bDrawVideo = false;
        bDrawArrow = false;
        bDrawFlow = false;
        
    } else if (bDrawAll) {
        bDrawAll = false;
        bDrawNone = false;
        
        bDrawGui = true;
        bDrawFPS = true;
        bDrawVideo = true;
        bDrawArrow = true;
        bDrawFlow = true;
        
    }
}

//--------------------------------------------------------------
void ofApp::exit() {
    panels[0].saveToFile(genPanelFilename);
    panels[1].saveToFile(ingrPanelFilename);
}

//--------------------------------------------------------------
void ofApp::updateMPU() {
    
#ifdef __arm__
    // update the imu accelerometer and gyroscope
    mpu.update();
    
    mpuFPS.addFrame();
    
    // print if requested
    if (bOutputMPU) cout << mpu.getStringValues() << endl;
#endif
}

//--------------------------------------------------------------
void ofApp::refreshFlowParams() {
    
    bRefreshFlowValues = false;
    
    flow.setPyramidScale(pyramidScale);
    flow.setNumLevels(numLevels);
    flow.setWindowSize(windowSize);
    flow.setNumIterations(numIterations);
    flow.setPolyN(polyN);
    flow.setPolySigma(polySigma);
    flow.setUseGaussian(useGaussian);
}

//--------------------------------------------------------------
void ofApp::updateBodyRadius(int &radius) {
    
    // recalculate the array of bools to know whether to include an optical flow measurement in the average
    ofVec2f bodyCenter(imgW/2.f, imgH);
    nAvgValues = 0;
    for (int row = 0; row < imgH; row++) {
        for (int col = 0; col < imgW; col++) {
            
            // if this point is within the body radius, don't include it in the average
            ofVec2f thisPoint(col, row);
            if (bodyCenter.distance(thisPoint) >= radius) {
                avgMask[col + row*imgW] = true;
                nAvgValues++;
            } else {
                avgMask[col + row*imgW] = false;
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::efficientCalc() {
#ifdef __arm__
    // get a pointer to the array of floats that represent the offsets in x and y
    // [x0, y0, x1, y1, x2, y2, ...]
    float* offsets = (float*)flow.getFlow().ptr();
    
    // sum of all magnitudes
    float sumDist = 0;
    
    // sums of vector components in x and y
    float sumXComp = 0;
    float sumYComp = 0;
    
    // First, find the average direction in the optical flow using a circular mean and the average activity
    // Skip every k for more efficiency
    int nSamples = 0;
    for (int row = 0; row < imgH; row += nSkipPixels) {
        for (int col = 0; col < imgW; col += nSkipPixels) {
            
            // find the index of this pixel
            int index = row * imgW + col;
            
            // skip values that are not in our mask
            if (!avgMask[index]) continue;
            
            // get the offsets in x and y
            float ox = offsets[index * 2];
            float oy = offsets[index * 2 + 1];
            
            // if any of the optical flow output is strange, skip it
            if (isnan(ox) || isnan(oy) || isinf(ox) || isinf(oy)) continue;
            
            // find the magnitude and clamp it below the max
            float dist = min(sqrt(ox * ox + oy * oy), (float)maxMagnitude);
            
            // if the distance is weird, skip it
            if (isnan(dist) || isinf(dist)) continue;
            
            // increment the number of samples (will be less than or equal to nPixels)
            nSamples++;

            // if there is no distance to this offset, just continue to prevent NaN's
            if (abs(dist) < 0.0000001) continue;

            // add this distance to our running sum
            sumDist += dist;
            
            // normalize the offsets and add them to our sums
            sumXComp += ox / dist;
            sumYComp += oy / dist;
        }
    }
    ofVec2f avgComp = ofVec2f(sumXComp / float(nSamples),
                              sumYComp / float(nSamples));
    avgComp.normalize();

    // In the following ingredients, averaging is included everywhere but may not need to be done
    
    // Find the average activity and add it to an ingredient
    // This will serve as the primary indicator of movement -- all other ingredients will either define direction or limit movement due to tilts
    activity.addRaw(sumDist / float(nSamples));
    activity.normalize();
    activity.sensitize();
    activity.average();
    activity.doneCooking();
    
    // Find the change in yaw (amount of rotation)
    // This will serve as the amount of linearity of the motion
    // This is one-sided [0, 1] because direction doesn't matter
    linearity.addRaw(mpu.getYaw());
    linearity.difference();
    linearity.normalize();
    linearity.invert();
    linearity.sensitize();
    linearity.average();
    linearity.doneCooking();
    
    // Find the maximum deviation of pitch and roll
    // This will serve as the amount of reliability in our readings, since someone looking directly up or directly down does not produce reliable data
    reliability.addRaw(max(abs(mpu.getRoll()), abs(mpu.getPitch())));
    reliability.normalize();
    reliability.invert();
    reliability.sensitize();
    reliability.average();
    reliability.doneCooking();
    
    // Combine activity, reliability, and linearity into mixture1
    // This mixture represents how confident we are in linear motion. It says nothing about direction of motion.
    mixture1.addRaw(activity.getCook() * linearity.getCook() * reliability.getCook());
    mixture1.normalize(); // superfluous
    mixture1.sensitize();
    mixture1.average();
    mixture1.doneCooking();
    
    // If mix1 is small at this point, don't bother doing anything else
    // I.e. Apply a threshold above which movement will be output
    if (bSkipSmallReadings && mixture1.getCook() < outputMovementThreshold) {
        // set the output to be 0
        movementForce.set(0, 0);
        return;
    }
    
    // Store the x and y components of direction in ingredients
    // These components are between [-1, 1]
    xComponent.addRaw(avgComp.x);
    xComponent.normalize(); // superfluous
    xComponent.average();
    xComponent.doneCooking();

    yComponent.addRaw(avgComp.y);
    yComponent.normalize(); // superfluous
    yComponent.average();
    yComponent.doneCooking();
    
    // Find the changes in Roll and Pitch that may cause the camera to perceive motion in unintended directions
    xStability.addRaw(mpu.getRoll());
    xStability.difference();
    xStability.normalize();
    xStability.average();
    xStability.doneCooking();
    
    yStability.addRaw(mpu.getPitch());
    yStability.difference();
    yStability.normalize();
    yStability.average();
    yStability.doneCooking();
    
    // Apply a second round of smoothing
    xStabilityPost.addRaw(xStability.getCook());
    xStabilityPost.normalize();
    xStabilityPost.sensitize();
    xStabilityPost.average();
    xStabilityPost.doneCooking();
    
    yStabilityPost.addRaw(yStability.getCook());
    yStabilityPost.normalize();
    yStabilityPost.sensitize();
    yStabilityPost.average();
    yStabilityPost.doneCooking();
    
    // Add components, stabilities, and mixture1 together into two new mixes that represent the near-final movement force
    // Components and stabilities are added in a piecewise manner to account for the nonlinear combinations of directions / tilt
    // mixtures 1 and 2 are two-sided [-1, 1]
    
    // add the x values
    // attenuate if vectors are pointing in the same direction
    if (xComponent.getCook() > 0 && xStabilityPost.getCook() > 0) {
        mixture2.addRaw(xComponent.getCook() * xStabilityPost.getCook() * mixture1.getCook());
    } else if (xComponent.getCook() < 0 && xStabilityPost.getCook() < 0) {
        mixture2.addRaw(- xComponent.getCook() * xStabilityPost.getCook() * mixture1.getCook());
    } else {        // don't apply gain if tilt is supporting cam prediction
        mixture2.addRaw(xComponent.getCook() * mixture1.getCook());
    }
    mixture2.normalize();
    mixture2.average();
    mixture2.doneCooking();
    
    // combine the y values
    if (yComponent.getCook() > 0 && yStabilityPost.getCook() > 0) {
        mixture3.addRaw(yComponent.getCook() * (1-yStabilityPost.getCook()) * mixture1.getCook());
    } else if (yComponent.getCook() < 0 && yStabilityPost.getCook() < 0) {
        mixture3.addRaw(yComponent.getCook() * (1+yStabilityPost.getCook()) * mixture1.getCook());
    } else {
        mixture3.addRaw(yComponent.getCook() * mixture1.getCook());
    }
    mixture3.normalize();
    mixture3.average();
    mixture3.doneCooking();
    
    // extract the direction and magnitude from mixtures 2 and 3 into the final movement vector
    movementForce.set(mixture2.getCook(), mixture3.getCook());
    
    
    // APPLY A FINAL ADJUSTMENT TO THE FORCE MAGNITUDE BEFORE EXPORTING
#endif
}

//--------------------------------------------------------------
void ofApp::drawBody(int x, int y, int w, int h) {
    
    ofPushMatrix();ofPushStyle();
    
    ofTranslate(x, y);
    
    ofSetColor(255, 100, 100, 50);
    ofDrawCircle(w/2., h, bodyRadius * w/imgW);
    
    ofPopMatrix(); ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawPredictedMovement(int x, int y, int size) {
    
    // uses average direction and confidence to predict heading and amount
    float scale = movementForce.length();
    float strokeWeight = scale * size / 10.;
    float length = scale * size;
    
    ofPushMatrix(); ofPopStyle();
    ofTranslate(x, y);
    ofRotate(ofRadToDeg(atan2(movementForce.y, movementForce.x))+180);
    ofSetLineWidth(strokeWeight);
    ofSetColor(255);
    ofDrawLine(0, 0, length, 0);
    ofDrawLine(length, 0, length/2, -length/3);
    ofDrawLine(length, 0, length/2, length/3);
    
    ofPopMatrix(); ofPopStyle();
    
    ofSetLineWidth(1);
    ofSetColor(255);
}

//--------------------------------------------------------------
void ofApp::drawAscii(ofPixels &pixels, int w, int h, int _stride, float _stretch) {
    
    stringstream ss;
    
    // adapted from OF ascii video example
    for (int row = 0; row < h; row += _stride*_stretch){
        
        ss << "\n";
        for (int col = 0; col < w; col += _stride){
            
            // get the pixel and its lightness (lightness is the average of its RGB values)
            float lightness = 255 - pixels.getColor(col,row).getLightness();
            
            // calculate the index of the character from our asciiCharacters array
            int index = powf( ofMap(lightness, 0, 255, 0, 1), 2.5) * (asciiChar.size()-1);
            
            // draw the character at the correct location
            ss << ofToString(asciiChar[index]);
        }
    }
    
    // draw it to the terminal
    cout << ss.str() << "\n" << endl;
}

//--------------------------------------------------------------
void ofApp::drawAsciiDirection(int size, float direction, float scale) {

    stringstream ss;
    
    float angle = direction/180.*3.141592654;
    float magnitude = scale * size/2;
    float px = - magnitude * cos(angle);
    float py = magnitude * sin(angle);
    
    // top line
    for (int i = 0; i < size; i++) {
        ss << "__";
    }
    ss << "\n";
    
    // top vertical line
    for (int i = 0; i < size/2-1; i++) {
        ss << "| ";
        for (int i = 0; i < size/2-1; i++) {
            ss << "  ";
        }
        if ((float(size/2-1-i)+0.5) <= py) {
            ss << " |";
        } else {
            ss << "  ";
        }
        for (int i = 0; i < size/2-1; i++) {
            ss << "  ";
        }
        ss << " |" << "\n";
    }
    
    // draw middle line
    ss << "|";
    for (int i = 0; i < size/2; i++) {
        if (px < 0) {
            if ((float(size/2-1-i)+0.5) <= abs(px)) {
                ss << "--";
                if (i == size/2) ss << "--";
            } else {
                ss << "  ";
                if (i == size/2) ss << " ";
            }
        } else {
            ss << "  ";
            if (i == size/2) ss << " ";
        }
    }
    for (int i = 0; i < size/2; i++) {
        if (px > 0) {
            if ((float(i)+0.5) <= px) {
                ss << "--";
            } else {
                ss << "  ";
            }
        } else {
            ss << "  ";
        }
    }
    ss << "|" << "\n";
    
    // do the bottom vertical line
    for (int i = 0; i < size/2-1; i++) {
        ss << "| ";
        for (int i = 0; i < size/2-1; i++) {
            ss << "  ";
        }
        if (py < 0) {
            if ((float(i)+0.5) <= abs(py)) {
                ss << " |";
            } else {
                ss << "  ";
            }
        } else {
            ss << "  ";
        }
        for (int i = 0; i < size/2-1; i++) {
            ss << "  ";
        }
        ss << " |" << "\n";
    }
    
    // bottom line
    ss << "|";
    for (int i = 0; i < size-1; i++) {
        ss << "__";
    }
    ss << "|" << "\n";
    
    cout << ss.str() << endl;
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    if (key == ' ') {
        if (bLive) {
            bLivePause = !bLivePause;
        } else {
            player.setPaused(!player.isPaused());
        }
    }
    if (key == OF_KEY_RIGHT) {
        if (player.getCurrentFrame() == player.getTotalNumFrames()-1) {
            player.firstFrame();
        } else {
            player.nextFrame();
        }
    }
    if (key == OF_KEY_LEFT) {
        if (player.getCurrentFrame() == 1) {
            player.setFrame(player.getTotalNumFrames());
        } else {
            player.previousFrame();
        }
    }
    if (key == 'r') {
        player.firstFrame();
        
        activity.trash();
        linearity.trash();
        reliability.trash();
        mixture1.trash();
        xComponent.trash();
        yComponent.trash();
        xStability.trash();
        yStability.trash();
        mixture2.trash();
        mixture3.trash();
        
    }
    if (key == 'f') ofToggleFullscreen();
    
    if (key == OF_KEY_UP) panels[selPanel].setPosition(panels[selPanel].getPosition().x, panels[selPanel].getPosition().y - 10);
    if (key == OF_KEY_DOWN) panels[selPanel].setPosition(panels[selPanel].getPosition().x, panels[selPanel].getPosition().y + 10);
    
    if (key == 'g') bDrawGui = !bDrawGui;
    
    if (key == '1') selPanel = 0;
    if (key == '2') selPanel = 1;
}
