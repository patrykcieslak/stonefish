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
//  Copyright (c) 2023-2025 Patryk Cieslak. All rights reserved.
//

#include "entities/animation/BSTrajectory.h"
#include <algorithm>

namespace sf
{

BSTrajectory::BSTrajectory(PlaybackMode playback) : PWLTrajectory(playback)
{
    lastPlayTime_ = 0.0;
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

    //Build B-spline
    if(points_.size() >= 3)
    {
        std::vector<Scalar> cp(points_.size() * 4);
        for(size_t i=0; i<points_.size(); ++i)
        {
            cp[i*4+0] = points_[i].t;
            cp[i*4+1] = points_[i].T.getOrigin().getX();
            cp[i*4+2] = points_[i].T.getOrigin().getY();
            cp[i*4+3] = points_[i].T.getOrigin().getZ();
        }
        spline_ = tinyspline::BSpline::interpolateCubicNatural(cp, 4);
        deriv_ = spline_.derive(1);
    }
    
    BuildGraphicalPath();
    Interpolate();
}

void BSTrajectory::Interpolate()
{
    if(points_.size() < 3)
        PWLTrajectory::Interpolate();
    else
    {
        //Find current path segment
        auto it = std::find_if(points_.begin(), points_.end(),
                               [&](const auto& key){ return key.t >= playTime_; });

        Transform T1, T2;
        Scalar t1, t2;

        if(it <= points_.begin()+1) //Beginning
        {
            T1 = points_[0].T;
            T2 = points_[1].T;
            t1 = points_[0].t;
            t2 = points_[1].t;
        }
        else if(it >= points_.end()-1) //End
        {
            T1 = points_[points_.size()-2].T;
            T2 = points_.back().T;
            t1 = points_[points_.size()-2].t;
            t2 = points_.back().t;
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
            interpTrans_.setOrigin(T2.getOrigin());
            interpVel_ = V0();
        }
        else
        {
            tinyspline::DeBoorNet net = spline_.bisect(playTime_, 0.0001);
            std::vector<Scalar> p = net.result();
            interpTrans_.setOrigin(Vector3(p[1], p[2], p[3]));
            
            Vector3 lastInterpVel = interpVel_;
            std::vector<Scalar> v = deriv_.eval(net.knot()).result();
            interpVel_ = Vector3(v[1]/v[0], v[2]/v[0], v[3]/v[0]);  // dx/dt = (dx/ds) / (ds/dt)
            if(!forward_)
                interpVel_ = -interpVel_;

            if(lastPlayTime_ > 0.0)
                interpAcc_ = (interpVel_ - lastInterpVel)/(playTime_ - lastPlayTime_);
            else
                interpAcc_.setZero();
                
            lastPlayTime_ = playTime_;
        }
        //Angular quantities
        Vector3 dummy;
        interpTrans_.setRotation(slerp(T1.getRotation(), T2.getRotation(), (playTime_-t1)/(t2-t1)));
        calculateVelocityShortestPath(T1, T2, t2-t1, dummy, interpAngVel_);
        if(!forward_)
            interpAngVel_ = -interpAngVel_;
    }
}

void BSTrajectory::BuildGraphicalPath()
{
    PWLTrajectory::BuildGraphicalPath();

    if(points_.size() >= 3)
    {
        vis_[1].getDataAsPoints()->clear();
        std::vector<Scalar> p = spline_.sample((size_t)ceil(points_.back().t * 10));
        for(size_t i = 0; i<p.size(); i+=4)
            vis_[1].getDataAsPoints()->push_back(glm::vec3((GLfloat)p[i+1], (GLfloat)p[i+2], (GLfloat)p[i+3]));
    }
}

}