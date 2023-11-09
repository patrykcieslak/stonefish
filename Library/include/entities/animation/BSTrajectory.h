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
//  BSTrajectory.h
//  Stonefish
//
//  Created by Patryk Cieslak on 08/11/23.
//  Copyright(c) 2023 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_BSTrajectory__
#define __Stonefish_BSTrajectory__

#include "entities/animation/PWLTrajectory.h"
#include "tinysplinecxx.h"

namespace sf
{
    //! A class representing a B-spline trajectory.
    class BSTrajectory : public PWLTrajectory
    {
    public:
        //! A constructor.
        /*!
         \param playback an enum representing the desired playback mode
         */
        BSTrajectory(PlaybackMode playback);

        //! A method adding a new key point.
        /*!
         \param keyTime the time at point
         \param keyTransform the transform at point
         */
        void AddKeyPoint(Scalar keyTime, Transform keyTransform);

        //! A method updating the interpolated transform and velocities.
        void Interpolate();

        //! A method that builds a graphical representation of the trajectory.
        void BuildGraphicalPath();

    private:
        tinyspline::BSpline spline;
        tinyspline::BSpline deriv1;
        tinyspline::BSpline deriv2;
    };
}

#endif