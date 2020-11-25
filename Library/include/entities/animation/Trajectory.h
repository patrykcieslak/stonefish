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
//  Trajectory.h
//  Stonefish
//
//  Created by Patryk Cieslak on 21/10/20.
//  Copyright(c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Trajectory__
#define __Stonefish_Trajectory__

#include "StonefishCommon.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    //! An enum representing available trajectory playback modes.
    enum class PlaybackMode {ONETIME, REPEAT, BOOMERANG};

    //! An abstract class representing a trajectory (time parametrized path).
    class Trajectory
    {
    public:
        //! A constructor.
        /*!
         \param playback an enum representing the desired playback mode
         */
        Trajectory(PlaybackMode playback);

        //! A destructor.
        virtual ~Trajectory();

        //! A method updating the trajectory position.
        /*!
         \param dt a time step [s]
         */
        void Play(Scalar dt);

        //! A method updating the interpolated transform and velocities.
        virtual void Interpolate() = 0;

        //! A method returning the elements that should be rendered.
        virtual Renderable Render() = 0;

        //! A method returning the current interpolated transform.
        Transform getInterpolatedTransform() const;

        //! A method returning the current interpolated linear velocity.
        Vector3 getInterpolatedLinearVelocity() const;

        //! A method returning the current interpolated angular velocity.
        Vector3 getInterpolatedAngularVelocity() const;

        //! A method returning the current playback time.
        Scalar getPlaybackTime() const;

        //! A method returning the current playback iteration.
        unsigned int getPlaybackIteration() const;

    protected:
        PlaybackMode playMode;
        Scalar playTime;
        Scalar endTime;
        unsigned int iteration;
        bool forward;
        Transform interpTrans;
        Vector3 interpVel;
        Vector3 interpAngVel;
    };
}

#endif