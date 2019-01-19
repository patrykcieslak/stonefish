//
//  Camera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Camera__
#define __Stonefish_Camera__

#include "sensors/VisionSensor.h"

namespace sf
{
    class SolidEntity;
    
    //!
    class Camera : public VisionSensor
    {
    public:
        Camera(std::string uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar horizFOVDeg, Scalar frequency);
        virtual ~Camera();
        
        virtual void InternalUpdate(Scalar dt) = 0;
        virtual void SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up) = 0;
        
        void UpdateTransform();
        void setDisplayOnScreen(bool screen);
        void setPan(Scalar value);
        void setTilt(Scalar value);
        Scalar getPan();
        Scalar getTilt();
        Scalar getHorizontalFOV();
        void getResolution(unsigned int& x, unsigned int& y);
        bool getDisplayOnScreen();
        std::vector<Renderable> Render();
        
    protected:
        Scalar fovH;
        Scalar pan;
        Scalar tilt;
        unsigned int resX;
        unsigned int resY;
        bool display;
    };
}

#endif
