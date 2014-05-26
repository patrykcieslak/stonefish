//
//  Trajectory.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 25/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "Trajectory.h"

Trajectory::Trajectory(std::string uniqueName, SolidEntity* attachment, btVector3 offset, unsigned int historyLength) : Sensor(uniqueName, historyLength)
{
    solid = attachment;
    relToCOG = solid->getRigidBody()->getCenterOfMassTransform().getBasis().inverse() * UnitSystem::SetPosition(offset);
}

void Trajectory::Reset()
{
}

void Trajectory::Update(btScalar dt)
{
    //calculate transformation from global to imu frame
    btVector3 cog = solid->getRigidBody()->getCenterOfMassPosition();
    
    //add offset
    if(relToCOG.length2() > 0)
        cog +=  solid->getRigidBody()->getWorldTransform().getBasis() * relToCOG;
    
    //save sample
    btScalar values[3] = {cog.x(), cog.y(), cog.z()};
    Sample s(3, values);
    AddSampleToHistory(s);
}

unsigned short Trajectory::getNumOfDimensions()
{
    return 3;
}

void Trajectory::Render()
{
    if(history.size() > 1)
    {
        glContactColor();
        glBegin(GL_LINE_STRIP);
        for(int i = 0; i < history.size(); i++)
        {
            btVector3 p(history[i]->getValue(0), history[i]->getValue(1), history[i]->getValue(2));
            glBulletVertex(p);
        }
        glEnd();
    }
}

