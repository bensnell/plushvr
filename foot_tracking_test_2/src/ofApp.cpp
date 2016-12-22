#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

// TODO:
// Movement suppressed by a change in roll or pitch (this sketch already suppresses yaw, though we can further suppress it)

//--------------------------------------------------------------
void ofApp::setup(){
    
//    ofSetVerticalSync(true);
    
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
    
    general.setName("General Settings");
    general.add(bDrawImage.set("Draw Img", true));
    general.add(bDrawFlow.set("Draw Flow", true));
    general.add(bodyRadius.set("Body Radius", 50, 1, 200));
    general.add(maxMagnitude.set("Max Mag", 10, 1, 30));
    general.add(bNormalizeHistogram.set("Norm Hist", true));
    general.add(histogramScale.set("Norm Hist Scale", 1, 0.1, 100));
    general.add(nStats.set("Num Stats Graphed", 50, 10, 500));
    general.add(maxVarianceHeight.set("Max Var Graph Ht", 10000, 100, 20000));
    general.add(meanSmoothingFactor.set("Smooth Mean", 0.9, 0.5, 0.9999));
    general.add(varianceSmoothingFactor.set("Smooth Var", 0.9, 0.5, 0.9999));
    general.add(confThresh.set("Conf Thresh", 1000, 100, 3000));
    general.add(confMult.set("Conf Mult", 0.1, 1, 100));
    general.add(confMax.set("Max Conf", 0, 50, 100));
    general.add(confSmoothingFactor.set("Smooth Conf", 0.9, 0.5, 1));
    general.add(confPower.set("Conf Power", 1, 0.5, 5));
    general.add(angleMask.set("Angle Mask", 90, 45, 135));
    general.add(activityPower.set("Act Power", 1, 0.1, 1));
    general.add(variancePower.set("Var Power", 1, 0.1, 2));
    general.add(mixPower.set("Mix Power", 1, 0.1, 4));
    general.add(activityMax.set("Act Max", 5, 1, 10));
    general.add(activityMin.set("Act Min", 1, 0.001, 4));
    general.add(varianceMin.set("Var Min", 100, 0.001, 500));
    general.add(mixMin.set("Mix Min", 0, 0, 1));
    general.add(mixMax.set("Mix Max", 1, 0, 1));
    
    panel.setup();
    panel.add(general);
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
    
    activityIngr.setup(nStats);
    varianceIngr.setup(nStats);
    confAvgr.setup(nStats);
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    if (bLive) {
        grabber.update();
    } else {
        player.update();
    }
    
    // determine if we have a new frame (player logic is broken
    // should be:
    // if (bLive ? grabber.isFrameNew() : player.isFrameNew()) {
    // but is:
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
    
    if (bDrawImage) {
        if (bLive) {
            grabber.draw(imgW*2, 0, imgW*2, imgH*2);
        } else {
            player.draw(imgW*2, 0, imgW*2, imgH*2);
        }
    }
    if (bDrawFlow) {
        flow.draw(0, 0, imgW*2, imgH*2);
        drawBody(0, 0, imgW*2, imgH*2);
    }
    
    // plot the histograms of the optical flow directions
    drawHistogram(bucketsA, nBuckets, 0, imgH*2, imgW*2, imgH*2);

    // draw the statistics for the histogram
    drawStatistics(means, variances, imgW*2, imgH*2, imgW*2, imgH*2);
    // draw the average statistics
    drawStatistics(avgMeans, avgVariances, imgW*2, imgH*2, imgW*2, imgH*2, ofColor(0, 255, 0, 150));
    
    
    
    // draw the variance ingredient
    drawPlot(varianceIngr.values, 1, varianceIngr.nHistories, imgW*2, imgH*4, imgW*2, imgH/2);
    // draw the activity ingredient
    drawPlot(activityIngr.values, 1, activityIngr.nHistories, imgW*2, imgH*9/2, imgW*2, imgH/2);
    // draw the confidence
    drawPlot(confAvgr.values, 1, confAvgr.nHistories, imgW*2, imgH*5, imgW*2, imgH/2);
    
    
    
    // draw the confidence -- certainty with which we're moving straight
//    drawPlot(confs, confMax, nStats, imgW*2, imgH*4, imgW*2, imgH);
    // draw the average confidence
//    drawPlot(avgConfs, confMax, nStats, imgW*2, imgH*4, imgW*2, imgH, ofColor(0, 255, 0, 150));
    // draw the average magntiude of the flow -- basically a measure of activity in the scene
//    drawPlot(activities, 1, nStats, imgW*2, imgH*5, imgW*2, imgH/2);
    
    
    drawPredictedMovement(imgW*3, imgH, 100);
    
    panel.setPosition(ofGetWidth() - panel.getWidth() - 10, 10);
    panel.draw();
    
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
    activity = sumDist / float(nAvgValues);
    // store this distance
    activities.push_back(activity);
    if (activities.size() > nStats) {
        activities.erase(activities.begin());
    }
    
    
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
    
    // Add these statistics to vectors
    means.push_back(dirMean);
    variances.push_back(dirVar);
    if (means.size() > nStats) {
        means.erase(means.begin());
        variances.erase(variances.begin());
    }
    
    // ---------------------------------------------
    // ------- COMBINE VARIANCE AND ACTIVITY -------
    // ---------------------------------------------
    
    // combine activity level and variance to ensure a low variance and low activity does not correspond to a high confidence
    
    // --------- GET ACTIVITY INGREDIENT ---------
    
    // normalize activity
    float activityNorm = map(activity, activityMin, activityMax, 0, 1, true);
    
    // adjust the sensitivity (activitySensitized)
    float activitySens = pow(activityNorm, activityPower);
    
    activityIngr.add(activitySens);
    
    // --------- GET VARIANCE INGREDIENT ---------
    
    // normalize variance -- max shoudl be about 2500
    // should the min be zero?
    float varianceNorm = map(dirVar, varianceMin, maxVarianceHeight, 0, 1, true);
    
    // optionally, sensitize variance (could sensitize after mixing)
    float varianceSens = pow(varianceNorm, variancePower);
    
    // invert variance (since low values correspond to high confidence levels)
    float varianceInv = 1 - varianceSens;
    
    varianceIngr.add(varianceInv);
    
    // ------------- MIX INGREDIENTS -------------
    
    // combine the signals
    float mix = activitySens * varianceInv;
    
    // re-normalize the mix, since the values so rarely reach 1
    float mixNorm = map(mix, mixMin, mixMax, 0, 1, true);
    
    // adjust the sensitivity
    float mixSens = pow(mixNorm, mixPower);
    
    // don't apply a sigmoid because it reduces the linearity of the upper values, squishing them all into 1 (a boolean)
    
    
    
    // smooth the signal
    
    conf = mixSens;
    confAvgr.add(conf);
    
    
    
    
    // ---------------------------------------------
    // ------------- SMOOTH STATISTICS -------------
    // ---------------------------------------------
    
    // Smooth the mean and the variance over time (moving averages)
    if (bNoPreviousData) {
        // just set the current mean
        avgDirMean = dirMean;
        avgDirVar = dirVar;
        bNoPreviousData = false;
    } else {
        // factor in previous data
        avgDirMean = avgDirMean * meanSmoothingFactor + dirMean * (1-meanSmoothingFactor);
        avgDirVar = avgDirVar * varianceSmoothingFactor + dirVar * (1-varianceSmoothingFactor);
    }
    
    avgMeans.push_back(avgDirMean);
    avgVariances.push_back(avgDirVar);
    if (avgMeans.size() > nStats) {
        avgMeans.erase(avgMeans.begin());
        avgVariances.erase(avgVariances.begin());
    }
    
    
    
    
    
    // ---------------------------------------------
    // -------------- FIND CONFIDENCE --------------
    // ---------------------------------------------
    
    // Use variance as a measure of confidence in which direction we're heading
    // This is the certainty with which we know we're heading in avgDirMean
    
    // store the confidence over time
    confs.push_back(conf);
    if (confs.size() > nStats) {
        confs.erase(confs.begin());
    }
    
    // find average confidence
    if (bNoPreviousConf) {
        bNoPreviousConf = false;
        avgConf = conf;
    } else {
        avgConf = avgConf * confSmoothingFactor + conf * (1-confSmoothingFactor);
    }
    avgConfs.push_back(avgConf);
    if (avgConfs.size() > nStats) {
        avgConfs.erase(avgConfs.begin());
    }

    
    
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
void ofApp::drawStatistics(vector<float> _means, vector<float> _variances, int x, int y, int w, int h, ofColor color) {
    
    if (_means.size() < 2) return;
    
    if (_means.size() != _variances.size()) {
        cout << "Statistics not the same size" << endl;
        return;
    }
    
    float margin = 5; // margin between graphs
    float graphHeight = (h - margin)/2.;
    
    // create two polylines
    ofPolyline meanLine, varLine;
    int nPts = _means.size();
    float spacing = float(w) / float(nStats - 1);
    for (int i = 0; i < nPts; i++) {
        meanLine.addVertex(i*spacing, graphHeight * (1 - _means[nPts - 1 - i]/360.));
        varLine.addVertex(i*spacing, graphHeight * (1 - _variances[nPts - 1 - i]/maxVarianceHeight));
    }
    
    ofPushMatrix(); ofPushStyle();
    ofSetColor(color);
    
    // draw mean graph
    ofTranslate(x, y);
    ofDrawLine(0, 0, 0, graphHeight);
    ofDrawLine(0, graphHeight, w, graphHeight);
    ofPushStyle(); ofSetColor(255, 100);
    for (int i = 0; i < 5; i++) {
        ofDrawLine(0, i*graphHeight/4., w, i*graphHeight/4.);
    }
    ofPopStyle();
    meanLine.draw();
    
    // draw variance graph
    ofTranslate(0, graphHeight + margin);
    ofDrawLine(0, 0, 0, graphHeight);
    ofDrawLine(0, graphHeight, w, graphHeight);
    varLine.draw();
    
    ofPopMatrix(); ofPopStyle();
    
}

//--------------------------------------------------------------
void ofApp::drawPlot(vector<float> values, float max, int maxNumValues, int x, int y, int w, int h, ofColor color) {
    
    if (values.size() < 2) return;
    
    // Create a polyline
    ofPolyline line;
    float spacing = float(w) / float(maxNumValues - 1);
    float nValues = values.size();
    for (int i = 0; i < nValues; i++) {
        line.addVertex(i*spacing, h * (1 - values[nValues - 1 - i]/max));
    }
    
    ofPushMatrix(); ofPushStyle();
    ofSetColor(color);
    
    ofTranslate(x, y);
    ofDrawLine(0, 0, 0, h);
    ofDrawLine(0, h, w, h);
    line.draw();
    ofDrawBitmapString(ofToString(values.back()), 0, 10);
    
    ofPopMatrix(); ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawPredictedMovement(int x, int y, int size) {
    
    // uses average direction and confidence to predict heading and amount
    float scale = confAvgr.avg;
    
    float strokeWeight = scale * size / 10.;
    float length = scale * size;
    
    ofPushMatrix(); ofPopStyle();
    ofSetLineWidth(strokeWeight);
    ofSetColor(255);
    
    ofTranslate(x, y);
    
    ofRotate(avgDirMean+180);
    
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
        means.clear();
        variances.clear();
        bNoPreviousData = true;
        avgMeans.clear();
        avgVariances.clear();
        activities.clear();
        confs.clear();
        bNoPreviousConf = true;
        avgConfs.clear();
        
        activityIngr.reset();
        varianceIngr.reset();
        confAvgr.reset();
        
    }
    if (key == 'f') ofToggleFullscreen();
}

// ------------------------------------------------------------------
float ofApp::map(float param, float minIn, float maxIn, float minOut, float maxOut, bool bClamp) {
    
    float mapped = (param - minIn) / (maxIn - minIn) * (maxOut - minOut) + minOut;
    return bClamp ? CLAMP(mapped, minOut, maxOut) : mapped;
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
