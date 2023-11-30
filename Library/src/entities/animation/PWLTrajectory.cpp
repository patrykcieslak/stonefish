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
//  Copyright (c) 2020-2023 Patryk Cieslak. All rights reserved.
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

    Renderable pathLine;
    pathLine.type = RenderableType::PATH_LINE_STRIP;
    pathLine.model = glm::mat4(1.f);

    vis.push_back(pathPoints);
    vis.push_back(pathLine);
    
    interpAcc = V0();
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
    auto it = std::find(points.begin(), points.end(), k);
    if(it != points.end())
        *it = k;
    else
        points.push_back(k);
    
    //Sort key points by time
    std::sort(points.begin(), points.end());

    //Reset
    playTime = Scalar(0);
    endTime = points.back().t;
    forward = true;
    BuildGraphicalPath();
    Interpolate();
}

void PWLTrajectory::Interpolate()
{
    if(points.size() == 1)
    {
        interpTrans = points[0].T;
        interpVel = V0();
        interpAngVel = V0();
        return;
    }

    //Find current path segment
    auto it = std::find_if(points.begin(), points.end(),
                           [&](const auto& key){ return key.t >= playTime; });

    if(it->t == playTime) //No interpolation needed
    {
        interpTrans = it->T;
        if(it == points.begin()) //Time = 0
            calculateVelocityShortestPath(it->T, (it+1)->T, (it+1)->t, interpVel, interpAngVel);
        else
            calculateVelocityShortestPath((it-1)->T, it->T, it->t-(it-1)->t, interpVel, interpAngVel);
    }
    else
    {
        Scalar alpha = (playTime - (it-1)->t)/(it->t - (it-1)->t);
        interpTrans.setOrigin(lerp((it-1)->T.getOrigin(), it->T.getOrigin(), alpha));
        interpTrans.setRotation(slerp((it-1)->T.getRotation(), it->T.getRotation(), alpha));
        calculateVelocityShortestPath((it-1)->T, it->T, it->t-(it-1)->t, interpVel, interpAngVel);
    }
}

void PWLTrajectory::BuildGraphicalPath()
{
    vis[0].points.clear();
    vis[1].points.clear();
    for(size_t i=0; i<points.size(); ++i)
        vis[0].points.push_back(glVectorFromVector(points[i].T.getOrigin()));
    vis[1].points = vis[0].points;
}

std::vector<Renderable> PWLTrajectory::Render()
{
    return vis;
}

}