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

namespace sf
{
    class FeatherstoneEntity;
    class SolidEntity;
    
    //! An abstract class representing an actuator that can be attached to a rigid body.
    class LinkActuator : public Actuator
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name of the link actuator
         */
        LinkActuator(std::string uniqueName);
        
        //! A method used to attach the actuator to a specified link of a multibody tree.
        /*!
         \param multibody a pointer to a rigid multibody object
         \param linkId an index of the link
         \param origin a transformation from the link origin to the actuator origin
         */
        virtual void AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId, const Transform& origin);
        
        //! A method used to attach the actuator to a specified rigid body.
        /*!
         \param solid a pointer to a rigid body
         \param origin a transformation from the body origin to the actuator origin
         */
        virtual void AttachToSolid(SolidEntity* solid, const Transform& origin);
        
        //! A method returning actuator frame in the world frame.
        Transform getActuatorFrame();
        
        //! A method returning the type of the actuator.
        virtual ActuatorType getType();
        
    protected:
        SolidEntity* attach;
        Transform o2a;
    };
}

#endif
