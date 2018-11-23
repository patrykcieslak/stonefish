//
//  JointSensor.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 21/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/JointSensor.h"

using namespace sf;

JointSensor::JointSensor(std::string uniqueName, btScalar frequency, int historyLength) : ScalarSensor(uniqueName, frequency, historyLength)
{
    fe = NULL;
    jId = 0;
    j = NULL;
}

SensorType JointSensor::getType()
{
    return SensorType::SENSOR_JOINT;
}

void JointSensor::AttachToJoint(FeatherstoneEntity* multibody, unsigned int jointId)
{
    if(multibody != NULL && jointId < multibody->getNumOfJoints())
    {
        fe = multibody;
        jId = jointId;
    }
}

void JointSensor::AttachToJoint(Joint* joint)
{
    if(joint != NULL)
        j = joint;
}
