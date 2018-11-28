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
    ForceTorque(std::string uniqueName, SolidEntity* attachment, const Transform& geomToSensor, Scalar frequency = Scalar(-1), int historyLength = -1);
    ForceTorque(std::string uniqueName, const Transform& geomToSensor, Scalar frequency = Scalar(-1), int historyLength = -1);
    
    void InternalUpdate(Scalar dt);
    void SetRange(const Vector3& forceMax, const Vector3& torqueMax);
    void SetNoise(Scalar forceStdDev, Scalar torqueStdDev);
    std::vector<Renderable> Render();
    
private:
    SolidEntity* attach;
    Transform g2s;
    Transform lastFrame;
};
    
}

#endif
