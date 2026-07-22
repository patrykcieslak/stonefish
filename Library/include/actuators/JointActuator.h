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
//  JointActuator.h
//  Stonefish
//
//  Created by Patryk Cieślak on 23/11/2018.
//  Copyright (c) 2018-2026 Patryk Cieslak. All rights reserved.
//

#pragma once

#include "actuators/Actuator.h"

namespace sf
{
    //! An enum designating a type of joint actuator.
    enum class JointActuatorType {MOTOR, DCMOTOR, SERVO, PLUGIN};

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
        JointActuator(const std::string& uniqueName);

        //! A destructor.
        virtual ~JointActuator() = default;
        
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
        const std::string& getJointName() const;

        //! A method returning the type of the actuator.
        ActuatorType getType() const override;

        //! A method returning type of joint actuator.
        virtual JointActuatorType getJointActuatorType() const = 0;
        
    protected:
        FeatherstoneEntity* fe_;
        unsigned int jId_;
        Joint* j_;
    };
}
