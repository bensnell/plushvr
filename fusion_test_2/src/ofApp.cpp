#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

// TODO:
// Movement suppressed by a change in roll or pitch (this sketch already suppresses yaw, though we can further suppress it)
// For some reason, MIN and MAX are not defined here...

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
    statisticsParams.add(angleMask.set("Angle Mask", 90, 45, 135));
    
    ingrParams.setName("Ingredient Params");
    ingrParams.add(nIngrValues.set("Num Values", 300));
    ingrParams.add(bStoreIngrHistory.set("Store History", true));
    
    renderingParams.setName("Rendering Params");
    renderingParams.add(windowW.set("Window Width", 1024, 1, 2000));
    renderingParams.add(windowH.set("Window Height", 768, 1, 2000));
    renderingParams.add(bNormalizeHistogram.set("Norm Hist", true));
    renderingParams.add(histogramScale.set("Norm Hist Scale", 45, 0.1, 100));
    renderingParams.add(ingrW.set("Ingr Width", 500, 1, 1000));
    renderingParams.add(ingrH.set("Ingr Height", 150, 1, 500));
    renderingParams.add(videoScale.set("Video Scale", 2, 0.1, 5));
    renderingParams.add(bDrawAll.set("Draw All", false));
    renderingParams.add(bDrawNone.set("Draw None", false));
    renderingParams.add(bDrawGui.set("Draw Gui", false));
    renderingParams.add(bDrawFPS.set("Draw FPS", true));
    renderingParams.add(bDrawVideo.set("Draw Video", true));
    renderingParams.add(bDrawArrow.set("Draw Arrow", true));
    renderingParams.add(bDrawCV.set("Draw CV", false));
    renderingParams.add(bDrawFlow.set("Draw Flow", false));
    renderingParams.add(bDrawHistogram.set("Draw Histogram", false));

    outputParams.setName("Output Params");
    outputParams.add(bOutputAscii.set("Output Ascii Video", false));
    outputParams.add(stride.set("Stride", 5, 1, 100));
    outputParams.add(stretch.set("Stretch", 2, 0.001, 10));
    outputParams.add(bOutputFPS.set("Output FPS", false));
    outputParams.add(bOutputDirection.set("Output Direction", false));
    outputParams.add(bOutputMPU.set("Output MPU", false));
    
    dir.setup("Direction", 300, 0, 360);
    var.setup("Variance", 300, 0, 3000);
    act.setup("Activity", 300, 0, 5);
    mix.setup("Mix", 300, 0, 1);
    mpuX.setup("MPU X", 300, 0, 15);
    mpuY.setup("MPU Y", 300, 0, 15);
    mixX.setup("Mix X", 300, 0, 1);
    mixY.setup("Mix Y", 300, 0, 1);
    
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
    panels[1].add(dir.params);
    panels[1].add(var.params);
    panels[1].add(act.params);
    panels[1].add(mix.params);
    panels[1].add(mpuX.params);
    panels[1].add(mpuY.params);
    panels[1].add(mixX.params);
    panels[1].add(mixY.params);
    panels[1].loadFromFile(ingrPanelFilename);
    
    // update settings from those loaded
    
    bRefreshFlowValues = false;
    bLivePause = false;
    
    // set the shape of the window
    ofSetWindowShape(windowW, windowH);
    
    // update the number of values stored               -- UDPDATE FOR NEW INGR
    dir.setNumValues(nIngrValues);
    var.setNumValues(nIngrValues);
    act.setNumValues(nIngrValues);
    mix.setNumValues(nIngrValues);
    mpuX.setNumValues(nIngrValues);
    mpuY.setNumValues(nIngrValues);
    mixX.setNumValues(nIngrValues);
    mixY.setNumValues(nIngrValues);
    
    dir.bStoreHistory = bStoreIngrHistory;
    var.bStoreHistory = bStoreIngrHistory;
    act.bStoreHistory = bStoreIngrHistory;
    mix.bStoreHistory = bStoreIngrHistory;
    mpuX.bStoreHistory = bStoreIngrHistory;
    mpuY.bStoreHistory = bStoreIngrHistory;
    mixX.bStoreHistory = bStoreIngrHistory;
    mixY.bStoreHistory = bStoreIngrHistory;
    
    
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

    // ------------- SETUP HISTOGRAM -------------
    
    nBuckets = 360 / bucketSize;
    bucketsA = new float[nBuckets];
    bucketsB = new float[nBuckets];
    memset(bucketsA, 0, sizeof(float) * nBuckets);
    memset(bucketsB, 0, sizeof(float) * nBuckets);
    // find representative angles
    repAngleA = new float[nBuckets];
    repAngleB = new float[nBuckets];
    for (int i = 0; i < nBuckets; i++) {
        repAngleA[i] = float(i * bucketSize) + float(bucketSize)/2.;
        repAngleB[i] = repAngleA[i] - 180.;
    }
    
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
    
    // update the imu
