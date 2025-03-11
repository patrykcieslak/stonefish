/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  CylindricalJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 28/03/2014.
//  Copyright (c) 2014-2023 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_CylindricalJoint__
#define __Stonefish_CylindricalJoint__

#include "joints/Joint.h"

namespace sf
{
    class SolidEntity;
    
    //! A class representing a cylindrical joint.
    class CylindricalJoint : public Joint
    {
    public:
        //! A constructor (a cylindrical joint between two solid bodies).
        /*!
         \param uniqueName a name for the cylindrical joint
         \param solidA pointer to the first solid body
         \param solidB pointer to the second solid body
         \param pivot a point where the axis of joint is located
         \param axis a vector parallel to the joint axis
         \param collideLinked a flag that sets if the bodies connected by the joint should coliide
         */
        CylindricalJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB,
                         const Vector3& pivot, const Vector3& axis, bool collideLinked = true);
        
        //! A method used to apply force to the joint.
        /*!
         \param F force to be applied to the joint [N]
         */
        void ApplyForce(Scalar F);
        
        //! A method used to apply torque to the joint.
        /*!
         \param T torque to be applied to the joint [Nm]
         */
        void ApplyTorque(Scalar T);
        
        //! A method applying damping to the joint.
        void ApplyDamping();
        
        //! A method implementing the rendering of the joint.
        std::vector<Renderable> Render();
        
        //! A method to set the damping characteristics of the joint.
        /*!
         \param linearConstantFactor a constant damping force [N]
         \param linearViscousFactor a coefficient of viscous damping [N*s*rad^-1]
         \param angularConstantFactor a constant damping torque [Nm]
         \param angularViscousFactor a coefficient of viscous damping [Nm*s*rad^-1]
         */
        void setDamping(Scalar linearConstantFactor, Scalar linearViscousFactor, Scalar angularConstantFactor, Scalar angularViscousFactor);
        
        //! A method to set the limits of the joint.
        /*!
         \param linearMin the minimum displacement of the joint [m]
         \param linearMax the maximum displacement of the joint [m]
         \param angularMin the minimum rotation angle of the joint [rad]
         \param angularMax the maximum rotation angle of the joint [rad]
         */
        void setLimits(Scalar linearMin, Scalar linearMax, Scalar angularMin, Scalar angularMax);
        
        //! A method to set the desired initial conditions of the joint.
        /*!
         \param displacement the initial displacement of the joint [m]
         \param angle the initial angle of the joint [rad]
         */
        void setIC(Scalar displacement, Scalar angle);
        
        //! A method returning the type of the joint.
        JointType getType() const;
        
    private:
        Vector3 axisInA;
        Vector3 pivotInA;
        Scalar linSigDamping;
        Scalar linVelDamping;
        Scalar angSigDamping;
        Scalar angVelDamping;
        Scalar displacementIC;
        Scalar angleIC;
    };
}

#endif
