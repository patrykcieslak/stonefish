//
//  SphericalJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 2/3/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SphericalJoint__
#define __Stonefish_SphericalJoint__

#include "joints/Joint.h"

namespace sf
{
    class SolidEntity;
    
    //!
    class SphericalJoint : public Joint
    {
    public:
        SphericalJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const Vector3& pivot, bool collideLinkedEntities = true);
        
        void ApplyTorque(Vector3 T);
        void ApplyDamping();
        bool SolvePositionIC(Scalar linearTolerance, Scalar angularTolerance);
        Vector3 Render();
        
        void setDamping(Vector3 constantFactor, Vector3 viscousFactor);
        void setIC(Vector3 angles);
        
        JointType getType();
        
    private:
        Vector3 sigDamping;
        Vector3 velDamping;
        Vector3 angleIC;
    };
}
    
#endif