#ifdef __arm__
    mpu.update();
    if (bOutputMPU) cout << mpu.getStringValues() << endl;
#endif
    
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
            
            // Average the directions
            calcAverageDirections();
        }
        
        // Draw to the terminal
        if (bOutputAscii) drawAscii(bLive ? grabber.getPixels() : player.getPixels(), imgW, imgH, stride, stretch);
        if (bOutputDirection) drawAsciiDirection(13, dir.getCook()*360., mix.getCook());
        
        // output the frame rate
        videoFPS.update();
        if (bOutputFPS) {
            stringstream ss;
            ss << std::fixed << std::setprecision(2) << "Video FPS:\t" << ofToString(videoFPS.getFPS()) << "\tApp FPS\t" << ofToString(ofGetFrameRate());
            cout << ss.str() << endl;
        }
    }
    videoFPS.update();
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
        ss << "App FPS: " << ofToString(ofGetFrameRate());
        ofDrawBitmapStringHighlight(ss.str(), 0, 20);
        
        py += 25;
    }
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
    if (bDrawHistogram) {
        if (py + ingrH > ofGetHeight()) {
            px = width;
            py = 0;
        }
        
        // plot the histograms of the optical flow directions
        drawHistogram(bucketsA, nBuckets, px, py, ingrW, ingrH);
        
        py += ingrH;
        width = max(width, (int)ingrW);
    }
    if (dir.bDraw) {
        if (py + ingrH > ofGetHeight()) {
            px = width;
            py = 0;
        }
        // draw the direction of flow
        dir.draw(px, py, ingrW, ingrH, {}, false, 4);
        py += ingrH;
        width = max(width, (int)ingrW);
    }
    if (var.bDraw) {
        if (py + ingrH > ofGetHeight()) {
            px = width;
            py = 0;
        }
        // draw the variance of directions
        var.draw(px, py, ingrW, ingrH, {}, false);
        py += ingrH;
        width = max(width, (int)ingrW);
    }
    if (act.bDraw) {
        if (py + ingrH > ofGetHeight()) {
            px = width;
            py = 0;
        }
        // draw the activity (average magnitude of flow)
        act.draw(px, py, ingrW, ingrH, {}, false);
        py += ingrH;
        width = max(width, (int)ingrW);
    }
    if (mix.bDraw) {
        if (py + ingrH > ofGetHeight()) {
            px = width;
            py = 0;
        }
        // draw the mix
        mix.draw(px, py, ingrW, ingrH, {}, false);
        py += ingrH;
        width = max(width, (int)ingrW);
    }
    if (mpuX.bDraw) {
        if (py + ingrH > ofGetHeight()) {
            px = width;
            py = 0;
        }
        mpuX.draw(px, py, ingrW, ingrH, {}, false);
        py += ingrH;
        width = max(width, (int)ingrW);
    }
    if (mpuY.bDraw) {
        if (py + ingrH > ofGetHeight()) {
            px = width;
            py = 0;
        }
        mpuY.draw(px, py, ingrW, ingrH, {}, false);
        py += ingrH;
        width = max(width, (int)ingrW);
    }
    if (mixX.bDraw) {
        if (py + ingrH > ofGetHeight()) {
            px = width;
            py = 0;
        }
        mixX.draw(px, py, ingrW, ingrH, {}, false);
        py += ingrH;
        width = max(width, (int)ingrW);
    }
    if (mixY.bDraw) {
        if (py + ingrH > ofGetHeight()) {
            px = width;
            py = 0;
        }
        mixY.draw(px, py, ingrW, ingrH, {}, false);
        py += ingrH;
        width = max(width, (int)ingrW);
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
        bDrawHistogram = false;

        var.bDraw = false;
        dir.bDraw = false;
        act.bDraw = false;
        mix.bDraw = false;
        mpuX.bDraw = false;
        mpuY.bDraw = false;
        mixX.bDraw = false;
        mixY.bDraw = false;
        
    } else if (bDrawAll) {
        bDrawAll = false;
        bDrawNone = false;
        
        bDrawGui = true;
        bDrawFPS = true;
        bDrawVideo = true;
        bDrawArrow = true;
        bDrawFlow = true;
        bDrawHistogram = true;
        
        var.bDraw = true;
        dir.bDraw = true;
        act.bDraw = true;
        mix.bDraw = true;
        mpuX.bDraw = true;
        mpuY.bDraw = true;
        mixX.bDraw = true;
        mixY.bDraw = true;
    }
}

