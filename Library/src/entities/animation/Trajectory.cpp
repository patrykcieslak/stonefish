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
//  Trajectory.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 21/10/2020.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#include "entities/animation/Trajectory.h"

namespace sf
{

Trajectory::Trajectory(PlaybackMode playback) 
    : playMode(playback), playTime(0), endTime(0), iteration(0), forward(true)
{
}

Trajectory::~Trajectory()
{
}

Scalar Trajectory::getPlaybackTime() const
{
    return playTime;
}

unsigned int Trajectory::getPlaybackIteration() const
{
    return iteration;
}

Transform Trajectory::getInterpolatedTransform() const
{
    return interpTrans;
}

Vector3 Trajectory::getInterpolatedLinearVelocity() const
{
    return interpVel;
}

Vector3 Trajectory::getInterpolatedAngularVelocity() const
{
    return interpAngVel;
}

void Trajectory::Play(Scalar dt)
{
    //Update time
    switch(playMode)
    {
        case PlaybackMode::ONETIME:
        {
            playTime += dt;
            if(playTime > endTime)
                playTime = endTime;
        }
            break;

        case PlaybackMode::REPEAT:
        {
            playTime += dt;
            if(playTime > endTime)
            {
                playTime -= endTime;
                ++iteration;
            }
        }
            break;

        case PlaybackMode::BOOMERANG:
        {
            playTime += forward ? dt : -dt;
            if(playTime > endTime)
            {
                playTime = endTime - (playTime - endTime);
                forward = false;
                ++iteration;
            }
            else if(playTime < Scalar(0))
            {
                playTime = -playTime;
                forward = true;
                ++iteration;
            }
        }
            break;
    }

    //Compute new interpolants
    Interpolate();
}

}