//
//  VisionSensor.h
//  Stonefish
//
//  Created by Patryk Cieślak on 20/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_VisionSensor__
#define __Stonefish_VisionSensor__

#include "sensors/Sensor.h"
#include "entities/FeatherstoneEntity.h"

namespace sf
{

//Abstract
class VisionSensor : public Sensor
{
public:
    VisionSensor(std::string uniqueName, Scalar frequency);
    virtual ~VisionSensor();
    
    virtual void InternalUpdate(Scalar dt) = 0;
    virtual void UpdateTransform() = 0;
    
    void AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId, const Transform& location);
    void AttachToSolid(SolidEntity* solid, const Transform& location);
    virtual Transform getSensorFrame();
    virtual SensorType getType();
    
private:
    SolidEntity* attach;
    Transform g2s;
};
    
}

#endif
