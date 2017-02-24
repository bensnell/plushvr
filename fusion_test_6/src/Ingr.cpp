//
//  Ingr.cpp
//  foot_tracking_test_3
//
//  Created by Ben Snell on 1/26/17. -- updated version
//
//

#include "Ingr.h"

// -----------------------------------------------------------
void Ingr::setup(string _name, int _nValues, float inLowerBound, float inUpperBound, bool _bTwoSided) {
    
    name = _name;
    
    bTwoSided = _bTwoSided;
    
    // setup gui
    params.setName(name);
    params.add(bDraw.set("Draw", true));
    params.add(min.set("Min", inLowerBound, inLowerBound, inUpperBound/4));
    params.add(max.set("Max", inUpperBound, inUpperBound/5, inUpperBound));
    params.add(centerOffset.set("Offset", 0, 0, (inUpperBound-inLowerBound)/2));
    params.add(power.set("Power", 1, 0, 4));
    params.add(bInvert.set("Invert", false));
    params.add(bDifference.set("Difference", false));
    params.add(easingParam.set("Easing", 0.75, 0, 1));
    params.add(bPrior.set("Prior", false));
    params.add(alpha.set("Alpha Prior", 0.01, 0.0001, 5));
    params.add(beta.set("Beta Prior", 0.01, 0.0001, 5));
    params.add(bWeightBeta.set("Weight Beta", false));
    
    setNumValues(_nValues);
    
    min.addListener(this, &Ingr::updateFloat);
    max.addListener(this, &Ingr::updateFloat);
    centerOffset.addListener(this, &Ingr::updateFloat);
    power.addListener(this, &Ingr::updateFloat);
    bInvert.addListener(this, &Ingr::updateBool);
    bDifference.addListener(this, &Ingr::updateBool);
    easingParam.addListener(this, &Ingr::updateFloat);
    bPrior.addListener(this, &Ingr::updateBool);
    alpha.addListener(this, &Ingr::updateFloat);
    beta.addListener(this, &Ingr::updateFloat);
    bWeightBeta.addListener(this, &Ingr::updateBool);
}

// -----------------------------------------------------------
void Ingr::setNumValues(int _nValues) {
    
    if (_nValues < 1) {
        cout << "Number of values to store must be >= 1" << endl;
        nValues = 1;
    } else {
        nValues = _nValues;
    }
}

// -----------------------------------------------------------
void Ingr::trash() {
    
    // reset all values
    raws.clear();
    cooks.clear();
    
    tastes.clear();
    tasteCounter = 0;
    
    steps.clear();
    
    bPrevAvg = false;
    bPrevDiff = false;
    bPrevPrior = false;
}

// -----------------------------------------------------------
void Ingr::addRaw(float _raw) {
    
    raw = _raw;
    cook = raw;
    
    // clear previous steps
    steps.clear();
    
    // add to previous raws
    insertNew(raws, raw, nValues);
}

// -----------------------------------------------------------
void Ingr::insertNew(vector<float> &array, float value, int maxValues) {
    
    // don't continue if we're not storing history
    if (!bStoreHistory) return;
    
    array.insert(array.begin(), value);
    while (array.size() > maxValues) {
        array.pop_back();
    }
}

// -----------------------------------------------------------
float Ingr::getRaw() {
    
    return raw;
}

// -----------------------------------------------------------
void Ingr::normalize() {
    
    // insert this step into the ingredients
    steps.push_back(NORM);
    
    // map the value to [0, 1]
    cook = normalizeCalc(cook, min, max, centerOffset);
}

// -----------------------------------------------------------
float Ingr::normalizeCalc(float _value, float _min, float _max, float _centerOffset) {
    
    if (bTwoSided) {
        // [-1, 1]
        // account for a center buffer region with nonzero _centerOffset
        if (_value < _min) {
            return map(_value, -_max, _min - _centerOffset, -1, 0, true);
        } else {
            return map(_value, _min + _centerOffset, _max, 0, 1, true);
        }
    } else {
        // one-sided data
        // [0, 1]
        return map(_value, min, max, 0, 1, true);
    }
}

// -----------------------------------------------------------
float Ingr::map(float param, float minIn, float maxIn, float minOut, float maxOut, bool bClamp) {
    
    float mapped = (param - minIn) / (maxIn - minIn) * (maxOut - minOut) + minOut;
    return bClamp ? CLAMP(mapped, minOut, maxOut) : mapped;
}

// -----------------------------------------------------------
void Ingr::sensitize() {
    
    steps.push_back(SENS);
    
    cook = sensitizeCalc(cook, power);
}

// -----------------------------------------------------------
float Ingr::sensitizeCalc(float _value, float _power) {
    
    return (_value < 0) ? -pow(-_value, _power) : pow(_value, _power);
}

// -----------------------------------------------------------
void Ingr::invert() {
    
    steps.push_back(INV);
    
    cook = invertCalc(cook, bInvert);
}

