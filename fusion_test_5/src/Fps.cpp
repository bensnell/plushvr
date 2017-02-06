//
//  Fps.cpp
//  server
//
//  Created by Ben Snell on 8/15/16.
//
//

#include "Fps.h"

// -------------------------------------------------------------------------

void Fps::addFrame() {
    
    times.push_back(ofGetElapsedTimef());
    
}

// -------------------------------------------------------------------------

void Fps::update() {
    
    // calculate video fps
    if (times.size() > 0) {
        while (times.front() < (ofGetElapsedTimef() - 1.) && times.size() > 0) {
            times.erase(times.begin());
        }
    }
    fps = fps * decay + times.size() * (1-decay);
}

// -------------------------------------------------------------------------

float Fps::getFPS() {
    
    return fps;
}

// -------------------------------------------------------------------------