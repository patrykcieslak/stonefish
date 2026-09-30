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
//  NCSTrajectory.h
//  Stonefish
//
//  Created by Roger Pi on 30/09/2026
//  Copyright (c) 2026 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_NCSTrajectory__
#define __Stonefish_NCSTrajectory__

#include "entities/animation/PWLTrajectory.h"
#include <array>

namespace sf
{
    //! A class representing a natural cubic spline trajectory, parametrized directly by the key point times.
    /*!
     Position is interpolated with a natural cubic spline. Orientation is interpolated with a C2 rotation spline,
     built from quintic segments in the local rotation vector coordinates of each segment start key point.
     */
    class NCSTrajectory : public PWLTrajectory
    {
    public:
        //! A constructor.
        /*!
         \param playback an enum representing the desired playback mode
         */
        NCSTrajectory(PlaybackMode playback);

        //! A method adding a new key point.
        /*!
         \param keyTime the time at point
         \param keyTransform the transform at point
         */
        void AddKeyPoint(Scalar keyTime, Transform keyTransform);

        //! A method updating the interpolated transform, velocities and accelerations.
        void Interpolate();

        //! A method that builds a graphical representation of the trajectory.
        void BuildGraphicalPath();

    private:
        void ComputeCoefficients();
        Vector3 EvalPosition(size_t seg, Scalar u) const;

        //Per segment polynomial coefficients: P(u) = a + b*u + c*u^2 + d*u^3, u = t - t_i
        std::vector<Vector3> a;
        std::vector<Vector3> b;
        std::vector<Vector3> c;
        std::vector<Vector3> d;
        //Per segment rotation coefficients: R(u) = R_i * exp(phi(u)), phi(u) = sum(rot[k] * u^k), k = 0..5
        std::vector<std::array<Vector3, 6>> rot;
    };
}

#endif
