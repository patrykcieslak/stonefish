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
//  OpenGLParticleSystem.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/08/19.
//  Copyright (c) 2019-2024 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLParticleSystem__
#define __Stonefish_OpenGLParticleSystem__

#include "graphics/OpenGLDataStructs.h"

namespace sf
{
	class OpenGLCamera;

	//! An abstract class implementing a general particle system
	class OpenGLParticleSystem
	{
	public:
        //! A constructor.
        /*!
         \param numOfParticles the number of simulated particles
         */
        OpenGLParticleSystem(GLuint numOfParticles);

        //! A destructor.
        virtual ~OpenGLParticleSystem();	

        //! A method initializing/resetting the particle system.
		/*!
		 \param cam a pointer to the active camera
		 */
        virtual void Setup(OpenGLCamera* cam) = 0;

        //! A method updating the positions/velocities of the particles.
        /*!
         \param cam a pointer to the active camera
         \param dt time step of the simulation [s]
         */
        virtual void Update(OpenGLCamera* cam, GLfloat dt) = 0;

        //! A method drawing the particles.
        /*!
         \param cam a pointer to the active camera
         */
		virtual void Draw(OpenGLCamera* cam) = 0;

	protected:
		GLuint nParticles;
	};
}

#endif
