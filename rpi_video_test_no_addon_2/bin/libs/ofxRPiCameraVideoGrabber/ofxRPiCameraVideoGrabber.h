/*
 *  ofxRPiCameraVideoGrabber.h
 *
 *  Created by jason van cleave on 6/1/13.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofAppEGLWindow.h"
#include "OMX_Maps.h"

#include "OMXCameraSettings.h"

#include "TextureEngine.h"
#include "DirectEngine.h"
#include "CameraState.h"


enum MIRROR
{
    MIRROR_NONE=OMX_MirrorNone,
    MIRROR_VERTICAL=OMX_MirrorVertical,
    MIRROR_HORIZONTAL=OMX_MirrorHorizontal,
    MIRROR_BOTH=OMX_MirrorBoth,
};

enum ROTATION
{
    ROTATION_0=0,
    ROTATION_90=90,
    ROTATION_180=180,
    ROTATION_270=270,
};

class ofxRPiCameraVideoGrabber
{

public:
    
    ofxRPiCameraVideoGrabber() {
        OMX_Maps::getInstance();
        resetValues();
        
        int zoomStepsSource[61] =
        {
            65536,  68157,  70124,  72745,
            75366,  77988,  80609,  83231,
            86508,  89784,  92406,  95683,
            99615,  102892, 106168, 110100,
            114033, 117965, 122552, 126484,
            131072, 135660, 140247, 145490,
            150733, 155976, 161219, 167117,
            173015, 178913, 185467, 192020,
            198574, 205783, 212992, 220201,
            228065, 236585, 244449, 252969,
            262144, 271319, 281149, 290980,
            300810, 311951, 322437, 334234,
            346030, 357827, 370934, 384041,
            397148, 411566, 425984, 441057,
            456131, 472515, 488899, 506593,
            524288
        };
        vector<int> converted(zoomStepsSource,
                              zoomStepsSource + sizeof zoomStepsSource / sizeof zoomStepsSource[0]);
        zoomLevels = converted;
        
        
        cropRectangle.set(0,0,100,100);
        
        hasAddedExithandler = false;
        updateFrameCounter = 0;
        frameCounter = 0;
        hasNewFrame = false;
        textureEngine = NULL;
        directEngine = NULL;
        camera = NULL;
        pixelsRequested = false;
        recordingRequested = false;
        ofAddListener(ofEvents().update, this, &ofxRPiCameraVideoGrabber::onUpdate);    
    };
    ~ofxRPiCameraVideoGrabber() {
        cout << "~ofxRPiCameraVideoGrabber" << endl;
        close();
    };
    
    OMXCameraSettings omxCameraSettings;
    //CameraMetering metering;
    
    void setup(OMXCameraSettings omxCameraSettings_) {
        ofRemoveListener(ofEvents().update, this, &ofxRPiCameraVideoGrabber::onUpdate);
        
        OMX_ERRORTYPE error = OMX_ErrorNone;
        error = OMX_Init();
        OMX_TRACE(error);
        
        omxCameraSettings = omxCameraSettings_;
        
        ofLogVerbose(__func__) << "omxCameraSettings: " << omxCameraSettings.toString();
        
        
        
        if(!hasAddedExithandler)
        {
            addExitHandler();
            hasAddedExithandler = true;
        }
        if(directEngine)
        {
            delete directEngine;
            directEngine = NULL;
            camera = NULL;
            ofLogVerbose() << "deleted directEngine";
            resetValues();
        }
        if(textureEngine)
        {
            delete textureEngine;
            textureEngine = NULL;
            camera = NULL;
            ofLogVerbose() << "deleted textureEngine";
            resetValues();
        }
        
        if (omxCameraSettings.enableTexture)
        {
            
            textureEngine = new TextureEngine();
            textureEngine->setup(omxCameraSettings);
            camera = textureEngine->camera;
            if (omxCameraSettings.enablePixels)
            {
                enablePixels();
            }
        }else
        {
            
            directEngine = new DirectEngine();
            directEngine->setup(omxCameraSettings);
            camera = directEngine->camera;
        }
        
        checkBurstMode();
        error = applyExposure(__func__);
        OMX_TRACE(error);
        checkFlickerCancellation();
        applyAllSettings();
        ofAddListener(ofEvents().update, this, &ofxRPiCameraVideoGrabber::onUpdate);    
        
    };
    void setup(CameraState cameraState) {
        setup(cameraState.cameraSettings);
        map<string, string> keyValueMap = cameraState.keyValueMap;
        for(auto iterator  = keyValueMap.begin(); iterator != keyValueMap.end(); iterator++)
        {
            string key = iterator->first;
            string value = iterator->second;
            //ofLogVerbose(__func__) << "key: " << key << " value: " << value;
            
            if(key == "sharpness")  setSharpness(ofToInt(value));
            if(key == "contrast")   setContrast(ofToInt(value));
            if(key == "brightness") setBrightness(ofToInt(value));
            if(key == "saturation") setSaturation(ofToInt(value));
            if(key == "ISO")        setISO(ofToInt(value));
            if(key == "AutoISO")    setAutoISO(ofToBool(value));
            if(key == "DRE")        setDRE(ofToInt(value));
            if(key == "cropRectangle")
            {
                vector<string> rectValues = ofSplitString(value, ",");
                if(rectValues.size() == 4)
                {
                    setSensorCrop(ofToInt(rectValues[0]),
                                  ofToInt(rectValues[1]),
                                  ofToInt(rectValues[2]),
                                  ofToInt(rectValues[3]));
                }
            }
            if(key == "zoomLevelNormalized")    setZoomLevelNormalized(ofToFloat(value));
            if(key == "mirror")                 setMirror(value);
            if(key == "rotation")               setRotation(ofToInt(value));
            if(key == "imageFilter")            setImageFilter(value);
            if(key == "exposurePreset")         setExposurePreset(value);
            if(key == "evCompensation")         setEvCompensation(ofToInt(value));
            if(key == "autoShutter")            setAutoShutter(ofToBool(value));
            if(key == "shutterSpeed")           setShutterSpeed(ofToInt(value));
            if(key == "meteringType")           setMeteringType(value);
            
            if(key == "SoftwareSaturationEnabled") setSoftwareSaturation(ofToBool(value));
            if(key == "SoftwareSharpeningEnabled") setSoftwareSharpening(ofToBool(value));
        }
    };
    int getWidth() {
        return omxCameraSettings.width;
    };
    int getHeight() {
        return omxCameraSettings.height;
    };
    int getFrameRate() {
        return omxCameraSettings.framerate;
    };
    bool isFrameNew() {
        return hasNewFrame;
    };
    bool isTextureEnabled() {
        bool result = false;
        if(textureEngine)
        {
            result = true;
        }else
        {
            result = false;
        }
        return result;
    };
    GLuint getTextureID() {
        if (!textureEngine)
        {
            ofLogError(__func__) << "TEXTURE NOT ENABLED - RETURN UNALLOCATED TEXTURE ID";
            return errorTexture.texData.textureID;
        }
        return textureEngine->getTexture().texData.textureID;
    };
    ofTexture& getTextureReference() {
        if (!textureEngine)
        {
            ofLogError(__func__) << "TEXTURE NOT ENABLED - RETURN UNALLOCATED TEXTURE";
            return errorTexture;
        }
        return textureEngine->getTexture();
    };
    
    bool recordingRequested;
    bool isRecording() {
        
        if (!directEngine && !textureEngine)
        {
            return false;
        }
        
        bool result = false;
        if (directEngine)
        {
            result = directEngine->isThreadRunning();
        }
        if (textureEngine)
        {
            result =  textureEngine->isThreadRunning();
        }
        return result;
        
        
    };
    void startRecording() {
        if (!directEngine && !textureEngine)
        {
            return;
        }
        bool isCurrentlyRecording = isRecording();
        ofLogVerbose(__func__) << "isCurrentlyRecording: " << isCurrentlyRecording;
        if(!isCurrentlyRecording)
        {
            recordingRequested = true;
        }
        
        
    };
    void stopRecording() {
        if (directEngine)
        {
            directEngine->stopRecording();
            /*
             
             direct mode has to use a lower resolution for display while recording
             set it back after recording
             */
            CameraState currentState = getCameraState();
            currentState.cameraSettings.doRecording = false;
            setup(currentState);
        }
        if (textureEngine) 
        {
            textureEngine->stopRecording();
        }
    };
    void enablePixels() {
        if(textureEngine)
        {
            textureEngine->enablePixels();
            pixelsRequested = true;
        }
    };
    void disablePixels() {
        if(textureEngine)
        {
            textureEngine->disablePixels();
            pixelsRequested = false;
        }
    };
    unsigned char * getPixels() {
        unsigned char * pixels = NULL;
        if (textureEngine) {
            pixels = textureEngine->getPixels();
        }
        return pixels;
    };
    
    TextureEngine* textureEngine;
    DirectEngine* directEngine;
    
    bool isReady() {
        
        if (directEngine)
        {
            return directEngine->isOpen;
        }
        
        if (textureEngine)
        {
            return textureEngine->isOpen;
        }
        
        return false;
    };
    
    void draw() {
        if (textureEngine)
        {
            textureEngine->getTexture().draw(0, 0);
            return;
        }
        if (directEngine)
        {
            setDisplayDrawRectangle(ofRectangle(0, 0, getWidth(), getHeight()));
        }
        
        
    };
    void draw(int x, int y) {
        if (textureEngine)
        {
            textureEngine->getTexture().draw(x, y);
            return;
        }
        
        if (directEngine)
        {
            setDisplayDrawRectangle(ofRectangle(x, y, getWidth(), getHeight()));
        }
    };
    void draw(int x, int y, int width, int height) {
        if (textureEngine)
        {
            textureEngine->getTexture().draw(x, y, width, height);
            return;
        }
        
        
        if (directEngine)
        {
            setDisplayDrawRectangle(ofRectangle(x, y, width, height));
        }
    };
    
    void reset() {
        resetValues();
        applyAllSettings();
    };
    void close() {
        
        cout << "ofxRPiCameraVideoGrabber::close" << endl;
        ofRemoveListener(ofEvents().update, this, &ofxRPiCameraVideoGrabber::onUpdate);
        if(directEngine)
        {
            cout << "~ofxRPiCameraVideoGrabber delete directEngine" << endl;
            delete directEngine;
            directEngine = NULL;
        }
        if(textureEngine)
        {
            delete textureEngine;
            textureEngine = NULL;
        }
        
        cout << "~ofxRPiCameraVideoGrabber::close END" << endl;
    };
    
    void setSharpness(int sharpness_) {
        sharpness = sharpness_;
        
        OMX_ERRORTYPE error = OMX_ErrorNone;
        sharpnessConfig.nSharpness = sharpness;
        
        error = OMX_SetConfig(camera, OMX_IndexConfigCommonSharpness, &sharpnessConfig);
        OMX_TRACE(error);
        
    };
    int getSharpness() { return sharpness; }
    
    void setContrast(int contrast_) {
        contrast = contrast_;
        
        OMX_ERRORTYPE error = OMX_ErrorNone;
        contrastConfig.nContrast = contrast;
        
        error = OMX_SetConfig(camera, OMX_IndexConfigCommonContrast, &contrastConfig);
        OMX_TRACE(error);
        
    };
    int getContrast() { return contrast; }
    
    void setBrightness(int brightness_) {
        brightness = brightness_;
        
        OMX_ERRORTYPE error = OMX_ErrorNone;
        brightnessConfig.nBrightness = brightness;
        
        error = OMX_SetConfig(camera, OMX_IndexConfigCommonBrightness, &brightnessConfig);
        OMX_TRACE(error);
        
    };
    int getBrightness() { return brightness; }
    
    void setSaturation(int saturation_) {
        saturation = saturation_;
        
        OMX_ERRORTYPE error = OMX_ErrorNone;
        saturationConfig.nSaturation	= saturation_;
        
        error = OMX_SetConfig(camera, OMX_IndexConfigCommonSaturation, &saturationConfig);
        OMX_TRACE(error);
        
    };
	int getSaturation()	{ return saturation; }
    
    bool frameStabilization;
    void setFrameStabilization(bool doStabilization) {
        OMX_ERRORTYPE error = OMX_ErrorNone;
        
        
        if (doStabilization)
        {
            framestabilizationConfig.bStab = OMX_TRUE;
        }else
        {
            framestabilizationConfig.bStab = OMX_FALSE;
        }
        
        error = OMX_SetConfig(camera, OMX_IndexConfigCommonFrameStabilisation, &framestabilizationConfig);
        if(error == OMX_ErrorNone)
        {
            frameStabilization = doStabilization;
        }
        
        OMX_TRACE(error);
        
    };
    bool getFrameStabilization(){ return frameStabilization;}
	
    OMX_ERRORTYPE setColorEnhancement(bool doColorEnhance, int U=128, int V=128) {
        
        colorEnhancementConfig.bColorEnhancement = toOMXBool(doColorEnhance);
        colorEnhancementConfig.nCustomizedU = U;
        colorEnhancementConfig.nCustomizedV = V;
        OMX_ERRORTYPE error = OMX_SetConfig(camera, OMX_IndexConfigCommonColorEnhancement, &colorEnhancementConfig);
        OMX_TRACE(error);
        return error;
        
    };
	
    
    void setDRE(int level) {
        OMX_DYNAMICRANGEEXPANSIONMODETYPE type = OMX_DynRangeExpOff;
        switch (level)
        {
            case 0:
            {
                type = OMX_DynRangeExpOff;
                break;
            }
            case 1:
            {
                type = OMX_DynRangeExpLow;
                break;
            }
            case 2:
            {
                type = OMX_DynRangeExpMedium;
                break;
            }
            case 3:
            {
                type = OMX_DynRangeExpHigh;
                break;
            }
            default:
            {
                type = OMX_DynRangeExpOff;
                break;
            }
                
        }
        OMX_CONFIG_DYNAMICRANGEEXPANSIONTYPE dreConfig;
        OMX_INIT_STRUCTURE(dreConfig);
        dreConfig.eMode = type;
        OMX_ERRORTYPE error = OMX_SetConfig(camera, OMX_IndexConfigDynamicRangeExpansion, &dreConfig);
        OMX_TRACE(error);
        
        if(error == OMX_ErrorNone)
        {
            dreLevel = level;
        }
    };
    int getDRE() { return dreLevel; }
	
    void toggleLED() {
        setLEDState(!LED);
    };
    void setLEDState(bool stateRequested) {
        if (!hasGPIOProgram)
        {
            return;
        }
        stringstream command;
        command << "gpio -g write ";
        command << LED_PIN;
        if (stateRequested)
        {
            command <<  " 1";
        }else
        {
            command <<  " 0";
        }
        
        
        int result = system(command.str().c_str());
        if(result == 0)
        {
            LED = stateRequested;
        }
    };
    bool getLEDState() { return LED; }
    
    OMX_ERRORTYPE setFlickerCancellation(OMX_COMMONFLICKERCANCELTYPE eFlickerCancel) {
        
        OMX_ERRORTYPE error = OMX_GetConfig(camera, OMX_IndexConfigCommonFlickerCancellation, &flickerCancelConfig);
        OMX_TRACE(error);
        if(error == OMX_ErrorNone)
        {
            switch (flickerCancelConfig.eFlickerCancel)
            {
                case OMX_COMMONFLICKERCANCEL_OFF:
                {
                    ofLogVerbose(__func__) << "OMX_COMMONFLICKERCANCEL_OFF";
                    break;
                }
                case OMX_COMMONFLICKERCANCEL_AUTO:
                {
                    ofLogVerbose(__func__) << "OMX_COMMONFLICKERCANCEL_AUTO";
                    break;
                }
                case OMX_COMMONFLICKERCANCEL_50:
                {
                    ofLogVerbose(__func__) << "OMX_COMMONFLICKERCANCEL_50";
                    break;
                }
                case OMX_COMMONFLICKERCANCEL_60:
                {
                    ofLogVerbose(__func__) << "OMX_COMMONFLICKERCANCEL_60";
                    break;
                }
                default:
                    break;
            }
            flickerCancelConfig.eFlickerCancel = eFlickerCancel;
            error = OMX_SetConfig(camera, OMX_IndexConfigCommonFlickerCancellation, &flickerCancelConfig);
            if(error == OMX_ErrorNone)
            {
                if(flickerCancelConfig.eFlickerCancel == OMX_COMMONFLICKERCANCEL_OFF)
                {
                    flickerCancellation = false;
                }else
                {
                    flickerCancellation = true;
                }
            }else
            {
                //error so assume it's not working
                flickerCancellation = false;
            }
            OMX_TRACE(error);
        }
        else
        {
            //error so assume it's not working
            flickerCancellation = false;
        }
        
        return error;
    };
    void setFlickerCancellation(bool enable) {
        if(enable)
        {
            setFlickerCancellation(OMX_COMMONFLICKERCANCEL_AUTO);
        }else
        {
            setFlickerCancellation(OMX_COMMONFLICKERCANCEL_OFF);
        }
        
    };
    void enableFlickerCancellation() {
        setFlickerCancellation(true);
    };
    void disableFlickerCancellation() {
        setFlickerCancellation(false);
    };
    bool isFlickerCancellationEnabled() { return flickerCancellation; }
    //TODO: enable explict 50/60 hz
    
    
    OMX_ERRORTYPE setSensorCrop(ofRectangle& rectangle) {
        
        
        sensorCropConfig.xLeft   = ((uint32_t)rectangle.getLeft()   << 16)/100;
        sensorCropConfig.xTop    = ((uint32_t)rectangle.getTop()    << 16)/100;
        sensorCropConfig.xWidth  = ((uint32_t)rectangle.getWidth()  << 16)/100;
        sensorCropConfig.xHeight = ((uint32_t)rectangle.getHeight() << 16)/100;
        
        OMX_ERRORTYPE error = OMX_SetConfig(camera, OMX_IndexConfigInputCropPercentages, &sensorCropConfig);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone)
        {
            ofLogError(__func__) << omxErrorToString(error);
            if(error == OMX_ErrorBadParameter)
            {
                ofLogWarning(__func__) << "resetting cropRectangle to known good params (0, 0, 100, 100)";
                cropRectangle.set(0, 0, 100, 100);
                return updateSensorCrop();
            }
            
        }
        return error;
        
    };
    ofRectangle& getCropRectangle() { return cropRectangle; }
    OMX_ERRORTYPE setSensorCrop(int left, int top, int width, int height) {
        cropRectangle.set(left, top, width, height);
        return updateSensorCrop();
    };
    
    vector<int> zoomLevels;
    OMX_ERRORTYPE setDigitalZoom() {
        
        if(zoomLevel<0 || (unsigned int) zoomLevel>zoomLevels.size())
        {
            
            ofLogError(__func__) << "BAD zoomLevel: " << zoomLevel << " SETTING TO 0" << " zoomLevels.size(): " << zoomLevels.size();
            zoomLevel = 0;
        }
        
        
        int value = zoomLevels[zoomLevel];
        if(digitalZoomConfig.xWidth != value && digitalZoomConfig.xHeight != value)
        {
            digitalZoomConfig.xWidth  = value;
            digitalZoomConfig.xHeight = value;
            
            OMX_ERRORTYPE error = OMX_SetConfig(camera, OMX_IndexConfigCommonDigitalZoom, &digitalZoomConfig);
            OMX_TRACE(error);
            return error;
        }
        return OMX_ErrorNone;
        
    };
    OMX_ERRORTYPE zoomIn() {
        if((unsigned int)zoomLevel+1 < zoomLevels.size())
        {
            zoomLevel++;
        }
        return setDigitalZoom();
    };
    OMX_ERRORTYPE zoomOut() {
        zoomLevel--;
        if(zoomLevel<0)
        {
            zoomLevel = 0;
        }
        
        return setDigitalZoom();
    };
    OMX_ERRORTYPE resetZoom() {
        zoomLevel = 0;
        return setDigitalZoom();
    };
    OMX_ERRORTYPE setZoomLevelNormalized(float value) {
        if(value<0 || value>1)
        {
            ofLogError(__func__) << value << "MUST BE BETWEEN 0.0 - 1.0";
            return OMX_ErrorBadParameter;
        }
        zoomLevel = (int) ofMap(value, 0.0f, 1.0f, 0, zoomLevels.size());
        return setDigitalZoom();
    };
    float getZoomLevelNormalized() {
        return ofMap(zoomLevel, 0, zoomLevels.size(), 0.0f, 1.0f);
    };
    
    OMX_ERRORTYPE setMirror(int mirrorType) {
        mirrorConfig.eMirror = (OMX_MIRRORTYPE)mirrorType;
        return applyMirror();
    };
    OMX_ERRORTYPE setMirror(string mirror_) {
        return setMirror(GetMirror(mirror_));
    };
    string getMirror() {
        
        return GetMirrorString(mirrorConfig.eMirror);
    };
    
    OMX_ERRORTYPE setRotation(int value) {
        return setRotation((ROTATION) value);
    };
    OMX_ERRORTYPE setRotation(ROTATION value) {
        switch (value)
        {
            case ROTATION_0: {rotationConfig.nRotation=0;} break;
            case ROTATION_90: {rotationConfig.nRotation=90;} break;
            case ROTATION_180: {rotationConfig.nRotation=180;} break;
            case ROTATION_270: {rotationConfig.nRotation=270;} break;
            default: {rotationConfig.nRotation=0;} break;
        }
        return applyRotation();
    };
    OMX_ERRORTYPE rotateClockwise() {
        int currentRotation  = getRotation();
        if(currentRotation+90<360)
        {
            rotationConfig.nRotation+=90;
        }else{
            rotationConfig.nRotation = 0;
        }
        
        return applyRotation();
    };
    OMX_ERRORTYPE rotateCounterClockwise() {
        int currentRotation  = getRotation();
        if(currentRotation-90>=0)
        {
            rotationConfig.nRotation-=90;
        }else{
            rotationConfig.nRotation = 270;
        }
        
        return applyRotation();
    };
    int getRotation() {
        return rotationConfig.nRotation;
    };

    OMX_ERRORTYPE setImageFilter(OMX_IMAGEFILTERTYPE imageFilter_) {
        
        imagefilterConfig.eImageFilter = imageFilter_;
        OMX_ERRORTYPE error = OMX_SetConfig(camera, OMX_IndexConfigCommonImageFilter, &imagefilterConfig);
        OMX_TRACE(error);
        if(error == OMX_ErrorNone)
        {
            imageFilter = GetImageFilterString(imageFilter_);
        }
        return error;
    };
    OMX_ERRORTYPE setImageFilter(string imageFilter_) {
        return setImageFilter(GetImageFilter(imageFilter_));
    };
    string getImageFilter() {
        return GetImageFilterString(imagefilterConfig.eImageFilter);
    };

    OMX_ERRORTYPE setExposurePreset(OMX_EXPOSURECONTROLTYPE exposurePreset_) {
        exposurePresetConfig.eExposureControl = exposurePreset_;
        
        OMX_ERRORTYPE error = OMX_SetConfig(camera, OMX_IndexConfigCommonExposure, &exposurePresetConfig);
        OMX_TRACE(error);
        if(error == OMX_ErrorNone)
        {
            exposurePreset = GetExposurePresetString(exposurePreset_);
        }
        return error;
    };
    OMX_ERRORTYPE setExposurePreset(string exposurePreset_) {
        
        return setExposurePreset(GetExposurePreset(exposurePreset_));
    };
    string getExposurePreset() {
        return GetExposurePresetString(exposurePresetConfig.eExposureControl);
    };

    OMX_ERRORTYPE setEvCompensation(int value) {
        if ((value >= -4) && (value <= 4))
        {
            exposureConfig.xEVCompensation = toQ16(value);
        }
        OMX_ERRORTYPE error =  applyExposure(__func__);
        if(error == OMX_ErrorNone)
        {
            evCompensation = getEvCompensation();
        }
        return error;
    }; //-4 to 4
    int getEvCompensation() {
        return fromQ16(exposureConfig.xEVCompensation);
    };

    OMX_ERRORTYPE setWhiteBalance(OMX_WHITEBALCONTROLTYPE whiteBalance_) {
        whiteBalanceConfig.eWhiteBalControl = whiteBalance_;
        
        OMX_ERRORTYPE error = OMX_SetConfig(camera, OMX_IndexConfigCommonWhiteBalance, &whiteBalanceConfig);
        OMX_TRACE(error);
        if(error == OMX_ErrorNone)
        {
            whiteBalance = whiteBalance_;
        }
        return error;
    };
    OMX_ERRORTYPE setWhiteBalance(string name) {
        
        return setWhiteBalance(GetWhiteBalance(name));
    };
    string getWhiteBalance() {
        return GetWhiteBalanceString(whiteBalanceConfig.eWhiteBalControl);
    };

    OMX_ERRORTYPE setAutoShutter(bool doAutoShutter) {
        OMX_ERRORTYPE error = OMX_GetConfig(camera, OMX_IndexConfigCommonExposureValue, &exposureConfig);
        OMX_TRACE(error);
        if(error == OMX_ErrorNone)
        {
            
            if(doAutoShutter)
            {
                exposureConfig.bAutoShutterSpeed = OMX_TRUE;
            }else
            {
                exposureConfig.bAutoShutterSpeed = OMX_FALSE;
            }
            error =  applyExposure(__func__);
            
            if(error == OMX_ErrorNone)
            {
                autoShutter = getAutoShutter();
            }
        }
        return error;
    };
    bool getAutoShutter() {
        return fromOMXBool(exposureConfig.bAutoShutterSpeed);
    };
    int getShutterSpeed() {
        return exposureConfig.nShutterSpeedMsec;
    };
    
    OMX_ERRORTYPE setShutterSpeed(int shutterSpeedMicroSeconds_) {
        
        OMX_ERRORTYPE error = OMX_GetConfig(camera, OMX_IndexConfigCommonExposureValue, &exposureConfig);
        OMX_TRACE(error);
        exposureConfig.nShutterSpeedMsec = shutterSpeedMicroSeconds_;
        error =  applyExposure(__func__);
        OMX_TRACE(error);
        if(error == OMX_ErrorNone)
        {
            shutterSpeed = getShutterSpeed();
        }
        ofLogVerbose(__func__) << "POST getShutterSpeed(): " << getShutterSpeed();
        return error;
        
    };
    
    OMX_ERRORTYPE setMeteringType(OMX_METERINGTYPE meteringType_) {
        exposureConfig.eMetering = meteringType_;
        OMX_ERRORTYPE error = applyExposure(__func__);
        if(error == OMX_ErrorNone)
        {
            meteringType     = exposureConfig.eMetering;
        }
        
        return error;
    };
    OMX_ERRORTYPE setMeteringType(string meteringType_) {
        return setMeteringType(GetMetering(meteringType_));
    };
    string getMeteringType() {
        return GetMeteringString(exposureConfig.eMetering);
    };

    bool doDisableSoftwareSharpen;
    bool doDisableSoftwareSaturation;
    OMX_ERRORTYPE setSoftwareSharpening(bool state) {
        OMX_ERRORTYPE error = OMX_GetConfig(camera, OMX_IndexParamSWSharpenDisable, &disableSoftwareSharpenConfig);
        OMX_TRACE(error);
        if(error == OMX_ErrorNone)
        {
            disableSoftwareSharpenConfig.bEnabled = toOMXBool(state);
            
            error = OMX_SetConfig(camera, OMX_IndexParamSWSharpenDisable, &disableSoftwareSharpenConfig);
            OMX_TRACE(error);
            if(error == OMX_ErrorNone)
            {
                doDisableSoftwareSharpen = state;
                ofLogVerbose() << "doDisableSoftwareSharpen: " << doDisableSoftwareSharpen;
            }
        }
        return error;
    };
    OMX_ERRORTYPE enableSoftwareSharpening() {
        return setSoftwareSharpening(false);
    };
    OMX_ERRORTYPE disableSoftwareSharpening() {
        
        return setSoftwareSharpening(true);
    };
    bool isSoftwareSharpeningEnabled() {return doDisableSoftwareSharpen;}
    
    OMX_ERRORTYPE setSoftwareSaturation(bool state) {
        OMX_ERRORTYPE error = OMX_GetConfig(camera, OMX_IndexParamSWSaturationDisable, &disableSoftwareSaturationConfig);
        OMX_TRACE(error);
        if(error == OMX_ErrorNone)
        {
            disableSoftwareSaturationConfig.bEnabled = toOMXBool(state);
            
            error = OMX_SetConfig(camera, OMX_IndexParamSWSaturationDisable, &disableSoftwareSaturationConfig);
            OMX_TRACE(error);
            if(error == OMX_ErrorNone)
            {
                doDisableSoftwareSaturation = state;
                ofLogVerbose() << "doDisableSoftwareSaturation: " << doDisableSoftwareSaturation;
                
            }
        }
        
        return error;
    };
    OMX_ERRORTYPE enableSoftwareSaturation() {
        return setSoftwareSaturation(false);
    };
    OMX_ERRORTYPE disableSoftwareSaturation() {
        
        return setSoftwareSaturation(true);
    };
    bool isSoftwareSaturationEnabled() {return doDisableSoftwareSaturation;}
    
    void setBurstMode(bool doBurstMode) {
        OMX_ERRORTYPE error = OMX_GetConfig(camera, OMX_IndexConfigBurstCapture, &burstModeConfig);
        OMX_TRACE(error);
        if(doBurstMode)
        {
            burstModeConfig.bEnabled = OMX_TRUE;
        }else
        {
            burstModeConfig.bEnabled = OMX_FALSE;
            
        }
        error = OMX_SetConfig(camera, OMX_IndexConfigBurstCapture, &burstModeConfig);
        OMX_TRACE(error);
        if(error == OMX_ErrorNone)
        {
            burstModeEnabled = doBurstMode;
        }
        ofLogVerbose() << "burstModeEnabled: " << burstModeEnabled;
    };
    bool isBurstModeEnabled(){ return burstModeEnabled;}
    
    OMX_ERRORTYPE setISO(int ISO_) {
        OMX_ERRORTYPE error = OMX_GetConfig(camera, OMX_IndexConfigCommonExposureValue, &exposureConfig);
        OMX_TRACE(error);
        if(error == OMX_ErrorNone)
        {
            exposureConfig.nSensitivity	= ISO_;
            error =  applyExposure(__func__);
            
            if(error == OMX_ErrorNone)
            {
                ISO = getISO();
            }
        }
        return error;
    };
    int getISO() {
        return exposureConfig.nSensitivity;
    };
    
    OMX_ERRORTYPE setAutoISO(bool doAutoISO) {
        OMX_ERRORTYPE error = OMX_GetConfig(camera, OMX_IndexConfigCommonExposureValue, &exposureConfig);
        OMX_TRACE(error);
        if(error == OMX_ErrorNone)
        {
            if(doAutoISO)
            {
                exposureConfig.bAutoSensitivity = OMX_TRUE;
            }else
            {
                exposureConfig.bAutoSensitivity = OMX_FALSE;
            }
            error = applyExposure(__func__);
            OMX_TRACE(error);
            
        }
        return error;
    };
    bool getAutoISO() {
        return fromOMXBool(exposureConfig.bAutoSensitivity);
    };
    
    OMX_CONFIG_EXPOSUREVALUETYPE exposureConfig;
    
    
    string printExposure() {
        
        
        stringstream ss;
        ss << "meteringType: " << OMX_Maps::getInstance().getMetering(exposureConfig.eMetering) << "\n";
        ss << "evCompensation: " << getEvCompensation() << "\n";
        
        ss << "autoShutter: " << getAutoShutter() << "\n";
        ss << "shutterSpeedMicroSeconds: " << getShutterSpeed() << "\n";
        
        ss << "autoISO: " << getAutoISO() << "\n";
        ss << "ISO: " << getISO() << "\n";
        return ss.str();
        
    };

    string currentStateToString() {
        stringstream info;
        info << "sharpness " << getSharpness() << endl;
        info << "contrast " << getContrast() << endl;
        info << "brightness " << getBrightness() << endl;
        info << "saturation " << getSaturation() << endl;
        
        info << "ISO " << getISO() << endl;
        info << "AutoISO " << getAutoISO() << endl;
        
        info << "DRE " << getDRE() << endl;
        info << "cropRectangle " << getCropRectangle() << endl;
        info << "zoomLevelNormalized " << getZoomLevelNormalized() << endl;
        info << "mirror " << getMirror() << endl;
        info << "rotation " << getRotation() << endl;
        info << "imageFilter " << getImageFilter() << endl;
        info << "exposurePreset " << getExposurePreset() << endl;
        info << "evCompensation " << getEvCompensation() << endl;
        info << "autoShutter " << getAutoShutter() << endl;
        info << "shutterSpeed " << getShutterSpeed() << endl;
        info << "meteringType " << getMeteringType() << endl;
        info << "SoftwareSaturationEnabled " << isSoftwareSaturationEnabled() << endl;
        info << "SoftwareSharpeningEnabled " << isSoftwareSharpeningEnabled() << endl;
        
        //OMXCameraSettings
        info << omxCameraSettings.toString() << endl;
        return info.str();
    };
    void saveStateToFile(string fileName="") {
        ofBuffer buffer(currentStateToString());
        if(fileName.empty())
        {
            fileName = "STATE_"+ofGetTimestampString()+".txt";
        }
        ofBufferToFile(fileName, buffer);
    };
    
    CameraState getCameraState() {
        CameraState cameraState;
        cameraState.setup(currentStateToString());
        return cameraState;
    };
    DirectDisplay* getDisplayManager() {
        if(directEngine)
        {
            return &directEngine->displayManager;
        }
        
        return NULL;
    };

    void setDisplayAlpha(int alpha) {
        if(directEngine)
        {
            getDisplayManager()->options.alpha = alpha;
        }
    };
    void setDisplayRotation(int rotationDegrees) {
        if(directEngine)
        {
            getDisplayManager()->rotateDisplay(rotationDegrees);
        }
    };
    void setDisplayDrawRectangle(ofRectangle drawRectangle) {
        if(directEngine)
        {
            getDisplayManager()->options.drawRectangle = drawRectangle;
            
        }
    };
    void setDisplayCropRectangle(ofRectangle cropRectangle) {
        if(directEngine)
        {
            getDisplayManager()->options.cropRectangle = cropRectangle;
            
        }
    };
    void setDisplayMirror(bool doMirror) {
        if(directEngine)
        {
            getDisplayManager()->options.doMirror = doMirror;
        }
    };

