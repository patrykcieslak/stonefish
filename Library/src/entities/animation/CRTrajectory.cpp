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
//  CRTrajectory.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 22/10/2020.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#include "entities/animation/CRTrajectory.h"
#include <algorithm>

namespace sf
{

CRTrajectory::CRTrajectory(PlaybackMode playback) : PWLTrajectory(playback)
{
}

void CRTrajectory::Interpolate()
{
    if(points.size() < 3)
        PWLTrajectory::Interpolate();
    else
    {
        //Find current path segment
        auto it = std::find_if(points.begin(), points.end(),
                               [&](const auto& key){ return key.t >= playTime; });

        Transform T0, T1, T2, T3;
        Scalar t0, t1, t2, t3;

        if(it <= points.begin()+1) //Beginning
        {
            T1 = points[0].T;
            T2 = points[1].T;
            T3 = points[2].T;
            T0.setOrigin(T1.getOrigin()-(T2.getOrigin()-T1.getOrigin()));
            T0.setRotation(T1.getRotation());
            t1 = points[0].t;
            t2 = points[1].t;
            t3 = points[2].t;
            t0 = t1-(t2-t1);
        }
        else if(it >= points.end()-1) //End
        {
            T0 = points[points.size()-3].T;
            T1 = points[points.size()-2].T;
            T2 = points.back().T;
            T3.setOrigin(T2.getOrigin()+(T2.getOrigin()-T1.getOrigin()));
            T3.setRotation(T2.getRotation());
            t0 = points[points.size()-3].t;
            t1 = points[points.size()-2].t;
            t2 = points.back().t;
            t3 = t2 + (t2-t1);
        }
        else
        {
            T0 = (it-2)->T;
            T1 = (it-1)->T;
            T2 = it->T;
            T3 = (it+1)->T;
            t0 = (it-2)->t;
            t1 = (it-1)->t;
            t2 = it->t;
            t3 = (it+1)->t;
        }
        
        //Linear quantities
        if(btFuzzyZero((T2.getOrigin()-T1.getOrigin()).safeNorm())) //Coinciding points
        {
            interpTrans.setOrigin(T2.getOrigin());
            interpVel = V0();
        }
        else
        {
            interpTrans.setOrigin(catmullRom(T0.getOrigin(), T1.getOrigin(), T2.getOrigin(), T3.getOrigin(),
                                                                                    t0, t1, t2, t3, playTime));
            interpVel = catmullRomDerivative(T0.getOrigin(), T1.getOrigin(), T2.getOrigin(), T3.getOrigin(),
                                                                                    t0, t1, t2, t3, playTime);        
        }
        //Angular quantities
        Vector3 dummy;
        interpTrans.setRotation(slerp(T1.getRotation(), T2.getRotation(), (playTime-t1)/(t2-t1)));
        btTransformUtil::calculateVelocity(T1, T2, t2-t1, dummy, interpAngVel);
    }
}

void CRTrajectory::BuildGraphicalPath()
{
    if(points.size() < 3)
        PWLTrajectory::BuildGraphicalPath();
    else
    {
        path.points.clear();
        for(size_t i=0; i<points.size()-1; ++i)
        {
            Vector3 P1 = points[i].T.getOrigin();
            Vector3 P2 = points[i+1].T.getOrigin();
            Scalar t1 = points[i].t;
            Scalar t2 = points[i+1].t;
            Scalar dist = (P2-P1).safeNorm();
            if(btFuzzyZero(dist))
                continue;

            Vector3 P0, P3;
            Scalar t0, t3;

            if(i==0) //Beginning
            {
                P3 = points[2].T.getOrigin();
                P0 = P1-(P2-P1);
                t3 = points[2].t;
                t0 = t1-(t2-t1);
            }
            else if(i==points.size()-2) //End
            {
                P0 = points[i-1].T.getOrigin();
                P3 = P2+(P2-P1);
                t0 = points[i-1].t;
                t3 = t2+(t2-t1);
            }
            else //Middle
            {
                P0 = points[i-1].T.getOrigin();
                P3 = points[i+2].T.getOrigin();
                t0 = points[i-1].t;
                t3 = points[i+2].t;
            }

            Scalar dt = (t2-t1)/Scalar(100.0);
            for(Scalar t=t1; t<t2; t+=dt)
                path.points.push_back(glVectorFromVector(catmullRom(P0, P1, P2, P3, t0, t1, t2, t3, t)));    
        }
        path.points.push_back(glVectorFromVector(points.back().T.getOrigin()));
    }
}

Vector3 CRTrajectory::catmullRom(Vector3 P0, Vector3 P1, Vector3 P2, Vector3 P3, 
                                 Scalar t0, Scalar t1, Scalar t2, Scalar t3, Scalar t)
{
    Vector3 A1 = (t1-t)/(t1-t0)*P0 + (t-t0)/(t1-t0)*P1;
    Vector3 A2 = (t2-t)/(t2-t1)*P1 + (t-t1)/(t2-t1)*P2;
    Vector3 A3 = (t3-t)/(t3-t2)*P2 + (t-t2)/(t3-t2)*P3;
    Vector3 B1 = (t2-t)/(t2-t0)*A1 + (t-t0)/(t2-t0)*A2;
    Vector3 B2 = (t3-t)/(t3-t1)*A2 + (t-t1)/(t3-t1)*A3;
    Vector3 C = (t2-t)/(t2-t1)*B1 + (t-t1)/(t2-t1)*B2;
    return C;
}

Vector3 CRTrajectory::catmullRomDerivative(Vector3 P0, Vector3 P1, Vector3 P2, Vector3 P3, 
                                           Scalar t0, Scalar t1, Scalar t2, Scalar t3, Scalar t)
{
    Vector3 A1 = (t1-t)/(t1-t0)*P0 + (t-t0)/(t1-t0)*P1;
    Vector3 A2 = (t2-t)/(t2-t1)*P1 + (t-t1)/(t2-t1)*P2;
    Vector3 A3 = (t3-t)/(t3-t2)*P2 + (t-t2)/(t3-t2)*P3;
    Vector3 B1 = (t2-t)/(t2-t0)*A1 + (t-t0)/(t2-t0)*A2;
    Vector3 B2 = (t3-t)/(t3-t1)*A2 + (t-t1)/(t3-t1)*A3;
    Vector3 dA1 = Scalar(1)/(t1-t0)*(P1-P0);
    Vector3 dA2 = Scalar(1)/(t2-t1)*(P2-P1);
    Vector3 dA3 = Scalar(1)/(t3-t2)*(P3-P2);
    Vector3 dB1 = Scalar(1)/(t2-t0)*(A2-A1) + (t2-t)/(t2-t0)*dA1 + (t-t0)/(t2-t0)*dA2;
    Vector3 dB2 = Scalar(1)/(t3-t1)*(A3-A2) + (t3-t)/(t3-t1)*dA2 + (t-t1)/(t3-t1)*dA3;
    Vector3 dC = Scalar(1)/(t2-t1)*(B2-B1) + (t2-t)/(t2-t1)*dB1 + (t-t1)/(t2-t1)*dB2;
    return dC;
}

}