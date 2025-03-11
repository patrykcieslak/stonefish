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
//  Copyright (c) 2019-2020 Patryk Cieslak. All rights reserved.
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
        OpenGLParticles(GLuint numOfParticles);
		
        //! A destructor.
        virtual ~OpenGLParticles();	
		
	protected:
		GLuint nParticles;
        GLuint particlePosSSBO; //Includes position and size
        GLuint particleVelSSBO; //Includes velocity and opacity
        GLuint particleVAO; //Vertex array
        GLuint particleEAB; //Indices of particle triangles
	};
}

#endif
