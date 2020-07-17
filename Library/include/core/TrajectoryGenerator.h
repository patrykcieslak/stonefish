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
//  TrajectoryGenerator.h
//  Stonefish
//
//  Created by Patryk Cieslak on 16/07/2020.
//  Copyright(c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_TrajectoryGenerator__
#define __Stonefish_TrajectoryGenerator__

#include "StonefishCommon.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{ 
    //! A structure representing single trajectory point.
    struct TrajectoryPoint
    {
        Vector3 p;
        Scalar t;

        TrajectoryPoint(Vector3 _p, Scalar _t)
        {
            p = _p;
            t = _t < Scalar(0) ? Scalar(0) : _t;
        }

        static bool Advancing(const TrajectoryPoint& p1, const TrajectoryPoint& p2) 
        { 
            return (p1.t < p2.t);
        }
    };

    //! An abstract class representing a part of trajectory.
    class TrajectorySegment
    {
    public: 
        TrajectorySegment();
        virtual ~TrajectorySegment();
        void AddPoint(const TrajectoryPoint& p);
        void Shift(Scalar startTime);
        virtual Vector3 PointAtTime(Scalar t) const = 0;
        virtual Vector3 VelocityAtTime(Scalar t) const = 0;
        virtual void Sample(std::vector<glm::vec3>& plist) const = 0;
        Scalar getStartTime() const;
        Scalar getEndTime() const;

    protected:
        std::vector<TrajectoryPoint> points;
    };

    //! A class representing a piece-wise linear part of trajectory.
    class PWLSegment : public TrajectorySegment
    {
    public:
        PWLSegment(const TrajectoryPoint& start, const TrajectoryPoint& end);
        Vector3 PointAtTime(Scalar t) const;
        Vector3 VelocityAtTime(Scalar t) const;
        void Sample(std::vector<glm::vec3>& plist) const;
    };

    //! A class representing a spline interpolated part of trajectory.
    /*
    class SplineSegment : public TrajectorySegment
    {
    public:
        SplineSegment(const TrajectoryPoint& start, const TrajectoryPoint& end, const Vector3& tangent1, const Vector3& tangent2);
        Vector3 PointAtTime(Scalar t) const;
        Vector3 VelocityAtTime(Scalar t) const;
    };
    */

    //! A class implementing a 3D trajectory generator.
    class TrajectoryGenerator
    {
    public:
        TrajectoryGenerator();
        ~TrajectoryGenerator();
        
        void AddSegment(TrajectorySegment* s);
        void Advance(Scalar dt);
        std::vector<Renderable> Render();

        Vector3 getCurrentPoint() const;
        Vector3 getCurrentVelocity() const;
    private:
        std::vector<TrajectorySegment*> segments;
        Vector3 p;
        Vector3 v;
        Scalar t;
        size_t seg;
    };
}

#endif