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
//  BSTrajectory.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 08/11/2023.
//  Copyright (c) 2023 Patryk Cieslak. All rights reserved.
//

#include "entities/animation/BSTrajectory.h"
#include <algorithm>

namespace sf
{

BSTrajectory::BSTrajectory(PlaybackMode playback) : PWLTrajectory(playback)
{
    lastPlayTime = 0.0;
}

void BSTrajectory::AddKeyPoint(Scalar keyTime, Transform keyTransform)
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

    //Build B-spline
    if(points.size() >= 3)
    {
        std::vector<Scalar> cp(points.size() * 4);
        for(size_t i=0; i<points.size(); ++i)
        {
            cp[i*4+0] = points[i].t;
            cp[i*4+1] = points[i].T.getOrigin().getX();
            cp[i*4+2] = points[i].T.getOrigin().getY();
            cp[i*4+3] = points[i].T.getOrigin().getZ();
        }
        spline = tinyspline::BSpline::interpolateCubicNatural(cp, 4);
        deriv = spline.derive(1);
    }
    
    BuildGraphicalPath();
    Interpolate();
}

void BSTrajectory::Interpolate()
{
    if(points.size() < 3)
        PWLTrajectory::Interpolate();
    else
    {
        //Find current path segment
        auto it = std::find_if(points.begin(), points.end(),
                               [&](const auto& key){ return key.t >= playTime; });

        Transform T1, T2;
        Scalar t1, t2;

        if(it <= points.begin()+1) //Beginning
        {
            T1 = points[0].T;
            T2 = points[1].T;
            t1 = points[0].t;
            t2 = points[1].t;
        }
        else if(it >= points.end()-1) //End
        {
            T1 = points[points.size()-2].T;
            T2 = points.back().T;
            t1 = points[points.size()-2].t;
            t2 = points.back().t;
        }
        else
        {
            T1 = (it-1)->T;
            T2 = it->T;
            t1 = (it-1)->t;
            t2 = it->t;
        }

        //Linear quantities
        if(btFuzzyZero((T2.getOrigin()-T1.getOrigin()).safeNorm())) //Coinciding points
        {
            interpTrans.setOrigin(T2.getOrigin());
            interpVel = V0();
        }
        else
        {
            tinyspline::DeBoorNet net = spline.bisect(playTime, 0.0001);
            std::vector<Scalar> p = net.result();
            interpTrans.setOrigin(Vector3(p[1], p[2], p[3]));
            
            Vector3 lastInterpVel = interpVel;
            std::vector<Scalar> v = deriv.eval(net.knot()).result();
            interpVel = Vector3(v[1]/v[0], v[2]/v[0], v[3]/v[0]);  // dx/dt = (dx/ds) / (ds/dt)

            if(lastPlayTime > 0.0)
                interpAcc = (interpVel - lastInterpVel)/(playTime - lastPlayTime);
            else
                interpAcc.setZero();
                
            lastPlayTime = playTime;
        }
        //Angular quantities
        Vector3 dummy;
        interpTrans.setRotation(slerp(T1.getRotation(), T2.getRotation(), (playTime-t1)/(t2-t1)));
        calculateVelocityShortestPath(T1, T2, t2-t1, dummy, interpAngVel);
    }
}

void BSTrajectory::BuildGraphicalPath()
{
    PWLTrajectory::BuildGraphicalPath();

    if(points.size() >= 3)
    {
        vis[1].points.clear();
        std::vector<Scalar> p = spline.sample((size_t)ceil(points.back().t * 10));
        for(size_t i = 0; i<p.size(); i+=4)
            vis[1].points.push_back(glm::vec3((GLfloat)p[i+1], (GLfloat)p[i+2], (GLfloat)p[i+3]));
    }
}

}