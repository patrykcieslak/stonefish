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
#include "SolidEntity.h"

class Camera : public Sensor
{
public:
    Camera(std::string uniqueName, uint32_t resX, uint32_t resY, btScalar horizFOVDeg, const btTransform& geomToSensor, SolidEntity* attachment = NULL, btScalar frequency = btScalar(-1.));
    virtual ~Camera();
    
    virtual void SetupCamera(const btVector3& eye, const btVector3& dir, const btVector3& up) = 0;
    void UpdateTransform();
    void setDisplayOnScreen(bool screen);
    void setPan(btScalar value);
    void setTilt(btScalar value);
    btScalar getPan();
    btScalar getTilt();
    btScalar getHorizontalFOV();
    void getResolution(uint32_t& x, uint32_t& y);
    uint8_t* getDataPointer(); //Should be only used in the callback
    btTransform getSensorFrame();
    bool getDisplayOnScreen();
    
    //Sensor
	virtual void InternalUpdate(btScalar dt) = 0;
    virtual std::vector<Renderable> Render();
	virtual SensorType getType();

protected:
    uint8_t* imageData;

private:
    SolidEntity* attach;
    btTransform g2s;
    btScalar fovH;
    btScalar pan;
    btScalar tilt;
    uint32_t resx;
    uint32_t resy;
    bool display;
};

#endif