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
//  ManualTrajectory.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 23/11/2020.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#include "entities/animation/ManualTrajectory.h"

namespace sf
{

ManualTrajectory::ManualTrajectory() : Trajectory(PlaybackMode::ONETIME)
{
    interpTrans = I4();
    interpVel = V0();
    interpAngVel = V0();
    endTime = 1e16; //Never ends
}

void ManualTrajectory::setTransform(const Transform& T)
{
    interpTrans = T;
}

void ManualTrajectory::setLinearVelocity(const Vector3& v)
{
    interpVel = v;
}

void ManualTrajectory::setAngularVelocity(const Vector3& omega)
{
    interpAngVel = omega;
}

void ManualTrajectory::Interpolate()
{
    return;
}

Renderable ManualTrajectory::Render()
{
    Renderable frame;
    frame.type = RenderableType::SENSOR_CS;
    frame.model = glMatrixFromTransform(interpTrans);
    return frame;
}

}