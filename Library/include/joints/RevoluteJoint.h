//
//  RevoluteJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/13/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_RevoluteJoint__
#define __Stonefish_RevoluteJoint__

#include "joints/Joint.h"

namespace sf
{
    class SolidEntity;
    
    //! A class representing a revolute joint.
    class RevoluteJoint : public Joint
    {
    public:
        RevoluteJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const Vector3& pivot, const Vector3& axis, bool collideLinkedEntities = true);
        RevoluteJoint(std::string uniqueName, SolidEntity* solid, const Vector3& pivot, const Vector3& axis);
        
        void ApplyTorque(Scalar T);
        
        void ApplyDamping();
        
        bool SolvePositionIC(Scalar linearTolerance, Scalar angularTolerance);
        
        std::vector<Renderable> Render();
        
        void setDamping(Scalar constantFactor, Scalar viscousFactor);
        void setLimits(Scalar min, Scalar max);
        void setIC(Scalar angle);
        Scalar getAngle();
        Scalar getAngularVelocity();
        JointType getType();
        
    private:
        Vector3 axisInA;
        Vector3 pivotInA;
        Scalar sigDamping;
        Scalar velDamping;
        Scalar angleIC;
    };
}

#endif
