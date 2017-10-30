//
//  DVL.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/10/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "DVL.h"
#include "SimulationApp.h"
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>

DVL::DVL(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency, unsigned int historyLength) : SimpleSensor(uniqueName, frequency, historyLength)
{
    attach = attachment;
    g2s = UnitSystem::SetTransform(geomToSensor);
    channels.push_back(SensorChannel("Velocity X", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Velocity Y", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Altitude", QUANTITY_LENGTH));
}    
    
void DVL::InternalUpdate(btScalar dt)
{
    //Check hit with bottom
    btScalar altitude(-1);
    btTransform dvlTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    btVector3 from = dvlTrans.getOrigin();
    btVector3 to = from + dvlTrans.getBasis().getColumn(2) * btScalar(10000);
    
    btCollisionWorld::ClosestRayResultCallback closest(from, to);
    SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld()->rayTest(from, to, closest);
                
    if(closest.hasHit())
    {
        btVector3 p = from.lerp(to, closest.m_closestHitFraction);
        altitude = (p-from).length();
    }
    
    //Get velocity
    btVector3 v = dvlTrans.getBasis() * attach->getLinearVelocity();
    
    //Record sample
    btScalar data[3] = {v.x(),v.y(),altitude};
    Sample s(3, data);
    AddSampleToHistory(s);
    
    std::cout << "DVL: " << data[0] << ", " << data[1] << ", " << data[2] << std::endl;
}
    
void DVL::Reset()
{
}
    