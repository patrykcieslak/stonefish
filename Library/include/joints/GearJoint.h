//
//  GearJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 28/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GearJoint__
#define __Stonefish_GearJoint__

#include "joints/Joint.h"

namespace sf
{
    class SolidEntity;
    
    //! A class representing a gear joint.
    class GearJoint : public Joint
    {
    public:
        GearJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const Vector3& axisA, const Vector3& axisB, Scalar ratio);
        
        std::vector<Renderable> Render();
        
        JointType getType();
        Scalar getRatio();
        
    private:
        Scalar gearRatio;
    };
}

#endif
