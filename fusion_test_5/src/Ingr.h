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

enum step {
    NORM,
    SENS,
    INV,
    DIFF,
    AVG,
    CHECK,
    PRIOR
};

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
    
    // Cooking the Ingredient
    vector<step> steps; // steps to cook it
    
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
    
    // priors perform a kind of averaging
    // priors maintain values when there are big, unexpected changes in the data
    // priors allows for more change when values are likely or expected
    // priors depends on defining a beta prior distribution with parameters alpha and beta. The ratio and magnitude of the parameters contributes to the tendancy and strength of the "averaging," respectively.
    bool bPrevPrior = false;
    float prevInputPrior;       // _thisValue (input) from the last iteration
    float prevOutputPrior;      // output from the last iteration
    void prior();
    float priorCalc(float _thisValue, float *_prevInputPrior, float *_prevOutputPrior, bool *_bPrevValue, float _alpha, float _beta, bool _bPrior);   // _thisValue is the input
    ofParameter<float> alpha;   // can't be zero
    ofParameter<float> beta;    // can't be zero
    ofParameter<bool> bPrior;
    
    void doneCooking(); // signal that cook can be stored
    
    void taste(); // store the current state in its own vector
    vector< vector< float > > tastes;
    float getTaste(int index); // gets the first element in a vector
    int tasteCounter = 0;
    
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
    
    void updateFloat(float &value);
    void updateBool(bool &value);
    
    // recalculate all values
    bool bRefresh = true;
    void refresh();
    
    // ----------- DRAWING -----------
    
    // draw cook and all checkpoints if you'd like
    // index = -2 --> raw
    // index = -1 --> cook
    // index = i --> tastes[i]
    // empty vector for all indices
    // margin pads top and bottom (graphs y from [-.5, 1.5]
    void draw(int x, int y, int w, int h, vector <int> indices, bool bMargin, int nRefRegions = -1);
    
    vector<ofColor> preColors = {
        {255, 0, 0},
        {255, 140, 0},
        {255, 235, 0},
        {0, 255, 0},
        {0, 255, 127},
        {0, 255, 255},
        {30, 144, 255},
        {148, 0, 211},
        {255, 20, 147}
    };
    
    ofParameter<bool> bDraw = true;
    
    
};

#endif /* defined(__foot_tracking_test_3__Ingr__) */
