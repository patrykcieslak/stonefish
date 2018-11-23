//
//  ForceTorque.h
//  Stonefish
//
//  Created by Patryk Cieslak on 30/10/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ForceTorque__
#define __Stonefish_ForceTorque__

#include "entities/FeatherstoneEntity.h"
#include "joints/Joint.h"
#include "sensors/scalar/JointSensor.h"

namespace sf
{

class ForceTorque : public JointSensor
{
public:
    ForceTorque(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency = btScalar(-1), int historyLength = -1);
    ForceTorque(std::string uniqueName, const btTransform& geomToSensor, btScalar frequency = btScalar(-1), int historyLength = -1);
    
    void InternalUpdate(btScalar dt);
    void SetRange(const btVector3& forceMax, const btVector3& torqueMax);
    void SetNoise(btScalar forceStdDev, btScalar torqueStdDev);
    std::vector<Renderable> Render();
    
private:
    SolidEntity* attach;
    btTransform g2s;
    btTransform lastFrame;
};
    
}

#endif
