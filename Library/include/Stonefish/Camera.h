//
//  Camera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Camera__
#define __Stonefish_Camera__

#include "Sensor.h"
#include "OpenGLCamera.h"
#include "SolidEntity.h"

class Camera : public Sensor
{
public:
    Camera(std::string uniqueName, unsigned int resX, unsigned int resY, btScalar horizFOV, const btTransform& geomToSensor, SolidEntity* attachment = NULL, btScalar frequency = btScalar(-1.), unsigned int spp = 1, bool ao = true);
    virtual ~Camera();
    
    void UpdateTransform();
	virtual void InternalUpdate(btScalar dt);
    virtual std::vector<Renderable> Render();
	virtual SensorType getType();

private:
    OpenGLCamera* glCamera;
    SolidEntity* attach;
    btTransform g2s;
    btScalar fovH;
    unsigned int resx;
    unsigned int resy;
    unsigned int renderSpp;
    bool renderAO;
};

#endif