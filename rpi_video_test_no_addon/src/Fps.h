//
//  Fps.h
//  server
//
//  Created by Ben Snell on 8/15/16.
//
//

#ifndef __server__Fps__
#define __server__Fps__

#include "ofMain.h"

class Fps {
    
private:
    
    float fps = 0;
    
    vector<float> times;
    
    float decay = 0.67;
    
public:
    
    void addFrame(); // call new frame
    
    void update();
    
    float getFPS();
    
};

#endif /* defined(__server__Fps__) */
