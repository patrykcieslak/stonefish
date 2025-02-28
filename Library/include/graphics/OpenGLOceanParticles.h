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
//  OpenGLOceanParticles.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/08/19.
//  Copyright (c) 2019-2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLOceanParticles__
#define __Stonefish_OpenGLOceanParticles__

#include <random>
#include "graphics/OpenGLParticles.h"

namespace sf
{
	class GLSLShader;
	class Ocean;
    class OpenGLOcean;

	//! A class implementing a particle system simulating underwater snow effect.
	class OpenGLOceanParticles : public OpenGLParticles
	{
	public:
        //! A constructor.
        /*!
         \param numOfParticles the number of simulated particles
		 \param visibleRange a distance at which particles are destroyed/created
         */
        OpenGLOceanParticles(size_t numOfParticles, GLfloat visibleRange);
		
        //! A destructor.
        ~OpenGLOceanParticles();	
		
		//! A method updating the positions/velocities of the particles.
		/*!
         \param cam a pointer to the active camera
		 \param dt time passed since last update
         */
		void Update(OpenGLCamera* cam, GLfloat dt);
		
		//! A method drawing the particles.
		/*!
		 \param cam a pointer to the active camera
         \param glOcn a pointer to the OpenGL ocean object
		 */
		void Draw(OpenGLCamera* cam, OpenGLOcean* glOcn);
		
		//! A method used to load particle shaders.
		static void Init();
		
		//! A method used to delete particle shaders.
		static void Destroy();
		
	private:
		void Create(glm::vec3 eyePos);
	
		std::default_random_engine generator;
		std::uniform_real_distribution<GLfloat> uniformd;
		std::normal_distribution<GLfloat> normald;
		bool initialised;
		GLfloat range;
		glm::vec3 lastEyePos;

		static GLuint flakeTexture;
		static GLuint noiseTexture; 
		static GLSLShader* updateShader;
		static GLSLShader* renderShader;
	};
}

#endif
