//
//  Thruster.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/10/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Thruster__
#define __Stonefish_Thruster__

#include "actuators/LinkActuator.h"

namespace sf
{
    //! A class representing a thruster.
    class Thruster : public LinkActuator
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the thruster
         \param propeller a pointer to a rigid body representing the propeller
         \param diameter the diameter of the propeller [m]
         \param thrustCoeff the thrust coefficient
         \param torqueCoeff the torque coefficient
         \param maxRPM the maximum rotational speed of the thruster [rpm]
         \param rightHand a flag to indicate if the propeller is right hand (clockwise rotation)
        */
        Thruster(std::string uniqueName, SolidEntity* propeller, Scalar diameter, Scalar thrustCoeff, Scalar torqueCoeff, Scalar maxRPM, bool rightHand);
        
        //! A destructor.
        ~Thruster();
        
        //! A method used to update the internal state of the thruster.
        /*!
         \param dt a time step of the simulation [s]
         */
        void Update(Scalar dt);
        
        //! A method implementing the rendering of the thruster.
        std::vector<Renderable> Render();
        
        //! A method setting the new value of the thruster speed setpoint.
        /*!
         \param s the desired speed of the thruster as fraction <0,1>
         */
        void setSetpoint(Scalar s);
        
        //! A method returning the current setpoint.
        Scalar getSetpoint();
        
        //! A method returning the generated thrust.
        Scalar getThrust();
        
        //! A method returning the angular position of the propeller [rad]
        Scalar getAngle();
        
        //! A method returning the angular velocity of the propeller [rad/s]
        Scalar getOmega();
        
    private:
        //Params
        Scalar D;
        Scalar I;
        Scalar kT;
        Scalar kQ;
        Scalar kp;
        Scalar ki;
        Scalar iLim;
        Scalar omegaLim;
        SolidEntity* prop;
        bool RH;
        
        //States
        Scalar theta;
        Scalar omega;
        Scalar thrust;
        Scalar torque;
        Scalar setpoint;
        Scalar iError;
    };
}

#endif
