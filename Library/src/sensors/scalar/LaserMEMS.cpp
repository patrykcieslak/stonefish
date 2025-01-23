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
//  LaserMEMS.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/08/2018.
//  Copyright (c) 2018-2021 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/LaserMEMS.h"

#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "utils/UnitSystem.h"
#include "sensors/Sample.h"
#include "graphics/OpenGLPipeline.h"
#include <iostream>

namespace sf
{

LaserMEMS::LaserMEMS(std::string uniqueName, unsigned int num_lines, unsigned int num_points, Scalar fov_horizontal, Scalar fov_vertical, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{

    fovH = fov_horizontal;
    fovV = fov_vertical;
    numL = num_lines;
    numP=num_points;
    verticalOffset=0;
    
    for(unsigned int i=0; i <= numP; ++i)
    {
    
        channels.push_back(SensorChannel("Distance", QuantityType::LENGTH));
        channels.back().rangeMin = Scalar(0);
        channels.back().rangeMax = BT_LARGE_FLOAT;
    }
    
    distances = std::vector<Scalar>(numP, Scalar(0));
    
}
    
void LaserMEMS::InternalUpdate(Scalar dt)
{
    //get sensor frame in world
    Transform mbTrans = getSensorFrame();
    
    // Set the number of lines and points per line
    //unsigned int numP = numP; // Number of lines
    //unsigned int numL = numL; // Number of points per line

    // Set horizontal FOV to 1 meter and vertical FOV to half a meter
    Scalar sensorWidth = fovH;
    Scalar sensorHeight = fovV;
    Scalar focalLength = sensorWidth / (2.0 * tan(sensorWidth)); // Calculate focal length
    Scalar horizontalStep = sensorWidth / (numP - 1);
    Scalar verticalStep = sensorHeight / (numL - 1);
    // Shoot rays

    for (unsigned int i = 0; i <= numP; ++i){
             // Calculate direction based on both horizontal and vertical angles
             Scalar currentHorizontalAngle = -0.25 + i * horizontalStep;
             Scalar currentVerticalAngle = -0.25 + verticalOffset * verticalStep; // Adjust the starting angle
        // Calculate direction based on both horizontal and vertical angles
        Vector3 dir = mbTrans.getBasis().getColumn(0) * btCos(currentHorizontalAngle) +
                      mbTrans.getBasis().getColumn(1) * btSin(currentHorizontalAngle) * btCos(currentVerticalAngle)+
                      mbTrans.getBasis().getColumn(2) * btSin(currentVerticalAngle);

        Vector3 from = mbTrans.getOrigin() + dir * channels[1].rangeMin;
        Vector3 to = mbTrans.getOrigin() + dir * channels[1].rangeMax;
    
        btCollisionWorld::ClosestRayResultCallback closest(from, to);
        closest.m_collisionFilterGroup = MASK_DYNAMIC;
        closest.m_collisionFilterMask = MASK_STATIC | MASK_DYNAMIC | MASK_ANIMATED_COLLIDING;
        SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld()->rayTest(from, to, closest);
        
        if(closest.hasHit())
        {
            Vector3 p = from.lerp(to, closest.m_closestHitFraction);
            distances[i] = (p - mbTrans.getOrigin()).length();
        }
        else
            distances[i] = channels[i].rangeMax;
     
    }
    
    // Increment vertical offset and reset if necessary
    verticalOffset++;
    if (verticalOffset == numL) {
        verticalOffset = 0; // Restart vertical step
    }
    
    //record sample
    Sample s(numP+1, distances.data());
    AddSampleToHistory(s);
}

std::vector<Renderable> LaserMEMS::Render()
{
    std::vector<Renderable> items = Sensor::Render();
    if (isRenderable())
    {
        Renderable item;
        item.type = RenderableType::SENSOR_LINES;
        item.model = glMatrixFromTransform(getSensorFrame());

        // Set the number of lines and points per line
        //unsigned int numP = numP; // Number of lines
        //unsigned int numL = numL; // Number of points per line

        // Set horizontal FOV to 1 meter and vertical FOV to half a meter
        Scalar sensorWidth = fovH;
        Scalar sensorHeight = fovV;

        Scalar focalLength = sensorWidth / (2.0 * tan(sensorWidth)); // Calculate focal length

        Scalar horizontalStep = sensorWidth / (numP - 1);
        Scalar verticalStep = sensorHeight / (numL - 1);

        for (unsigned int i = 0; i <= numP; ++i)
        {
                // Calculate direction based on both horizontal and vertical angles
                Scalar currentHorizontalAngle = -0.25 + i * horizontalStep;
                Scalar currentVerticalAngle = -0.25 +  verticalOffset * verticalStep; // Adjust the starting angle

                Vector3 dir = Vector3(
                    btCos(currentHorizontalAngle),
                    btSin(currentHorizontalAngle) * btCos(currentVerticalAngle),
                    btSin(currentVerticalAngle)
                );

                item.points.push_back(glm::vec3(0, 0, 0));
                item.points.push_back(glm::vec3(
                    dir.x() *  distances[i],
                    dir.y() *  distances[i],
                    dir.z() *  distances[i]
                ));
            
        items.push_back(item);
        }
        }
    
    return items;
}





void LaserMEMS::setRange(Scalar rangeMin, Scalar rangeMax)
{
    btClamp(rangeMin, Scalar(0), Scalar(BT_LARGE_FLOAT));
    btClamp(rangeMax, Scalar(0), Scalar(BT_LARGE_FLOAT)); 

    for(unsigned int i=0; i<=numP*numL; ++i)
    {
        channels[i].rangeMin = rangeMin;
        channels[i].rangeMax = rangeMax;
    }
}

void LaserMEMS::setNoise(Scalar rangeStdDev)
{
    btClamp(rangeStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT));

    for(unsigned int i=0; i<=numP*numL; ++i)
        channels[i].setStdDev(rangeStdDev);
}

ScalarSensorType LaserMEMS::getScalarSensorType()
{
    return ScalarSensorType::LASERMEMS;
}

Scalar LaserMEMS::getHorizontalFOV(){
    return fovH;
}
        
Scalar LaserMEMS::getVerticalFOV(){
    return fovV;
}
        
int LaserMEMS::getNumPoints(){
    return numP;
}
        
int LaserMEMS::getNumLines(){
    return numL;
}
}
