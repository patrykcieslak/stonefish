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
//  TrajectoryGenerator.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 16/07/2020.
//  Copyright(c) 2020 Patryk Cieslak. All rights reserved.
//

#include "core/TrajectoryGenerator.h"

#include <algorithm>

namespace sf
{

TrajectoryGenerator::TrajectoryGenerator()
{
    t = Scalar(0);
    p = V0();
    v = V0();
    seg = 0;
}

TrajectoryGenerator::~TrajectoryGenerator()
{
    for(size_t i=0; i<segments.size(); ++i)
        delete segments[i];
}

Vector3 TrajectoryGenerator::getCurrentPoint() const
{
    return p;
}

Vector3 TrajectoryGenerator::getCurrentVelocity() const
{
    return v;
}

void TrajectoryGenerator::AddSegment(TrajectorySegment* s)
{
    if(segments.size() > 0)
    {
        Scalar endTime = segments[segments.size()-1]->getEndTime();
        s->Shift(endTime); //Shift segment to start immediately after the previous
    }
    else
    {
        p = s->PointAtTime(s->getStartTime()); //Move to start point
    }
    segments.push_back(s);    
}

void TrajectoryGenerator::Advance(Scalar dt)
{
    if(segments.size() == 0) return;
    //Advance time
    t += dt;
    //Check end of current segment
    if(t > segments[seg]->getEndTime())
    {
        if(seg < segments.size()-1)
            ++seg;
    }
    //Calculate new point and velocity
    p = segments[seg]->PointAtTime(t);
    v = segments[seg]->VelocityAtTime(t);   
}

std::vector<Renderable> TrajectoryGenerator::Render()
{
    std::vector<Renderable> items(0);
    
    if(segments.size() == 0)
        return items;
    
    Renderable item;
    item.model = glm::mat4(1.f);
    item.type = RenderableType::SENSOR_LINE_STRIP;
    for(size_t i=0; i<segments.size(); ++i)
        segments[i]->Sample(item.points);
    items.push_back(item);

    return items;
}

//Segments
TrajectorySegment::TrajectorySegment()
{
}

TrajectorySegment::~TrajectorySegment()
{
}

void TrajectorySegment::AddPoint(const TrajectoryPoint& p)
{
    points.push_back(p);
    std::sort(points.begin(), points.end(), TrajectoryPoint::Advancing);
}

void TrajectorySegment::Shift(Scalar startTime)
{
    if(getStartTime() != startTime)
    {
        Scalar shift = startTime - getStartTime();
        for(size_t i=0; i<points.size(); ++i)
            points[i].t += shift;
    }
}

Scalar TrajectorySegment::getStartTime() const
{
    return points.front().t;
}

Scalar TrajectorySegment::getEndTime() const
{
    return points.back().t;
}

//PWL
PWLSegment::PWLSegment(const TrajectoryPoint& start, const TrajectoryPoint& end)
{
    AddPoint(start);
    AddPoint(end);
}

Vector3 PWLSegment::PointAtTime(Scalar t) const
{
    if(t <= getStartTime())
        return points.front().p;
    else if(t >= getEndTime())
        return points.back().p;
    else
    {
        //Find linear part
        for(size_t i=1; i<points.size(); ++i)
        {
            if(t <= points[i].t)
            {
                Scalar interp = (t - points[i-1].t)/(points[i].t - points[i-1].t);
                return points[i-1].p + (points[i].p - points[i-1].p) * interp;
            }
        }
        return V0(); //Never reached
    }
}

Vector3 PWLSegment::VelocityAtTime(Scalar t) const
{
    if(t < getStartTime())
        return V0();
    else if(t > getEndTime())
        return V0();
    else 
    {
        //Find linear part
        for(size_t i=1; i<points.size(); ++i)
        {
            if(t <= points[i].t)
                return (points[i].p - points[i-1].p)/(points[i].t - points[i-1].t);
        }
        return V0(); //Never reached
    }
}

void PWLSegment::Sample(std::vector<glm::vec3>& plist) const
{
    for(size_t i=0; i<points.size(); ++i)
        plist.push_back(glm::vec3((GLfloat)points[i].p.getX(), 
                                  (GLfloat)points[i].p.getY(),
                                  (GLfloat)points[i].p.getZ()));    
}

}