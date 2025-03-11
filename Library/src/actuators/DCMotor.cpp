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
//  DCMotor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/11/13.
//  Copyright (c) 2013-2023 Patryk Cieslak. All rights reserved.
//

#include "actuators/DCMotor.h"

namespace sf
{

DCMotor::DCMotor(std::string uniqueName, Scalar motorR, Scalar motorL, Scalar motorKe, Scalar motorKt, Scalar friction) : Motor(uniqueName)
{
    //Params
    R = motorR;
    L = motorL;
    Ke = motorKe;
    Kt = motorKt;
    B = friction;
    gearEnabled = false;
    gearEff = Scalar(1.);
    gearRatio = Scalar(1.);
    
    //Internal states
    I = Scalar(0.);
    V = Scalar(0.);
    lastVoverL = Scalar(0.);
}

Scalar DCMotor::getKe() const
{
    return Ke;
}

Scalar DCMotor::getKt() const
{
    return Kt;
}

Scalar DCMotor::getR() const
{
    return R;
}

Scalar DCMotor::getL() const
{
    return L;
}

Scalar DCMotor::getGearRatio() const
{
    return gearRatio;
}

void DCMotor::setIntensity(Scalar volt)
{
    V = volt;
}

Scalar DCMotor::getVoltage() const
{
    return V;
}

Scalar DCMotor::getTorque() const
{
    return torque;
}

Scalar DCMotor::getCurrent() const
{
    return I;
}

Scalar DCMotor::getAngle() const
{
    Scalar angle = Motor::getAngle();
    return angle * gearRatio;
}

Scalar DCMotor::getAngularVelocity() const
{
    Scalar angularV = Motor::getAngularVelocity();
    return angularV * gearRatio;
}

void DCMotor::Update(Scalar dt)
{
    //Get joint angular velocity in radians
    Scalar aVelocity = getAngularVelocity();
    
    //Calculate internal state and output
    torque = (I * Kt - aVelocity * B) * gearRatio * gearEff;
    Scalar VoverL = (V - aVelocity * Ke * 9.5493 - I * R)/L;
	I += VoverL * dt;
	
	//Hack to avoid system blowup when the motor starts (shortcut)
	if((btFabs(I) > btFabs(V/R)) && (I*V > Scalar(0)))
		I = V/R;
        
	//I += Scalar(0.5) * (VoverL + lastVoverL) * dt; //Integration (mid-point)
    //lastVoverL = VoverL;
    
	//Drive the joint
    Motor::Update(dt);
}

void DCMotor::SetupGearbox(bool enable, Scalar ratio, Scalar efficiency)
{
    gearEnabled = enable;
    gearRatio = ratio > 0.0 ? ratio : 1.0;
    gearEff = efficiency > 0.0 ? (efficiency <= 1.0 ? efficiency : 1.0) : 1.0;
}

}