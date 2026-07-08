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
//  Copyright (c) 2013-2026 Patryk Cieslak. All rights reserved.
//

#include "actuators/DCMotor.h"

namespace sf
{

DCMotor::DCMotor(const std::string& uniqueName, Scalar motorR, Scalar motorL, Scalar motorKe, Scalar motorKt, Scalar friction) : Motor(uniqueName)
{
    //Params
    R_ = motorR;
    L_ = motorL;
    Ke_ = motorKe;
    Kt_ = motorKt;
    B_ = friction;
    gearEnabled_ = false;
    gearEff_ = Scalar(1.);
    gearRatio_ = Scalar(1.);
    
    //Internal states
    I_ = Scalar(0.);
    V_ = Scalar(0.);
    lastVoverL_ = Scalar(0.);
}

Scalar DCMotor::getKe() const
{
    return Ke_;
}

Scalar DCMotor::getKt() const
{
    return Kt_;
}

Scalar DCMotor::getR() const
{
    return R_;
}

Scalar DCMotor::getL() const
{
    return L_;
}

Scalar DCMotor::getGearRatio() const
{
    return gearRatio_;
}

void DCMotor::setCommand(Scalar volt)
{
    V_ = volt;
}

Scalar DCMotor::getVoltage() const
{
    return V_;
}

Scalar DCMotor::getTorque() const
{
    return torque_;
}

Scalar DCMotor::getCurrent() const
{
    return I_;
}

Scalar DCMotor::getAngle() const
{
    Scalar angle = Motor::getAngle();
    return angle * gearRatio_;
}

Scalar DCMotor::getAngularVelocity() const
{
    Scalar angularV = Motor::getAngularVelocity();
    return angularV * gearRatio_;
}

void DCMotor::Update(Scalar dt)
{
    //Get joint angular velocity in radians
    Scalar aVelocity = getAngularVelocity();
    
    //Calculate internal state and output
    torque_ = (I_ * Kt_ - aVelocity * B_) * gearRatio_ * gearEff_;
    Scalar VoverL = (V_ - aVelocity * Ke_ * 9.5493 - I_ * R_)/L_;
	I_ += VoverL * dt;
	
	//Hack to avoid system blowup when the motor starts (shortcut)
	if((btFabs(I_) > btFabs(V_/R_)) && (I_*V_ > Scalar(0)))
		I_ = V_/R_;
        
	//I += Scalar(0.5) * (VoverL + lastVoverL) * dt; //Integration (mid-point)
    //lastVoverL = VoverL;
    
	//Drive the joint
    Motor::Update(dt);
}

void DCMotor::SetupGearbox(bool enable, Scalar ratio, Scalar efficiency)
{
    gearEnabled_ = enable;
    gearRatio_ = ratio > 0.0 ? ratio : 1.0;
    gearEff_ = efficiency > 0.0 ? (efficiency <= 1.0 ? efficiency : 1.0) : 1.0;
}

}