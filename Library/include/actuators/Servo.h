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
//  Servo.h
//  Stonefish
//
//  Created by Patryk Cieslak on 08/01/2019.
//  Copyright (c) 2019-2024 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ServoMotor__
#define __Stonefish_ServoMotor__

#include "actuators/JointActuator.h"

namespace sf
{
    //! An enum defining possible control modes.
    enum class ServoControlMode {POSITION, VELOCITY, TORQUE};
    
    //! A class implementing a position/velocity servo motor.
    class Servo : public JointActuator
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the servo motor
         \param positionGain a gain factor used in position control
         \param velocityGain a gain factor used in velocity control
         \param maxTorque the maximum torque that the motor can generate [Nm]
         */
        Servo(std::string uniqueName, Scalar positionGain, Scalar velocityGain, Scalar maxTorque);
        
        //! A method used to attach the actuator to the specified joint of a rigid multibody.
        /*!
         \param multibody a pointer to the multibody
         \param jointId the index of the multibody joint to be actuated
         */
        void AttachToJoint(FeatherstoneEntity* multibody, unsigned int jointId);
        
        //! A method used to attach the actuator to a discrete joint.
        /*!
         \param joint a pointer to a joint object
         */
        void AttachToJoint(Joint* joint);
        
        //! A method used to update the internal state of the actuator.
        /*!
         \param dt the time step of the simulation [s]
         */
        virtual void Update(Scalar dt);
        
        //! A method to set the desired control mode.
        /*!
         \param m control mode
         */
        void setControlMode(ServoControlMode m);
        
        //! A method to set the desired position setpoint.
        /*!
         \param pos the desired angular pos [rad]
         */
        void setDesiredPosition(Scalar pos);
        
        //! A method to set the desired velocity setpoint.
        /*!
         \param vel the desired angular velocity [rad/s]
         */
        void setDesiredVelocity(Scalar vel);
        
        //! A method to set the maximum torque applied by the servo.
        /*!
         \param tau the max torque [Nm]
         */
        void setMaxTorque(Scalar tau);

        //! A method returning the desired position setpoint.
        Scalar getDesiredPosition() const;
        
        //! A method returning the desired velocity setpoint.
        Scalar getDesiredVelocity() const;
        
        //! A method returning the position of the servo motor.
        Scalar getPosition() const;
        
        //! A method returning the velocity of the servo motor.
        Scalar getVelocity() const;
        
        //! A method returning the effort of the servo motor (force or torque).
        Scalar getEffort() const;
        
        //! A method returning the type of the actuator.
        ActuatorType getType() const;
        
    private:
        void WatchdogTimeout() override;

        ServoControlMode mode;
        Scalar pSetpoint;
        Scalar vSetpoint;
        Scalar Kp;
        Scalar Kv;
        Scalar tauMax;
    };
}

#endif
