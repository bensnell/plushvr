//
//  Avgr.h
//  foot_tracking_test_2
//
//  Created by Ben Snell on 12/19/16.
//
//

#ifndef __foot_tracking_test_2__Avgr__
#define __foot_tracking_test_2__Avgr__

#include "ofMain.h"

class Avgr {
    
public:
    
    float value;
    float prev;
    
    bool bPrevData = false;
    
    float avg;
    
    float param = 0.75; // by which average changes
    
    void add(float newValue, float easingParam = -1);
    
    void reset();
    
    void setup(int _nHistories);
    
    // number of values to keep
    int nHistories;
    
    // old values
    vector<float> values;
    vector<float> avgs;
    
};



#endif /* defined(__foot_tracking_test_2__Avgr__) */
