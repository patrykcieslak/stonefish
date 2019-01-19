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

namespace sf
{
    class SolidEntity;
    class FeatherstoneEntity;
    
    //!
    class VisionSensor : public Sensor
    {
    public:
        VisionSensor(std::string uniqueName, Scalar frequency);
        virtual ~VisionSensor();
        
        virtual void InternalUpdate(Scalar dt) = 0;
        virtual void UpdateTransform() = 0;
        
        void AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId, const Transform& origin);
        void AttachToSolid(SolidEntity* solid, const Transform& origin);
        virtual Transform getSensorFrame();
        virtual SensorType getType();
        
    protected:
        virtual void InitGraphics() = 0;
        
    private:
        SolidEntity* attach;
        Transform o2s;
    };
}

#endif
