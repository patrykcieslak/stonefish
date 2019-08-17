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
//  OpenGLParticles.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/08/19.
//  Copyright (c) 2019 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLParticles.h"

namespace sf
{
	
OpenGLParticles::OpenGLParticles(size_t numOfParticles)	
{
	nParticles = numOfParticles;
	positionsSizes = new glm::vec4[numOfParticles];
	velocities = new glm::vec3[numOfParticles];
}

OpenGLParticles::~OpenGLParticles()
{
	delete [] positionsSizes;
	delete [] velocities;
}

size_t OpenGLParticles::getNumOfParticles()
{
	return nParticles;
}
	
glm::vec4* OpenGLParticles::getPositionsSizes()
{
	return positionsSizes;
}
	
glm::vec3* OpenGLParticles::getVelocities()
{
	return velocities;
}

}
