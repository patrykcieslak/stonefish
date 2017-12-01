//
//  Camera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Camera__
#define __Stonefish_Camera__

#include <functional>
#include "Sensor.h"
#include "OpenGLCamera.h"
#include "SolidEntity.h"

class Camera : public Sensor
{
public:
    //Camera
    Camera(std::string uniqueName, uint32_t resX, uint32_t resY, btScalar horizFOV, const btTransform& geomToSensor, SolidEntity* attachment = NULL, btScalar frequency = btScalar(-1.), uint32_t spp = 1, bool ao = true);
    virtual ~Camera();
    
    void InstallNewDataHandler(std::function<void(Camera*)> callback);
    void NewDataReady();
    void UpdateTransform();
    
    btScalar getHorizontalFOV();
    void getResolution(uint32_t& x, uint32_t& y);
    uint8_t* getDataPointer(); //Should be only used in the callback
    btTransform getSensorFrame();
    
    //Sensor
	virtual void InternalUpdate(btScalar dt);
    virtual std::vector<Renderable> Render();
	virtual SensorType getType();

private:
    OpenGLCamera* glCamera;
    SolidEntity* attach;
    btTransform g2s;
    btScalar fovH;
    uint32_t resx;
    uint32_t resy;
    uint32_t renderSpp;
    bool renderAO;
    std::function<void(Camera*)> newDataCallback;
    uint8_t* imageData;
};

#endif