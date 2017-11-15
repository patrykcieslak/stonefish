//
//  FOG.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "FOG.h"

FOG::FOG(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency, int historyLength) : SimpleSensor(uniqueName, geomToSensor, frequency, historyLength)
{
    attach = attachment;
    channels.push_back(SensorChannel("Heading", QUANTITY_ANGLE));
}

void FOG::InternalUpdate(btScalar dt)
{
    btTransform fogTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    
    //Get angles
    btScalar yaw, pitch, roll;
    fogTrans.getBasis().getEulerYPR(yaw, pitch, roll);
    
    //Record sample
    Sample s(1, &yaw);
    AddSampleToHistory(s);
}

void FOG::SetNoise(btScalar headingStdDev)
{
    channels[0].setStdDev(headingStdDev);
}

btTransform FOG::getSensorFrame()
{
    return attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
}