// -----------------------------------------------------------
float Ingr::invertCalc(float _value, bool _bInvert) {

    if (bTwoSided) {
        return _bInvert ? (-_value) : _value;
    } else {
        return _bInvert ? (1-_value) : _value;
    }
}

// -----------------------------------------------------------
void Ingr::difference() {
    
    steps.push_back(DIFF);
    
    cook = differenceCalc(cook, &prevDiff, &bPrevDiff, bDifference);
}

// -----------------------------------------------------------
float Ingr::differenceCalc(float _thisValue, float *_prevValue, bool *_bPrevValue, bool _bDifference) {
    
    // don't do calculations if we're not calculating differences
    if (!_bDifference) return _thisValue;
    
    float result;
    
    if (*_bPrevValue) {
        // previous data exists
        // if we don't have a two-sided distribution, then do absolute value of the difference
        result = bTwoSided ? (_thisValue - *_prevValue) : abs(_thisValue - *_prevValue);
        *_prevValue = _thisValue;
    } else {
        // previous data does not exist
        *_prevValue = _thisValue;
        result = 0;
        *_bPrevValue = true;
    }
    
    return result;
}

// -----------------------------------------------------------
void Ingr::average() {
    
    steps.push_back(AVG);
    
    cook = averageCalc(cook, &prevAvg, &bPrevAvg, easingParam);
}

// -----------------------------------------------------------
float Ingr::averageCalc(float _thisValue, float *_prevValue, bool *_bPrevValue, float _easingParam) {
    
    float result;
    
    if (*_bPrevValue) {
        result = *_prevValue * _easingParam + _thisValue * (1-easingParam);
        *_prevValue = result;
    } else {
        result = _thisValue;
        *_prevValue = _thisValue;
        *_bPrevValue = true;
    }
    
    return result;
}

// -----------------------------------------------------------
void Ingr::prior() {
    
    steps.push_back(PRIOR);
    
    cook = priorCalc(cook, &prevInputPrior, &prevOutputPrior, &bPrevPrior, alpha, beta, bPrior);
}

// -----------------------------------------------------------
float Ingr::priorCalc(float _thisValue, float *_prevInputPrior, float *_prevOutputPrior, bool *_bPrevValue, float _alpha, float _beta, bool _bPrior) {
    
    if (!_bPrior) return _thisValue;
    
    float result;
    
    // if there are no previous priors, first set previous values to zero
    if (!*_bPrevValue) {
        
        *_prevInputPrior = 0;
        *_prevOutputPrior = 0;
        
        // reset flag
        *_bPrevValue = true;
    }
    
    // determine the easing probability based on this being a one or two-sided distribution
    // probability depends on previous input values to this function, not output values
    float probability;
    if (bTwoSided) {    // two sided distribution

        if (bWeightBeta) {
            // weights distribution based on _thisValue
            probability = (abs(_thisValue) + _alpha) / (abs(_thisValue) + (((*_prevInputPrior > 0 && _thisValue < 0) || (*_prevInputPrior < 0 && _thisValue > 0)) ? 0 : abs(*_prevInputPrior)) + _alpha + abs(_thisValue)*_beta);
        } else {
            // normal algorithm
            probability = (abs(_thisValue) + _alpha) / (abs(_thisValue) + (((*_prevInputPrior > 0 && _thisValue < 0) || (*_prevInputPrior < 0 && _thisValue > 0)) ? 0 : abs(*_prevInputPrior)) + _alpha + _beta);

        }
        
    } else {            // one sided distribution
        probability = (_thisValue + _alpha) / (_thisValue + *_prevInputPrior + _alpha + _beta);
    }
    
    // optionally, easing can be applied to the probability to more drastically separate the highs from the lows
    
    // calculate the new prior
    // this depends on the probability and the previous output prior
    result = _thisValue * probability + *_prevOutputPrior * (1-probability);
    
    // set the new previous values
    *_prevInputPrior = _thisValue;
    *_prevOutputPrior = result;
    
    return result;
}

// -----------------------------------------------------------
void Ingr::doneCooking() {
    
    insertNew(cooks, cook, nValues);
    
    // reset the taste counter
    tasteCounter = 0;
}

// -----------------------------------------------------------
void Ingr::taste() {
    
    if (tasteCounter < tastes.size()) {
        // this taste already exists, so add to it
        insertNew(tastes[tasteCounter], cook, nValues);
        // increment tastes
        tasteCounter++;
    } else if (tasteCounter == tastes.size()) {
        // insert a new taste now
        vector<float> newTaste;
        insertNew(newTaste, cook, nValues);
        tastes.push_back(newTaste);
    } else {
        cout << "The taste counter is out of range. Requested taste cannot be completed." << endl;
    }
}

