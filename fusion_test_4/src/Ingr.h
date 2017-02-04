//
//  Ingr.h
//  foot_tracking_test_3
//
//  Created by Ben Snell on 1/26/17. -- updated version
//
//

#ifndef __foot_tracking_test_3__Ingr__
#define __foot_tracking_test_3__Ingr__

#include "ofMain.h"

// Ingredient class: handles data manipulation and visualization of a single variable
// Normalize should always be done, and always be first. The rest of the operations depend on it being there
class Ingr {
    
public:
    
    void setup(string _name, int _nValues, float inLowerBound, float inUpperBound, bool _bTwoSided = false);
    bool bTwoSided = false;
    // Default: bTwoSided = false --> treats data as one-sided [0, 1]
    // bTwoSided = true --> treats the data as existing from [inLowerBound-inUpperBound, inLowerBound+inUpperBound] and normalize it to [-1, 1]. Useful applications for this include difference data (nowData - prevData) and vectors capable of being negative or positive.
    string name;
    int nValues;
    void setNumValues(int _nValues);
    
    // Reset
    void trash();
    
    // Raw Ingredient
    void addRaw(float _raw); // resets the steps
    float raw;
    vector<float> raws;
    float getRaw();
    
    // norm must always be first
    void normalize();
    float normalizeCalc(float _value, float _min, float _max, float _centerOffset);
    ofParameter<float> min;
    ofParameter<float> max;
    ofParameter<float> centerOffset; // buffer around min for two-sided distributions: should be positive and less than (max-min)
    // ^ SHOULD SEPARATE INTO NORM THEN CLAMP
    
    void sensitize();
    float sensitizeCalc(float _value, float _power);
    ofParameter<float> power;
    
    void invert(); // doesn't actually invert -- bInvert does
    float invertCalc(float _value, bool _bInvert);
    ofParameter<bool> bInvert;
    
    // diff performs absolute value difference if bTwoSided is false
    bool bPrevDiff = false;
    float prevDiff;
    void difference();
    float differenceCalc(float _thisValue, float *_prevValue, bool *_bPrevDiff, bool _bDifference);
    ofParameter<bool> bDifference; // must be true for diffCalc to be executed
    
    bool bPrevAvg = false;
    float prevAvg;
    void average();
    float averageCalc(float _thisValue, float *_prevValue, bool *_bPrevValue, float _easingParam);
    ofParameter<float> easingParam;
    
    void doneCooking(); // signal that cook can be stored
    
    float cook;
    vector<float> cooks;
    float getCook();
    
    // GUI
    ofParameterGroup params;
    
    // helpers
    float map(float param, float minIn, float maxIn, float minOut, float maxOut, bool bClamp);

    // inserts value at beginning of array, capping the length if necessary
    bool bStoreHistory = true;
    void insertNew(vector<float> &array, float value, int maxValues);
    
    
};

#endif /* defined(__foot_tracking_test_3__Ingr__) */
