#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

// TODO:
// Movement suppressed by a change in roll or pitch (this sketch already suppresses yaw, though we can further suppress it)

//--------------------------------------------------------------
void ofApp::setup(){
    
    if (bLive) {
        grabber.setDeviceID(3);
        grabber.setDesiredFrameRate(60);
        grabber.setup(imgW, imgH);
    } else {
        player.load(videoName);
        player.setLoopState(OF_LOOP_NORMAL);
        player.setPaused(true);
    }
    
    flow.setPyramidScale(0.5);
    flow.setNumLevels(4);
    flow.setWindowSize(10);
    flow.setNumIterations(2);
    flow.setPolyN(7);
    flow.setPolySigma(1.5);
    flow.setUseGaussian(false);
    
    thisFrame.allocate(imgW, imgH, OF_PIXELS_RGB);
    
    general.setName("General");
    general.add(bDrawImage.set("Draw Img", true));
    general.add(bDrawFlow.set("Draw Flow", true));
    general.add(bodyRadius.set("Body Radius", 50, 1, 200));
    general.add(maxMagnitude.set("Max Mag", 10, 1, 30));
    general.add(bNormalizeHistogram.set("Norm Hist", true));
    general.add(histogramScale.set("Norm Hist Scale", 1, 0.1, 100));
    general.add(nStats.set("Num Stats Graphed", 50, 10, 500));
    general.add(angleMask.set("Angle Mask", 90, 45, 135));

    
    dir.setup("Direction", 300, 0, 360);
    var.setup("Variance", 300, 0, 3000);
    act.setup("Activity", 300, 0, 5);
    mix.setup("Mix", 300, 0, 1);
    
    panel.setup();
    panel.add(general);
    panel.add(dir.params);
    panel.add(var.params);
    panel.add(act.params);
    panel.add(mix.params);
    panel.loadFromFile(settingsFilename);
    
    bodyRadius.addListener(this, &ofApp::updateBodyRadius);
    nPixels = imgW * imgH;
    avgMask = new bool[nPixels];
    int _bodyRadius = bodyRadius;
    updateBodyRadius(_bodyRadius);

    // histogram
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
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    if (bLive) {
        grabber.update();
    } else {
        player.update();
    }
    
    // determine if we have a new frame
    if (bLive ? grabber.isFrameNew() : (player.getCurrentFrame() != prevFrame)) {
        if (!bLive) prevFrame = player.getCurrentFrame();

        if (bLive) {
            flow.calcOpticalFlow(grabber);
        } else {
            player.getPixels().resizeTo(thisFrame);
            flow.calcOpticalFlow(thisFrame);
        }
        
        // Average the directions
        calcAverageDirections();
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    // draw the video, flow, and body location
    if (bLive) {
        grabber.draw(0, 0, imgW*2, imgH*2);
    } else {
        player.draw(0, 0, imgW*2, imgH*2);
    }
    flow.draw(0, imgH*2, imgW*2, imgH*2);
    drawBody(0, imgH*2, imgW*2, imgH*2);
    
    // plot the histograms of the optical flow directions
    drawHistogram(bucketsA, nBuckets, 0, imgH*4, imgW*2, imgH);

    // draw the direction of flow
    dir.draw(imgW*2+10, 0, 500, 150, {}, false, 4);
    
    // draw the variance of directions
    var.draw(imgW*2+10, 160, 500, 150, {}, false);

    // draw the activity (average magnitude of flow)
    act.draw(imgW*2+10, 320, 500, 150, {}, false);
    
    // draw the mix
    mix.draw(imgW*2+10, 480, 500, 150, {}, false);
    
    // draw the predicted movement size and direction
    drawPredictedMovement(imgW, imgH, 100);
    
    // gui
    panel.setPosition(ofGetWidth() - panel.getWidth() - 10, 10);
    panel.draw();
    
    // debug
    ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 10, 20);
}

//--------------------------------------------------------------
void ofApp::exit() {
    panel.saveToFile(settingsFilename);
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
        float dist = MIN(sqrt(ox * ox + oy * oy), (float)maxMagnitude);
        
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
void ofApp::keyPressed(int key){

    if (key == ' ') {
        player.setPaused(!player.isPaused());
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
