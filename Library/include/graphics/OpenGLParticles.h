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
//  OpenGLParticles.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/08/19.
//  Copyright (c) 2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLParticles__
#define __Stonefish_OpenGLParticles__

#include "graphics/OpenGLDataStructs.h"

namespace sf
{
	class OpenGLCamera;
	
	//! An abstract class implementing a general particle system
	class OpenGLParticles
	{
	public:
        //! A constructor.
        /*!
         \param numOfParticles the number of simulated particles
         */
        OpenGLParticles(size_t numOfParticles);
		
        //! A destructor.
        virtual ~OpenGLParticles();	
		
		//! A method drawing the particles.
		/*!
		 \param cam a pointer to the active camera
		 */
		virtual void Draw(OpenGLCamera* cam) = 0;
		
		//! A method returning the number of particles.
		size_t getNumOfParticles();
		
		//! A method returning the pointer to the position buffer.
		glm::vec3* getPositions();
		
		//! A method returning the pointer to the velocity buffer.
		glm::vec3* getVelocities();
		
	protected:
		size_t nParticles;
        glm::vec3* positions;
		glm::vec3* velocities;
	};
}

#endif
