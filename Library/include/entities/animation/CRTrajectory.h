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
//  CRTrajectory.h
//  Stonefish
//
//  Created by Patryk Cieslak on 22/10/20.
//  Copyright(c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_CRTrajectory__
#define __Stonefish_CRTrajectory__

#include "entities/animation/PWLTrajectory.h"

namespace sf
{
    //! A class representing a Catmull-Rom trajectory.
    class CRTrajectory : public PWLTrajectory
    {
    public:
        //! A constructor.
        /*!
         \param playback an enum representing the desired playback mode
         */
        CRTrajectory(PlaybackMode playback);

        //! A method updating the interpolated transform and velocities.
        void Interpolate();

        //! A method that builds a graphical representation of the trajectory.
        void BuildGraphicalPath();

    private:
        Vector3 catmullRom(Vector3 P0, Vector3 P1, Vector3 P2, Vector3 P3, 
                           Scalar t0, Scalar t1, Scalar t2, Scalar t3, Scalar t);
        Vector3 catmullRomDerivative(Vector3 P0, Vector3 P1, Vector3 P2, Vector3 P3, 
                                     Scalar t0, Scalar t1, Scalar t2, Scalar t3, Scalar t);
    };
}

#endif