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
//  Odometry.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 09/11/2017.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Odometry.h"

#include "entities/SolidEntity.h"
#include "sensors/Sample.h"

namespace sf
{

Odometry::Odometry(std::string uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Position X", QUANTITY_LENGTH));
    channels.push_back(SensorChannel("Position Y", QUANTITY_LENGTH));
    channels.push_back(SensorChannel("Position Z", QUANTITY_LENGTH));
    channels.push_back(SensorChannel("Velocity X", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Velocity Y", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Velocity Z", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Orientation X", QUANTITY_UNITLESS));
    channels.push_back(SensorChannel("Orientation Y", QUANTITY_UNITLESS));
    channels.push_back(SensorChannel("Orientation Z", QUANTITY_UNITLESS));
    channels.push_back(SensorChannel("Orientation W", QUANTITY_UNITLESS));
    channels.push_back(SensorChannel("Angular velocity X", QUANTITY_ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Y", QUANTITY_ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Z", QUANTITY_ANGULAR_VELOCITY));
}

void Odometry::InternalUpdate(Scalar dt)
{
    //Calculate transformation from global to imu frame
    Transform odomTrans = getSensorFrame();
    
    Vector3 pos = odomTrans.getOrigin();
    Vector3 v = odomTrans.getBasis().inverse() * attach->getLinearVelocityInLocalPoint(odomTrans.getOrigin() - attach->getCGTransform().getOrigin());
    
    Quaternion orn = odomTrans.getRotation();
    Vector3 av = odomTrans.getBasis().inverse() * attach->getAngularVelocity();
    
    //Record sample
    Scalar values[13] = {pos.x(), pos.y(), pos.z(), v.x(), v.y(), v.z(), orn.x(), orn.y(), orn.z(), orn.w(), av.x(), av.y(), av.z()};
    Sample s(13, values);
    AddSampleToHistory(s);
}
   
void Odometry::setNoise(Scalar positionStdDev, Scalar velocityStdDev, Scalar orientationStdDev, Scalar angularVelocityStdDev)
{
    channels[0].setStdDev(positionStdDev);
    channels[1].setStdDev(positionStdDev);
    channels[2].setStdDev(positionStdDev);
}

}
