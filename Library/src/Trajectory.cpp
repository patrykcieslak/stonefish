//
//  Trajectory.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 25/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "Trajectory.h"

#pragma mark Constructors
Trajectory::Trajectory(std::string uniqueName, SolidEntity* attachment, btVector3 offset, btScalar frequency, unsigned int historyLength) : Sensor(uniqueName, frequency, historyLength)
{
    solid = attachment;
    relToCOG = solid->getTransform().getBasis().inverse() * UnitSystem::SetPosition(offset);
    channels.push_back(SensorChannel("Coordinate X", QUANTITY_LENGTH));
    channels.push_back(SensorChannel("Coordinate Y", QUANTITY_LENGTH));
    channels.push_back(SensorChannel("Coordinate Z", QUANTITY_LENGTH));
}

#pragma mark - Methods
void Trajectory::Reset()
{
    Sensor::Reset();
}

void Trajectory::InternalUpdate(btScalar dt)
{
    //calculate transformation from global to imu frame
    btVector3 cog = solid->getTransform().getOrigin();
    
    //add offset
    if(relToCOG.length2() > 0)
        cog +=  solid->getTransform().getBasis() * relToCOG;
    
    //save sample
    btScalar values[3] = {cog.x(), cog.y(), cog.z()};
    Sample s(3, values);
    AddSampleToHistory(s);
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

