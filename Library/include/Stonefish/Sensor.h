//
//  Sensor.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Sensor__
#define __Stonefish_Sensor__

#include "UnitSystem.h"
#include "NameManager.h"

typedef enum {SENSOR_SIMPLE, SENSOR_CAMERA, SENSOR_LIGHT} SensorType;

//Abstract class
class Sensor
{
public:
    Sensor(std::string uniqueName, btScalar frequency = btScalar(-1.));
    virtual ~Sensor();
    
    virtual void Reset();
    void Update(btScalar dt);
    bool isRenderable();
    void setRenderable(bool render);
    std::string getName();
	
	//Abstract
	virtual void InternalUpdate(btScalar dt) = 0;
    virtual void Render() = 0;
	virtual SensorType getType() = 0;
    
protected:
    btScalar freq;
    
private:
    std::string name;
    btScalar eleapsedTime;
    bool renderable;
    
    static NameManager nameManager;
};

#endif
