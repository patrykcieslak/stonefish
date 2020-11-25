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
//  ManualTrajectory.h
//  Stonefish
//
//  Created by Patryk Cieslak on 23/11/20.
//  Copyright(c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ManualTrajectory__
#define __Stonefish_ManualTrajectory__

#include "entities/animation/Trajectory.h"

namespace sf
{
    //! A class representing a dummy trajectory for setting the transforms and velocities manually.
    class ManualTrajectory : public Trajectory
    {
    public:
        //! A constructor.
        ManualTrajectory();

        //! A method setting the current transform.
        /*!
         \param T current transform
         */
        void setTransform(const Transform& T);

        //! A method setting the current linear velocity.
        /*!
         \param v velocity [m/s]
         */
        void setLinearVelocity(const Vector3& v);

        //! A method setting the current angular velocity.
        /*!
         \param omega angular velocity [rad/s]
         */
        void setAngularVelocity(const Vector3& omega);

        //! A method updating the interpolated transform and velocities.
        virtual void Interpolate();

        //! A method returning the elements that should be rendered.
        Renderable Render();
    };
}

#endif