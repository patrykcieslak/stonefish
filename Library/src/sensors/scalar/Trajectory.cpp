//
//  Trajectory.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 25/05/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Trajectory.h"

using namespace sf;

Trajectory::Trajectory(std::string uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Coordinate X", QUANTITY_LENGTH));
    channels.push_back(SensorChannel("Coordinate Y", QUANTITY_LENGTH));
    channels.push_back(SensorChannel("Coordinate Z", QUANTITY_LENGTH));
    channels.push_back(SensorChannel("Roll", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Pitch", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Yaw", QUANTITY_ANGLE));
}

void Trajectory::InternalUpdate(Scalar dt)
{
    //get angles
    Transform trajFrame = getSensorFrame();
    Scalar yaw, pitch, roll;
    trajFrame.getBasis().getEulerYPR(yaw, pitch, roll);
    
    //record sample
    Scalar values[6] = {trajFrame.getOrigin().x(), trajFrame.getOrigin().y(), trajFrame.getOrigin().z(), roll, pitch, yaw};
    Sample s(6, values);
    AddSampleToHistory(s);
}

std::vector<Renderable> Trajectory::Render()
{
    std::vector<Renderable> items(0);
    
    /*if(history.size() > 1)
	{
		std::vector<glm::vec3> vertices;
		
		for(unsigned int i = 0; i < history.size(); ++i)
            vertices.push_back(glm::vec3(history[i]->getValue(0), history[i]->getValue(1), history[i]->getValue(2)));
    
		OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINE_STRIP, vertices, CONTACT_COLOR);
	}*/
    
    return items;
}

