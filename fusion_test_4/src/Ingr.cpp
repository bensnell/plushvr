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
    params.add(min.set("Min", inLowerBound, inLowerBound, inUpperBound/4));
    params.add(max.set("Max", inUpperBound, inUpperBound/5, inUpperBound));
    params.add(centerOffset.set("Offset", 0, 0, (inUpperBound-inLowerBound)/2));
    params.add(power.set("Power", 1, 0, 4));
    params.add(bInvert.set("Invert", false));
    params.add(bDifference.set("Difference", false));
    params.add(easingParam.set("Easing", 0.75, 0, 1));
    
    setNumValues(_nValues);
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
    
    bPrevAvg = false;
    bPrevDiff = false;
}

// -----------------------------------------------------------
void Ingr::addRaw(float _raw) {
    
    raw = _raw;
    cook = raw;
    
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
    
    cook = sensitizeCalc(cook, power);
}

// -----------------------------------------------------------
float Ingr::sensitizeCalc(float _value, float _power) {
    
    return (_value < 0) ? -pow(-_value, _power) : pow(_value, _power);
}

// -----------------------------------------------------------
void Ingr::invert() {
    
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
void Ingr::doneCooking() {
    
    insertNew(cooks, cook, nValues);
}

// -----------------------------------------------------------
float Ingr::getCook() {
    
    return cook;
}