// -----------------------------------------------------------
float Ingr::getTaste(int index) {
    
    if (index < 0 || index >= tastes.size()) {
        cout << "Taste Index is out of range" << endl;
        return 0;
    }
    
    if (tastes[index].size() == 0) {
        cout << "There are no tastes stored in index " << index << endl;
    }
    
    return tastes[index][0];
}

// -----------------------------------------------------------
float Ingr::getCook() {
    
    return cook;
}

// -----------------------------------------------------------
void Ingr::updateFloat(float &value) {
    
    refresh();
}

// -----------------------------------------------------------
void Ingr::updateBool(bool &value) {
    
    refresh();
}

// -----------------------------------------------------------
void Ingr::refresh() {
    
    if (!bRefresh) return;
    
    float _prevDiff;
    bool _bPrevDiff = false;
    float _prevAvg;
    bool _bPrevAvg = false;
    float _prevInputPrior;
    float _prevOutputPrior;
    bool _bPrevPrior = false;
    
    // given all the current raw's, recalculate all the cooks
    // assume all vectors are the same length
    for (int i = raws.size()-1; i >= 0; i--) {
        
        float tmpCook = raws[i];
        int tmpTasteCounter = 0;
        
        for (int j = 0; j < steps.size(); j++) {
            switch(steps[j]) {
                    
                case NORM:
                    tmpCook = normalizeCalc(tmpCook, min, max, centerOffset);
                    break;
                case SENS:
                    tmpCook = sensitizeCalc(tmpCook, power);
                    break;
                case INV:
                    tmpCook = invertCalc(tmpCook, bInvert);
                    break;
                case DIFF:
                    tmpCook = differenceCalc(tmpCook, &_prevDiff, &_bPrevDiff, bDifference);
                    if (i == 0) prevDiff = _prevDiff;
                    break;
                case AVG:
                    tmpCook = averageCalc(tmpCook, &_prevAvg, &_bPrevAvg, easingParam);
                    if (i == 0) prevAvg = _prevAvg;
                    break;
                case PRIOR:
                    tmpCook = priorCalc(tmpCook, &_prevInputPrior, &_prevOutputPrior, &_bPrevPrior, alpha, beta, bPrior);
                    if (i == 0) {
                        prevInputPrior = _prevInputPrior;
                        prevOutputPrior = _prevOutputPrior;
                    }
                    break;
                case CHECK:
                    tastes[tmpTasteCounter][i] = tmpCook;
                    tmpTasteCounter++;
                    break;
            }
        }
        
        // now cook has the proper value to store
        cooks[i] = tmpCook;
        if (i == 0) cook = tmpCook;
    }
}

// -----------------------------------------------------------
void Ingr::draw(int x, int y, int w, int h, vector<int> indices, bool bMargin, int nRefRegions) {
    
    if (!bDraw) return;
    
    vector<ofColor> colors;
    vector<ofPolyline> lines;
    
    if (indices.size() == 0) {
        for (int i = 0; i < tastes.size(); i++) {
            indices.push_back(i);
        }
        indices.push_back(-1);
    }
    
    
    for (int i = 0; i < indices.size(); i++) {
        
        int index = indices[i];
        if (index < -1 || index >= int(tastes.size())) continue;
        
        vector<float> *arr = (index == -1) ? &cooks : &(tastes[index]);
        
        // add a color
        if (index == -1) {
            colors.push_back(ofColor(255));
        } else {
            colors.push_back(preColors[index%preColors.size()]);
        }
        
        // create the line
        float graphScale, graphOffset;
        if (!bMargin) {
            graphScale = h;
            graphOffset = h;
        } else {
            graphScale = h/2.;
            graphOffset = h*3./4.;
        }
        ofPolyline line;
        float spacing = float(w) / float(nValues);
        for (int j = 0; j < arr->size(); j++) {
            line.addVertex(j * spacing, -(*arr)[j] * graphScale + graphOffset);
        }
        lines.push_back(line);
    }
    
    ofPushMatrix(); ofPushStyle();
    ofTranslate(x, y);
    ofSetColor(255);
    ofSetLineWidth(1);
    ofDrawBitmapString(name, 2, 10);
    if (bTwoSided) ofScale(1, 0.5);
    ofDrawLine(0, 0, 0, bTwoSided ? (2 * h) : h);
    if (bMargin) {
        ofDrawLine(0, h*0.75, w, h*0.75);
        ofSetColor(255, 100);
        ofDrawLine(0, h*0.25, w, h*0.25);
    } else {
        ofDrawLine(0, h, w, h);
    }
    if (nRefRegions > 0) {
        ofSetColor(255, 100);
        for (int i = 0; i < nRefRegions+1; i++) {
            ofDrawLine(0, h/nRefRegions*i, w, h/nRefRegions*i);
        }
    }
    for (int i = 0; i < colors.size(); i++) {
        ofSetColor(colors[i]);
        lines[i].draw();
    }
    ofPopMatrix(); ofPopStyle();
}




