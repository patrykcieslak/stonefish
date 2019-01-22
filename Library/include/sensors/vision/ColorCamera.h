//
//  ColorCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 4/5/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ColorCamera__
#define __Stonefish_ColorCamera__

#include <functional>
#include "sensors/vision/Camera.h"

namespace sf
{
    class OpenGLRealCamera;
    
    //!
    class ColorCamera : public Camera
    {
    public:
        ColorCamera(std::string uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar horizFOVDeg, unsigned char spp = 1, Scalar frequency = Scalar(-1));
        ~ColorCamera();
        
        void InternalUpdate(Scalar dt);
        void SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up);
        void InstallNewDataHandler(std::function<void(ColorCamera*)> callback);
        void NewDataReady(unsigned int index = 0);
        
        void setExposureCompensation(Scalar comp);
        Scalar getExposureCompensation();
        void* getImageDataPointer(unsigned int index = 0);
        
    private:
        void InitGraphics();
        
        OpenGLRealCamera* glCamera;
        unsigned char samples;
        uint8_t* imageData;
        std::function<void(ColorCamera*)> newDataCallback;
    };
}

#endif
