//
//  LinkActuator.h
//  Stonefish
//
//  Created by Patryk Cieślak on 23/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_LinkActuator__
#define __Stonefish_LinkActuator__

#include "actuators/Actuator.h"
#include "entities/SolidEntity.h"
#include "entities/FeatherstoneEntity.h"

namespace sf
{

class LinkActuator : public Actuator
{
public:
    LinkActuator(std::string uniqueName);
   
    void AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId, const btTransform& location);
    void AttachToSolid(SolidEntity* solid, const btTransform& location);
    ActuatorType getType();
    
protected:
    SolidEntity* attach;
    btTransform g2a;    
};
    
}

#endif
