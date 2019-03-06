//
//  JointActuator.h
//  Stonefish
//
//  Created by Patryk Cieślak on 23/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_JointActuator__
#define __Stonefish_JointActuator__

#include "actuators/Actuator.h"

namespace sf
{
    class FeatherstoneEntity;
    class Joint;
    
    //! An abstract class representing a joint actuator.
    class JointActuator : public Actuator
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the actuator.
         */
        JointActuator(std::string uniqueName);
        
        //! A method used to attach the actuator to the specified joint of a rigid multibody.
        /*!
         \param multibody a pointer to the multibody
         \param jointId the index of the multibody joint to be actuated
         */
        virtual void AttachToJoint(FeatherstoneEntity* multibody, unsigned int jointId);
        
        //! A method used to attach the actuator to a discrete joint.
        /*!
         \param joint a pointer to a joint object
         */
        virtual void AttachToJoint(Joint* joint);
        
        //! A method returning the name of the joint that the actuator is driving.
        std::string getJointName();
        
        //! A method returning the type of the actuator.
        ActuatorType getType();
        
    protected:
        FeatherstoneEntity* fe;
        unsigned int jId;
        Joint* j;
    };
}

#endif
