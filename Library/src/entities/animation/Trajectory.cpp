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

Vector3 Trajectory::getInterpolatedLinearAcceleration() const
{
    return interpAcc;
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

void Trajectory::calculateVelocityShortestPath(const Transform &transform0, const Transform &transform1, Scalar timeStep, Vector3 &linVel, Vector3 &angVel)
{
    linVel = (transform1.getOrigin() - transform0.getOrigin()) / timeStep;

    //  https://doi.org/10.48550/arXiv.1812.01537 Appendix B, SO(3) Logarithm (Lie Theory)
    Matrix3 R0, R1;
    R0 = transform0.getBasis();
    R1 = transform1.getBasis();
    Matrix3 dR = R0.transpose() * R1;

    // trace of dR
    Scalar tr = dR[0][0] + dR[1][1] + dR[2][2];

    Scalar theta = btAcos((tr - 1.0) / 2.0);

    // check for divide by zero
    if (btFabs(theta) < SIMD_EPSILON)
    {
        angVel.setZero();
    }
    else
    {
        Matrix3 R3 = dR - dR.transpose();
        Vector3 v(R3[2][1], R3[0][2], R3[1][0]);

        angVel = R0 * (v * theta / (2.0 * btSin(theta)) / timeStep);
    }
}
}