//
//  SphericalJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 2/3/13.
//  Copyright (c) 2013-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SphericalJoint__
#define __Stonefish_SphericalJoint__

#include "joints/Joint.h"

namespace sf
{
    class SolidEntity;
    
    //! A class representing a spherical joint.
    class SphericalJoint : public Joint
    {
    public:
        //! A constructor (a spherical joint between two solid bodies).
        /*!
         \param uniqueName a name for the spherical joint
         \param solidA pointer to the first solid body
         \param solidB pointer to the second solid body
         \param pivot a point where the axis of joint is located
         \param collideLinked a flag that sets if the bodies connected by the joint should coliide
         */
        SphericalJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const Vector3& pivot, bool collideLinked = true);
        
        //! A method used to apply torque to the joint.
        /*!
         \param T a torque vector to be applied to the joint [Nm]
         */
        void ApplyTorque(Vector3 T);
        
        //! A method applying damping to the joint.
        void ApplyDamping();
        
        //! A method implementing the rendering of the joint.
        std::vector<Renderable> Render();
        
        //! A method to set the damping characteristics of the joint.
        /*!
         \param constantFactor a constant damping torque vector [Nm]
         \param viscousFactor a coefficient of viscous damping [Nm*s*rad^-1]
         */
        void setDamping(Vector3 constantFactor, Vector3 viscousFactor);
        
        //! A method to set the desired initial condition of the joint.
        /*!
         \param angles the initial angles of rotation the joint [rad]
         */
        void setIC(Vector3 angles);
        
        //! A method returning the type of the joint.
        JointType getType();
        
    private:
        Vector3 sigDamping;
        Vector3 velDamping;
        Vector3 angleIC;
    };
}
    
#endif