//--------------------------------------------------------------
void ofApp::exit() {
    panels[0].saveToFile(genPanelFilename);
    panels[1].saveToFile(ingrPanelFilename);
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
    
    maxSummedHeight = radius * nAvgValues;
}

//--------------------------------------------------------------
void ofApp::calcAverageDirections() {
    
    // ---------------------------------------------
    // --------- FIND ANGLES AND MAGNITUDE ---------
    // ---------------------------------------------
    
    // For every arrow in the flow matrix, get its offset (this is the direction in x and y) -- not the position -- and calculate its magnitude to use as a weight in the histogram
    // Should cap the magnitude at a certain amount
    
    // get a pointer to the array of floats that represent the offsets in x and y
    // [x0, y0, x1, y1, x2, y2, ...]
    float* offsets = (float*)flow.getFlow().ptr();
    
    // clear all the data in the buckets
    memset(bucketsA, 0, sizeof(float) * nBuckets);
    memset(bucketsB, 0, sizeof(float) * nBuckets);
    
    // sum of all distances (magnitudes)
    float sumDist = 0;
    
    // find the angles and magnitude of flow at each pixel
    for (int i = 0; i < nPixels; i++) {
        
        // skip values that are not in our mask
        if (!avgMask[i]) continue;
        
        // get the offsets in x and y
        float ox = offsets[i * 2];
        float oy = offsets[i * 2 + 1];
        
        // ^ This is the same as:
        // const Vec2f& vec = flow.getFlow().at<Vec2f>(py, px);
        // cout << vec[0] << "\t" << vec[1] << endl;
        
        // find the magnitude and clamp it below the max
        float dist = min(sqrt(ox * ox + oy * oy), (float)maxMagnitude);
        

        sumDist += dist;
        
        // get the angle in degrees
        float angle = fmod(atan2(oy, ox) * 57.295779513 + 360., 360.);
        
        // put this into the appropriate bin to create a histogram
        int bucketIndex = CLAMP((int)floor(angle / (float)bucketSize), 0, nBuckets-1);
        bucketsA[bucketIndex] += dist;
    }
    
    // Put data from A into B, shifted by 180 degrees
    for (int i = 0; i < nBuckets; i++) {
        bucketsB[i] = bucketsA[(i + nBuckets/2) % nBuckets];
    }
    
    // find the average distance (i.e. the level of activity)
    float activity = sumDist / float(nAvgValues);
    act.addRaw(activity);
    
    
    // ---------------------------------------------
    // -------- CALCULATE MEAN AND VARIANCE --------
    // ---------------------------------------------
    
    // Each bucket now represents the total weight for every representative bucket
    // Find the average weighted angle and weighted variance of angles
    // Do this for both A [0, 360] and B [-180, 180]
    float meanA, varianceA, meanB, varianceB;
    calcStatistics(repAngleA, bucketsA, nBuckets, meanA, varianceA);
    calcStatistics(repAngleB, bucketsB, nBuckets, meanB, varianceB);

    // The distribution with the lower variance is the correct distribution
    float dirMean, dirVar;
    if (varianceA < varianceB) {
        // [0, 360] is the distribution that best fits this data
        
        // Remove all measurements outside of the angle mask
        float *tmpBuckets = new float[nBuckets];
        memcpy(tmpBuckets, bucketsA, nBuckets*sizeof(float));
        for (int i = 0; i < nBuckets; i++) {
            float thisAngle = abs(repAngleA[i] - meanA);
            if (thisAngle > angleMask && thisAngle < (360-angleMask)) {
                // remove the measurement
                tmpBuckets[i] = 0;
            }
        }
        
        // recalculate the mean and variance
        calcStatistics(repAngleA, tmpBuckets, nBuckets, dirMean, dirVar);
        
    } else {
        // [-180, 180] is the best distribution to fit this data
        
        // Remove all measurements outside of the angle mask
        float *tmpBuckets = new float[nBuckets];
        memcpy(tmpBuckets, bucketsB, nBuckets*sizeof(float));
        for (int i = 0; i < nBuckets; i++) {
            float thisAngle = abs(repAngleB[i] - meanB);
            if (thisAngle > angleMask && thisAngle < (360-angleMask)) {
                // remove the measurement
                tmpBuckets[i] = 0;
            }
        }
        
        // recalculate mean and variance
        calcStatistics(repAngleB, tmpBuckets, nBuckets, dirMean, dirVar);
    }
    
    // wrap the mean
    dirMean = fmod(dirMean + 360., 360.);
    dir.addRaw(dirMean);
    dir.normalize();
    dir.taste();
    dir.average();
    dir.doneCooking();
    
    var.addRaw(dirVar);
    
    // ---------------------------------------------
    // ------- COMBINE VARIANCE AND ACTIVITY -------
    // ---------------------------------------------
    
    // combine activity level and variance to ensure a low variance and low activity does not correspond to a high confidence
    
    // --------- GET ACTIVITY INGREDIENT ---------
    
    // normalize activity
    act.normalize();
    act.taste();
    
    // adjust the sensitivity
    act.sensitize();
    
    act.doneCooking();
    
    // --------- GET VARIANCE INGREDIENT ---------
    
    // normalize variance
    var.normalize();
    var.taste();
    
    // sensitize variance
    var.sensitize();
    
    // invert variance (since low values correspond to high confidence levels)
    var.invert();

    var.doneCooking();
    
    // ------------- MIX INGREDIENTS -------------
    
    // combine the signals
    mix.addRaw(act.getCook() * var.getCook());
    
    // re-normalize the mix, since the values so rarely reach 1
    mix.normalize();
    
    // adjust the sensitivity
    mix.sensitize();
    mix.taste();
    
    // average it
    mix.average();
    
    mix.doneCooking();
    
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
void ofApp::drawHistogram(float *_buckets, int _nBuckets, int x, int y, int w, int h) {
    
    ofPushMatrix(); ofPushStyle();
    ofTranslate(x, y);
    
    float barWidth = (float)w / (float)_nBuckets;
    
    // find the highest value
    float max = 0;
    if (bNormalizeHistogram) {
        for (int i = 0; i < _nBuckets; i++) {
            if (_buckets[i] > max) {
                max = _buckets[i];
            }
        }
    }
    
    ofSetColor(255);
    for (int i = 0; i < _nBuckets; i++) {
        
        float barHeight;
        if (!bNormalizeHistogram) {
            barHeight = _buckets[i] / maxSummedHeight * float(h) * histogramScale;
        } else {
            barHeight = _buckets[i] / max * (float)h;
        }
        
        ofDrawRectangle(i * barWidth, h - barHeight, barWidth, barHeight);
    }
    
    // draw labels
    for (int i = 0; i < _nBuckets+1; i++) {
        float extraHeight = (i % (_nBuckets/4) == 0) ? 6 : 2;
        ofDrawLine(i * barWidth, h, i * barWidth, h + extraHeight);
    }
    
    ofPopMatrix(); ofPopStyle();
}

//--------------------------------------------------------------
// calc stats for a histogram
void ofApp::calcStatistics(float *values, float *weights, int nValues, float &mean, float &variance) {
    
    // Find the average
    float sumWeightedValues = 0;
    float sumWeights = 0;
    for (int i = 0; i < nValues; i++) {
        
        // Sum the values with the weights
        sumWeightedValues += values[i] * weights[i];
        sumWeights += weights[i];
    }
    mean = sumWeightedValues / sumWeights;

    // Find the variance
    float weightedSumSquares = 0;
    for (int i = 0; i < nValues; i++) {
        
        // find the difference between a value and the mean
        float diff = values[i] - mean;
        weightedSumSquares += diff * diff * weights[i];
    }
    variance = weightedSumSquares / sumWeights;
}

//--------------------------------------------------------------
void ofApp::drawPredictedMovement(int x, int y, int size) {
    
    // uses average direction and confidence to predict heading and amount
    float scale = mix.getCook();
    
    float strokeWeight = scale * size / 10.;
    float length = scale * size;
    
    ofPushMatrix(); ofPopStyle();
    ofSetLineWidth(strokeWeight);
    ofSetColor(255);
    
    ofTranslate(x, y);
    
    ofRotate(dir.getCook()*360+180);
    
    ofDrawLine(0, 0, length, 0);
    ofDrawLine(length, 0, length/2, -length/3);
    ofDrawLine(length, 0, length/2, length/3);
    
    ofPopMatrix(); ofPopStyle();
    
    ofSetLineWidth(1);
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
        
        dir.trash();
        var.trash();
        act.trash();
        mix.trash();
        
    }
    if (key == 'f') ofToggleFullscreen();
    
    if (key == OF_KEY_UP) panels[selPanel].setPosition(panels[selPanel].getPosition().x, panels[selPanel].getPosition().y - 10);
    if (key == OF_KEY_DOWN) panels[selPanel].setPosition(panels[selPanel].getPosition().x, panels[selPanel].getPosition().y + 10);
    
    if (key == 'g') bDrawGui = !bDrawGui;
    
    if (key == '1') selPanel = 0;
    if (key == '2') selPanel = 1;
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
