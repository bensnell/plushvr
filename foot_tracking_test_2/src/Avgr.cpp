//
//  Avgr.cpp
//  foot_tracking_test_2
//
//  Created by Ben Snell on 12/19/16.
//
//

#include "Avgr.h"

// -----------------------------------------------------------
void Avgr::setup(int _nHistories) {
    
    nHistories = _nHistories;
}

// -----------------------------------------------------------
void Avgr::add(float _value, float _param) {
    
    if (_param != -1) param = _param;
    
    prev = value;
    value = _value;
    
    values.push_back(value);
    if (values.size() > nHistories) values.erase(values.begin());
    
    if (bPrevData) {
        avg = param * avg + (1.-param) * value;
    } else {
        avg = value;
        bPrevData = true;
    }
    
    avgs.push_back(avg);
    if (avgs.size() > nHistories) avgs.erase(avgs.begin());
}

// -----------------------------------------------------------
void Avgr::reset() {
    
    values.clear();
    avgs.clear();
    bPrevData = false;
}

// -----------------------------------------------------------

// -----------------------------------------------------------

// -----------------------------------------------------------

// -----------------------------------------------------------

// -----------------------------------------------------------

// -----------------------------------------------------------