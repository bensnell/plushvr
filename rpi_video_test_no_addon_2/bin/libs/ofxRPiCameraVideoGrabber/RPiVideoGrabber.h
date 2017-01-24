#pragma once 

#include "ofxRPiCameraVideoGrabber.h"


class RPiVideoGrabber: public ofBaseVideoGrabber
{
public:
    RPiVideoGrabber() {
        cameraWidth = 0;
        cameraHeight = 0;
        didInit = false;
        hasNewFrame = true;
    };
    
    bool setup(int w, int h) {
        return initGrabber(w, h);
    };
    bool initGrabber(int w, int h) {
        
        
        omxCameraSettings.width = w;
        omxCameraSettings.height = h;
        omxCameraSettings.enablePixels = true;
        cameraWidth = w;
        cameraHeight = h;
        
        omxCameraSettings.enableTexture = true; //default true
        videoGrabber.setup(omxCameraSettings);
        videoGrabber.enablePixels();
        
        pixels.setFromExternalPixels((unsigned char *)videoGrabber.getPixels(), cameraWidth, cameraHeight, OF_PIXELS_RGBA);
        
        didInit = true;
        return didInit;
    };
    bool isFrameNew() const {
        return hasNewFrame;
    };
    int getFrameRate() {
        return omxCameraSettings.framerate;
    };
    void update() {
        hasNewFrame = videoGrabber.isFrameNew();
    };
    float getHeight() const {
        return cameraHeight;
    };
    float getWidth() const {
        return cameraWidth;
    };
    ofTexture* getTexturePtr() {
        return &videoGrabber.textureEngine->fbo.getTexture();
    };
    vector<ofVideoDevice>	listDevices() const {
        ofLogError(__func__) << "you don't need to call this";
        return unused;
    };
    void setDesiredFrameRate(int framerate) {
        omxCameraSettings.framerate = framerate;
    };
    void close() {
        videoGrabber.close();
    };
    bool setPixelFormat(ofPixelFormat pixelFormat) {
        if (pixelFormat == OF_PIXELS_RGBA)
        {
            return true;
        }
        return false;
    };
    ofPixelFormat getPixelFormat() const {
        return OF_PIXELS_RGBA;
        
    };
    ofPixels&		getPixels() {
        return pixels;
    };
    const ofPixels& getPixels() const {
        return pixels;
    };
    void videoSettings() {
        ofLogError(__func__) << "not used";
    };
    bool isInitialized() const {
        return didInit;
    };
    
    void draw(int x, int y) {
        videoGrabber.getTextureReference().draw(x, y);
    };
    void draw(int x, int y, int width, int height) {
        videoGrabber.getTextureReference().draw(x, y, width, height);
    };
    ofxRPiCameraVideoGrabber videoGrabber;
    
    float cameraWidth;
    float cameraHeight;
    bool didInit;
    ofPixels pixels;
    
    bool hasNewFrame;
    
    OMXCameraSettings omxCameraSettings;
    
    vector<ofVideoDevice> unused;
};