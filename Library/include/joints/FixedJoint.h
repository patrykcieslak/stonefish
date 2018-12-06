//
//  FixedJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 2/4/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FixedJoint__
#define __Stonefish_FixedJoint__

#include "joints/Joint.h"

namespace sf
{
    class SolidEntity;
    class FeatherstoneEntity;
    
    //! A class representing a fixed joint.
    class FixedJoint : public Joint
    {
    public:
        //! A constructor to create fixed joint between two solid bodies.
        /*!
         \param uniqueName a name for the joint
         \param solidA a pointer to the first solid body
         \param solidB a pointer to the second solid body
         */
        FixedJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB);
        
        //! A constructor to create fixed joint between two multibodies.
        /*!
         \param uniqueName a name for the joint
         \param feA a pointer to the first multibody
         \param feB a pointer to the second multibody
         \param linkIdA an index of the link of the first multibody
         \param linkIdB an index of the link of the second multibody
         \param pivot a connection point
         */
        FixedJoint(std::string uniqueName, FeatherstoneEntity* feA, FeatherstoneEntity* feB, int linkIdA, int linkIdB, const Vector3& pivot);
        
        //! A method returning the type of the joint.
        JointType getType();
    };
}
    
#endif
