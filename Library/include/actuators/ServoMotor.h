//
//  ServoMotor.h
//  Stonefish
//
//  Created by Patryk Cieslak on 08/01/2019.
//  Copyright (c) 2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ServoMotor__
#define __Stonefish_ServoMotor__

#include "actuators/JointActuator.h"

namespace sf
{
    //! A class implementing a position/velocity servo motor.
    class ServoMotor : public JointActuator
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the servo motor
         \param positionGain a gain factor used in position control
         \param velocityGain a gain factor used in velocity control
         \param maxTorque the maximum torque that the motor can generate [Nm]
         */
        ServoMotor(std::string uniqueName, Scalar positionGain, Scalar velocityGain, Scalar maxTorque);
        
        //! A method used to attach the actuator to the specified joint of a rigid multibody.
        /*!
         \param multibody a pointer to the multibody
         \param jointId the index of the multibody joint to be actuated
         */
        void AttachToJoint(FeatherstoneEntity* multibody, unsigned int jointId);
        
        //! A method used to update the internal state of the actuator.
        /*!
         \param dt the time step of the simulation [s]
         */
        virtual void Update(Scalar dt);
        
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
        
        //! A method returning the position of the servo motor.
        Scalar getPosition();
        
        //! A method returning the velocity of the servo motor.
        Scalar getVelocity();
        
        //! A method returning the effort of the servo motor (force or torque).
        Scalar getEffort();
        
    private:
        bool velocityMode;
        Scalar pSetpoint;
        Scalar vSetpoint;
        Scalar Kp;
        Scalar Kv;
        Scalar tauMax;
    };
}

#endif
