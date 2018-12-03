//
//  LinkSensor.h
//  Stonefish
//
//  Created by Patryk Cieślak on 20/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_LinkSensor__
#define __Stonefish_LinkSensor__

#include "sensors/ScalarSensor.h"

namespace sf
{
    class SolidEntity;
    class FeatherstoneEntity;
    
    //!
    class LinkSensor : public ScalarSensor
    {
    public:
        LinkSensor(std::string uniqueName, Scalar frequency, int historyLength);
        virtual ~LinkSensor();
        
        virtual void InternalUpdate(Scalar dt) = 0;
        
        virtual std::vector<Renderable> Render();
        void AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId, const Transform& origin);
        void AttachToSolid(SolidEntity* solid, const Transform& origin);
        Transform getSensorFrame();
        SensorType getType();
        
    protected:
        SolidEntity* attach;
        Transform o2s;
    };
}

#endif
