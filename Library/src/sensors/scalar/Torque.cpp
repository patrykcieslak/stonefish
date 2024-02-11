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
//  Torque.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 20/03/2018.
//  Copyright (c) 2018-2021 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Torque.h"

#include "entities/FeatherstoneEntity.h"
#include "joints/Joint.h"
#include "sensors/Sample.h"

namespace sf
{

Torque::Torque(std::string uniqueName, Scalar frequency, int historyLength) : JointSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Torque", QuantityType::TORQUE));
}

void Torque::InternalUpdate(Scalar dt)
{
    //Vector3 force, torque;
    //fe->getJointFeedback(jId, force, torque);
    //Vector3 axis = fe->getJointAxis(jId);
    //Scalar tau = torque.dot(axis);
    if(fe != NULL)
    {
        Scalar tau = fe->getMotorForceTorque(jId);
    
        Scalar values[1] = {tau};
        Sample s(1, values);
        AddSampleToHistory(s);
    }
}

void Torque::setRange(Scalar torqueMax)
{
    channels[0].rangeMin = -btClamped(torqueMax, Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[0].rangeMax = btClamped(torqueMax, Scalar(0), Scalar(BT_LARGE_FLOAT));
}

void Torque::setNoise(Scalar torqueStdDev)
{
    channels[0].setStdDev(btClamped(torqueStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
}

ScalarSensorType Torque::getScalarSensorType()
{
    return ScalarSensorType::TORQUE;
}

}
