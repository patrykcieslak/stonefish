//
//  Camera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Camera__
#define __Stonefish_Camera__

#include "entities/SolidEntity.h"
#include "sensors/VisionSensor.h"

namespace sf
{

class Camera : public VisionSensor
{
public:
    Camera(std::string uniqueName, uint32_t resX, uint32_t resY, Scalar horizFOVDeg, Scalar frequency);
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
    void getResolution(uint32_t& x, uint32_t& y);
    bool getDisplayOnScreen();
    std::vector<Renderable> Render();
	
private:
    Scalar fovH;
    Scalar pan;
    Scalar tilt;
    uint32_t resx;
    uint32_t resy;
    bool display;
};
    
}

#endif
