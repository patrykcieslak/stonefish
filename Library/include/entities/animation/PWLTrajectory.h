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
//  PWLTrajectory.h
//  Stonefish
//
//  Created by Patryk Cieslak on 21/10/20.
//  Copyright(c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_PWLTrajectory__
#define __Stonefish_PWLTrajectory__

#include "entities/animation/Trajectory.h"

namespace sf
{
    //! A structure representing a trajectory point.
    struct KeyPoint
    {
        Scalar t;
        Transform T;

        bool operator< (const KeyPoint& rhs) const { return (t < rhs.t); }
        bool operator== (const KeyPoint& rhs) const { return (t == rhs.t); }
    };

    //! A class representing a piece-wise linear trajectory.
    class PWLTrajectory : public Trajectory
    {
    public:
        //! A constructor.
        /*!
         \param playback an enum representing the desired playback mode
         */
        PWLTrajectory(PlaybackMode playback);

        //! A method adding a new key point.
        /*!
         \param keyTime the time at point
         \param keyTransform the transform at point
         */
        void AddKeyPoint(Scalar keyTime, Transform keyTransform);

        //! A method updating the interpolated transform and velocities.
        virtual void Interpolate();

        //! A method that builds a graphical representation of the trajectory.
        virtual void BuildGraphicalPath();

        //! A method returning the elements that should be rendered.
        Renderable Render();

    protected:
        std::vector<KeyPoint> points;
        Renderable path;
    };
}

#endif