private:
    ofTexture errorTexture;
    OMX_HANDLETYPE camera;
    bool hasNewFrame;
    
    OMX_METERINGTYPE meteringType;
    bool autoISO;
    int ISO;
    bool autoShutter;
    int shutterSpeed;
    int sharpness;	//	-100 to 100
    int contrast;	//  -100 to 100 
    int brightness; //     0 to 100
    int saturation; //  -100 to 100 
    int dreLevel; 
    ofRectangle cropRectangle;
    int zoomLevel;
    string mirror; 
    int rotation;
    string imageFilter;
    string exposurePreset;
    int evCompensation;
    string whiteBalance;
    
    bool LED;
    string LED_PIN;
    string getLEDPin() {
        //default as RPI1 GPIO Layout
        string result = "5";
        if(hasGPIOProgram)
        {
            string command = "gpio -v";
            FILE* myPipe = popen(command.c_str(), "r");
            char buffer[128];
            string commandOutput = "";
            while (!feof(myPipe))
            {
                if (fgets(buffer, 128, myPipe) != NULL)
                {
                    commandOutput += buffer;
                }
            }
            
            pclose (myPipe);
            //ofLogVerbose(__func__) << "result: " << result;
            if(!commandOutput.empty())
            {
                vector<string> contents = ofSplitString(commandOutput, "Type:");
                string secondLine = contents[1];
                //ofLogVerbose(__func__) << "secondLine: " << secondLine;
                vector<string> secondLineContents = ofSplitString(secondLine, ",");
                string modelValue = secondLineContents[0];
                ofLogVerbose(__func__) << "modelValue: " << modelValue;
                //assuming RPI2 and RPI3 GPIO layout is the same
                //TODO: check RPI3
                if(ofIsStringInString(modelValue, "2") || ofIsStringInString(modelValue, "3"))
                {
                    result = "32";
                }
            }
        }
        
        return result;
        
    };
    bool hasGPIOProgram;
    int updateFrameCounter;
    int frameCounter;
    bool pixelsRequested;
    bool burstModeEnabled;
    bool flickerCancellation;

    bool hasAddedExithandler;
    void addExitHandler() {
        
        vector<int> signals;
        signals.push_back(SIGINT);
        signals.push_back(SIGQUIT);
        
        for (size_t i=0; i<signals.size(); i++)
        {
            int SIGNAL_TO_BLOCK = signals[i];
            //http://stackoverflow.com/questions/11465148/using-sigaction-c-cpp
            
            //Struct for the new action associated to the SIGNAL_TO_BLOCK
            struct sigaction new_action;
            new_action.sa_handler = signal_handler;
            
            //Empty the sa_mask. This means that no signal is blocked while the signal_handler runs.
            sigemptyset(&new_action.sa_mask);
            
            //Block the SEGTERM signal so while the signal_handler runs, the SIGTERM signal is ignored
            sigaddset(&new_action.sa_mask, SIGTERM);
            
            //Remove any flag from sa_flag. See documentation for flags allowed
            new_action.sa_flags = 0;
            
            struct sigaction old_action;
            //Read the old signal associated to SIGNAL_TO_BLOCK
            sigaction(SIGNAL_TO_BLOCK, NULL, &old_action);
            
            //If the old handler wasn't SIG_IGN it is a handler that just "ignores" the signal
            if (old_action.sa_handler != SIG_IGN)
            {
                //Replace the signal handler of SIGNAL_TO_BLOCK with the one described by new_action
                sigaction(SIGNAL_TO_BLOCK, &new_action, NULL);
            }
            
        }
        
        ofAddListener(ofEvents().update, this, &ofxRPiCameraVideoGrabber::onUpdateDuringExit);
    };
    
    void checkBurstMode() {
        OMX_ERRORTYPE error = OMX_GetConfig(camera, OMX_IndexConfigBurstCapture, &burstModeConfig);
        OMX_TRACE(error);
        burstModeEnabled = fromOMXBool(burstModeConfig.bEnabled);
    };
    void checkFlickerCancellation() {
        OMX_ERRORTYPE error = OMX_GetConfig(camera, OMX_IndexConfigCommonFlickerCancellation, &flickerCancelConfig);
        OMX_TRACE(error);
        if(error == OMX_ErrorNone)
        {
            switch (flickerCancelConfig.eFlickerCancel)
            {
                case OMX_COMMONFLICKERCANCEL_OFF:
                {
                    ofLogVerbose(__func__) << "OMX_COMMONFLICKERCANCEL_OFF";
                    break;
                }
                case OMX_COMMONFLICKERCANCEL_AUTO:
                {
                    ofLogVerbose(__func__) << "OMX_COMMONFLICKERCANCEL_AUTO";
                    break;
                }
                case OMX_COMMONFLICKERCANCEL_50:
                {
                    ofLogVerbose(__func__) << "OMX_COMMONFLICKERCANCEL_50";
                    break;
                }
                case OMX_COMMONFLICKERCANCEL_60:
                {
                    ofLogVerbose(__func__) << "OMX_COMMONFLICKERCANCEL_60";
                    break;
                }
                default:
                    break;
            }
            if(flickerCancelConfig.eFlickerCancel == OMX_COMMONFLICKERCANCEL_OFF)
            {
                flickerCancellation = false;
            }else
            {
                flickerCancellation = true;
            }
        }
        else
        {
            //error so assume it's not working
            flickerCancellation = false;
        }
    };
    OMX_ERRORTYPE applyMirror() {
        OMX_ERRORTYPE error = OMX_SetConfig(camera, OMX_IndexConfigCommonMirror, &mirrorConfig);
        OMX_TRACE(error);
        if(error == OMX_ErrorNone)
        {
            mirror = GetMirrorString(mirrorConfig.eMirror);
        }
        return error;
    };
    void applyImageFilter(OMX_IMAGEFILTERTYPE imageFilter) {
        
        imagefilterConfig.eImageFilter = imageFilter;
        
        OMX_ERRORTYPE error = OMX_SetConfig(camera, OMX_IndexConfigCommonImageFilter, &imagefilterConfig);
        OMX_TRACE(error);
        
    };
    OMX_ERRORTYPE applyRotation() {
        
        OMX_ERRORTYPE error = OMX_SetConfig(camera, OMX_IndexConfigCommonRotate, &rotationConfig);
        OMX_TRACE(error);
        if(error == OMX_ErrorNone)
        {
            rotation = getRotation();
        }
        return error;
    };
    OMX_ERRORTYPE applyCurrentMeteringMode();
    OMX_ERRORTYPE applyExposure(string caller="UNDEFINED") {
        OMX_ERRORTYPE error = OMX_ErrorNone;
        error = OMX_SetConfig(camera, OMX_IndexConfigCommonExposureValue, &exposureConfig);
        OMX_TRACE(error);
#if 0
        stringstream info;
        info << "eMetering: " << exposureConfig.eMetering << endl;
        info << "bAutoShutterSpeed: " << exposureConfig.xEVCompensation << endl;
        info << "xEVCompensation: " << exposureConfig.xEVCompensation << endl;
        info << "nShutterSpeedMsec: " << exposureConfig.nShutterSpeedMsec << endl;
        info << "bAutoSensitivity: " << exposureConfig.bAutoSensitivity << endl;
        info << "nSensitivity: " << exposureConfig.nSensitivity << endl;
        info << endl;
        info << "eMetering: " << GetMeteringString(exposureConfig.eMetering) << endl;
        info << "autoShutter: " << autoShutter << endl;
        info << "shutterSpeed: " << shutterSpeed << endl;
        info << "autoISO: " << autoISO << endl;
        info << "ISO: " << ISO << endl;
        ofLogVerbose(__func__) << " caller: " << caller;
        ofLogVerbose(__func__) << " info: " << info.str();
#endif
        /*if (error == OMX_ErrorNone)
         {
         meteringType     = exposureConfig.eMetering;
         evCompensation   = fromQ16(exposureConfig.xEVCompensation);
         
         autoShutter      = fromOMXBool(exposureConfig.bAutoShutterSpeed);
         shutterSpeed     = exposureConfig.nShutterSpeedMsec;
         
         autoISO          = fromOMXBool(exposureConfig.bAutoSensitivity);
         ISO              = exposureConfig.nSensitivity;
         }*/
        //ofLogVerbose(__func__) << printExposure();
        
        return error;
    };
    OMX_ERRORTYPE updateSensorCrop() {
        return setSensorCrop(cropRectangle);
    };
    
    void resetValues() {
        ofLogVerbose(__func__) << endl;
        exposurePreset = "Auto";
        
        meteringType = OMX_MeteringModeAverage;
        autoISO = true;
        ISO = 0;
        autoShutter = true;
        shutterSpeed = 0;
        
        sharpness=-50;
        contrast=-10;
        brightness=50;
        saturation=0;
        frameStabilization=false;
        flickerCancellation = false;
        whiteBalance="Auto";
        imageFilter="None";
        dreLevel=0;
        cropRectangle.set(0,0,100,100);
        zoomLevel=0;
        rotation=0;
        mirror="MIRROR_NONE";
        doDisableSoftwareSharpen = false;
        doDisableSoftwareSaturation = false;
        LED = true;
        
        OMX_INIT_STRUCTURE(exposurePresetConfig);
        exposurePresetConfig.nPortIndex = OMX_ALL;
        
        OMX_INIT_STRUCTURE(sharpnessConfig);
        sharpnessConfig.nPortIndex = OMX_ALL;
        
        OMX_INIT_STRUCTURE(framestabilizationConfig);
        framestabilizationConfig.nPortIndex = OMX_ALL;
        
        OMX_INIT_STRUCTURE(contrastConfig);
        contrastConfig.nPortIndex = OMX_ALL;
        
        OMX_INIT_STRUCTURE(brightnessConfig);
        brightnessConfig.nPortIndex = OMX_ALL;
        
        OMX_INIT_STRUCTURE(saturationConfig);
        saturationConfig.nPortIndex = OMX_ALL;
        
        OMX_INIT_STRUCTURE(whiteBalanceConfig);
        whiteBalanceConfig.nPortIndex = OMX_ALL;
        
        OMX_INIT_STRUCTURE(colorEnhancementConfig);
        colorEnhancementConfig.nPortIndex = OMX_ALL;
        
        OMX_INIT_STRUCTURE(imagefilterConfig);
        imagefilterConfig.nPortIndex = OMX_ALL;
        
        OMX_INIT_STRUCTURE(sensorCropConfig);
        sensorCropConfig.nPortIndex = OMX_ALL;
        
        OMX_INIT_STRUCTURE(digitalZoomConfig);
        digitalZoomConfig.nPortIndex = OMX_ALL;
        
        OMX_INIT_STRUCTURE(rotationConfig);
        rotationConfig.nPortIndex = CAMERA_OUTPUT_PORT;
        
        OMX_INIT_STRUCTURE(mirrorConfig);
        mirrorConfig.nPortIndex = CAMERA_OUTPUT_PORT;
        
        OMX_INIT_STRUCTURE(disableSoftwareSharpenConfig);
        OMX_INIT_STRUCTURE(disableSoftwareSaturationConfig);
        
        OMX_INIT_STRUCTURE(dreConfig);
        
        OMX_INIT_STRUCTURE(exposureConfig);
        exposureConfig.nPortIndex = OMX_ALL;
        
        OMX_INIT_STRUCTURE(flickerCancelConfig);
        flickerCancelConfig.nPortIndex = OMX_ALL;
        
        OMX_INIT_STRUCTURE(burstModeConfig);    
    };
    void applyAllSettings() {
        
        setExposurePreset(exposurePreset);
        setMeteringType(meteringType);
        setAutoISO(autoISO);
        setISO(ISO);
        setAutoShutter(autoShutter);
        setShutterSpeed(shutterSpeed);
        
        setSharpness(sharpness);
        setContrast(contrast);
        setBrightness(brightness);
        setSaturation(saturation);
        setFrameStabilization(frameStabilization);
        setWhiteBalance(whiteBalance);
        setImageFilter(imageFilter);
        setColorEnhancement(false);	 //TODO implement
        setDRE(dreLevel);
        setSensorCrop(cropRectangle);
        setDigitalZoom();
        setRotation(rotation);
        setMirror(mirror);
        
        setSoftwareSharpening(doDisableSoftwareSharpen);
        setSoftwareSaturation(doDisableSoftwareSaturation);
        applyExposure(__func__);
        
        //Requires gpio program provided via wiringPi
        //sudo apt-get install wiringpi
        ofFile gpioProgram("/usr/bin/gpio");
        hasGPIOProgram = gpioProgram.exists();
        
        if(hasGPIOProgram)
        {
            LED_PIN = getLEDPin();
            
            stringstream command;
            command << "gpio export ";
            command << LED_PIN;
            command << " out";
            
            if(system(command.str().c_str()) == 0)
            {
                //silence compiler warning 
            }
            LED = true;
            setLEDState(LED);
        } 
    };
    void onUpdate(ofEventArgs & args) {
        if(textureEngine)
        {
            frameCounter  = textureEngine->getFrameCounter();
            
        }else
        {
            if (directEngine)
            {
                frameCounter  = directEngine->getFrameCounter();
            }
        }
        
        if (frameCounter > updateFrameCounter)
        {
            updateFrameCounter = frameCounter;
            hasNewFrame = true;
            
        }else
        {
            hasNewFrame = false;
        }
        if (hasNewFrame)
        {
            if (textureEngine)
            {
                if (pixelsRequested)
                {
                    textureEngine->updatePixels();
                }
            }
        }
        if (recordingRequested)
        {
            recordingRequested = false;
            CameraState currentState = getCameraState();
            currentState.cameraSettings.doRecording = true;
            //ofLogVerbose() << "CALLING SETUP: " << currentStateToString();
            setup(currentState);
        }
        //ofLogVerbose() << "hasNewFrame: " << hasNewFrame;
    };
    void onUpdateDuringExit(ofEventArgs& args) {
        if (doExit)
        {
            ofLogVerbose(__func__) << " EXITING VIA SIGNAL";
            close();
            ofExit();
        }
    };

    OMX_CONFIG_EXPOSURECONTROLTYPE exposurePresetConfig;
    
    OMX_CONFIG_SHARPNESSTYPE sharpnessConfig;
    OMX_CONFIG_FRAMESTABTYPE framestabilizationConfig;
    OMX_CONFIG_CONTRASTTYPE contrastConfig;
    OMX_CONFIG_BRIGHTNESSTYPE brightnessConfig;
    OMX_CONFIG_SATURATIONTYPE saturationConfig;
    OMX_CONFIG_WHITEBALCONTROLTYPE whiteBalanceConfig;
    OMX_CONFIG_COLORENHANCEMENTTYPE colorEnhancementConfig;
    OMX_CONFIG_IMAGEFILTERTYPE imagefilterConfig;
    
    OMX_CONFIG_BOOLEANTYPE burstModeConfig;
    OMX_CONFIG_SCALEFACTORTYPE digitalZoomConfig;
    
    OMX_CONFIG_FLICKERCANCELTYPE flickerCancelConfig;
    OMX_CONFIG_DYNAMICRANGEEXPANSIONTYPE dreConfig;
    OMX_CONFIG_INPUTCROPTYPE sensorCropConfig;
    
    
    
    OMX_CONFIG_ROTATIONTYPE rotationConfig;
    OMX_CONFIG_MIRRORTYPE mirrorConfig;
    
    OMX_CONFIG_BOOLEANTYPE disableSoftwareSharpenConfig;
    OMX_CONFIG_BOOLEANTYPE disableSoftwareSaturationConfig;
	
};

bool doExit = false;
void signal_handler(int signum)
{
    cout << "ofxRPiCameraVideoGrabber caught signal " << signum;
    doExit = true;
}