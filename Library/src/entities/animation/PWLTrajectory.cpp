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
//  PWLTrajectory.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 21/10/2020.
//  Copyright (c) 2020-2025 Patryk Cieslak. All rights reserved.
//

#include "entities/animation/PWLTrajectory.h"
#include <algorithm>

namespace sf
{

PWLTrajectory::PWLTrajectory(PlaybackMode playback) : Trajectory(playback)
{
    Renderable pathPoints;
    pathPoints.type = RenderableType::PATH_POINTS;
    pathPoints.model = glm::mat4(1.f);
    pathPoints.data = std::make_shared<std::vector<glm::vec3>>();

    Renderable pathLine;
    pathLine.type = RenderableType::PATH_LINE_STRIP;
    pathLine.model = glm::mat4(1.f);
    pathLine.data = std::make_shared<std::vector<glm::vec3>>();

    vis_.push_back(pathPoints);
    vis_.push_back(pathLine);
    
    interpAcc_ = V0();
    AddKeyPoint(Scalar(0), I4());
}

void PWLTrajectory::AddKeyPoint(Scalar keyTime, Transform keyTransform)
{
    //Check if time correct
    if(keyTime < Scalar(0)) return;

    //Create key point
    KeyPoint k;
    k.t = keyTime;
    k.T = keyTransform;

    //Add to the list
    auto it = std::find(points_.begin(), points_.end(), k);
    if(it != points_.end())
        *it = k;
    else
        points_.push_back(k);
    
    //Sort key points by time
    std::sort(points_.begin(), points_.end());

    //Reset
    playTime_ = Scalar(0);
    endTime_ = points_.back().t;
    forward_ = true;
    BuildGraphicalPath();
    Interpolate();
}

void PWLTrajectory::Interpolate()
{
    if(points_.size() == 1)
    {
        interpTrans_ = points_[0].T;
        interpVel_ = V0();
        interpAngVel_ = V0();
        return;
    }

    //Find current path segment
    auto it = std::find_if(points_.begin(), points_.end(),
                           [&](const auto& key){ return key.t >= playTime_; });

    if(it->t == playTime_) //No interpolation needed
    {
        interpTrans_ = it->T;
        if(it == points_.begin()) //Time = 0
            calculateVelocityShortestPath(it->T, (it+1)->T, (it+1)->t, interpVel_, interpAngVel_);
        else
            calculateVelocityShortestPath((it-1)->T, it->T, it->t-(it-1)->t, interpVel_, interpAngVel_);
    }
    else
    {
        Scalar alpha = (playTime_ - (it-1)->t)/(it->t - (it-1)->t);
        interpTrans_.setOrigin(lerp((it-1)->T.getOrigin(), it->T.getOrigin(), alpha));
        interpTrans_.setRotation(slerp((it-1)->T.getRotation(), it->T.getRotation(), alpha));
        calculateVelocityShortestPath((it-1)->T, it->T, it->t-(it-1)->t, interpVel_, interpAngVel_);
    }

    if(!forward_)
    {
        interpVel_ = -interpVel_;
        interpAngVel_ = -interpAngVel_;
    }
}

void PWLTrajectory::BuildGraphicalPath()
{
    vis_[0].getDataAsPoints()->clear();
    vis_[1].getDataAsPoints()->clear();
    for(size_t i=0; i<points_.size(); ++i)
        vis_[0].getDataAsPoints()->push_back(glVectorFromVector(points_[i].T.getOrigin()));
    *vis_[1].getDataAsPoints() = *vis_[0].getDataAsPoints();
}

std::vector<Renderable> PWLTrajectory::Render()
{
    return vis_;
}

}