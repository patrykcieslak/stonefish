//
//  ForceTorque.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/10/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "ForceTorque.h"

ForceTorque::ForceTorque(std::string uniqueName, Joint* j, btScalar frequency, unsigned int historyLength) : SimpleSensor(uniqueName, frequency, historyLength)
{
    joint = j;
    
    channels.push_back(SensorChannel("Force X", QUANTITY_FORCE));
    channels.push_back(SensorChannel("Force Y", QUANTITY_FORCE));
    channels.push_back(SensorChannel("Force Z", QUANTITY_FORCE));
    channels.push_back(SensorChannel("Torque X", QUANTITY_TORQUE));
    channels.push_back(SensorChannel("Torque Y", QUANTITY_TORQUE));
    channels.push_back(SensorChannel("Torque Z", QUANTITY_TORQUE));
}

void ForceTorque::Reset()
{
    SimpleSensor::Reset();
}

void ForceTorque::InternalUpdate(btScalar dt)
{
    btScalar Fx = joint->getFeedback(0);
    btScalar Fy = joint->getFeedback(1);
    btScalar Fz = joint->getFeedback(2);
    btScalar Tx = joint->getFeedback(3);
    btScalar Ty = joint->getFeedback(4);
    btScalar Tz = joint->getFeedback(5);
    
    if(joint->isMultibodyJoint())
    {
        Fx /= dt;
        Fy /= dt;
        Fz /= dt;
        Tx /= dt;
        Ty /= dt;
        Tz /= dt;
    }
    
    std::cout << "FT: " << Fx << "," << Fy << "," << Fz << std::endl;
    
    btScalar values[6] = {Fx,Fy,Fz,Tx,Ty,Tz};
    Sample s(6, values);
    AddSampleToHistory(s);
}