//
//  PrismaticJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 27/03/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_PrismaticJoint__
#define __Stonefish_PrismaticJoint__

#include "joints/Joint.h"

namespace sf
{
    class SolidEntity;
    
    //! A class representing a prismatic joint.
    class PrismaticJoint : public Joint
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the joint
         \param solidA a pointer to the first solid body
         \param solidB a pointer to the second solid body
         \param axis a vector parallel to the joint axis
         \param collideLinked a flag that sets if the bodies connected by the joint should collide
         */
        PrismaticJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB,
                       const Vector3& axis, bool collideLinked = true);
        
        //! A method used to apply force to the joint.
        /*!
         \param F a force to be applied to the joint [N]
         */
        void ApplyForce(Scalar F);
        
        //! A method applying damping to the joint.
        void ApplyDamping();
        
        //! A method implementing the rendering of the joint.
        std::vector<Renderable> Render();
        
        //! A method to set the damping characteristics of the joint.
        /*!
         \param constantFactor a constant damping force [N]
         \param viscousFactor a coefficient of viscous damping [N*s*rad^-1]
         */
        void setDamping(Scalar constantFactor, Scalar viscousFactor);
        
        //! A method to set the limits of the joint.
        /*!
         \param min the minimum displacement of the joint [m]
         \param max the maximum displacement of the joint [m]
         */
        void setLimits(Scalar min, Scalar max);
        
        //! A method to set the desired initial condition of the joint.
        /*!
         \param displacement the initial displacement of the joint [m]
         */
        void setIC(Scalar displacement);
        
        //! A method returning the type of the joint.
        JointType getType();
        
    private:
        Vector3 axisInA;
        Scalar sigDamping;
        Scalar velDamping;
        Scalar displacementIC;
    };
}

#endif
