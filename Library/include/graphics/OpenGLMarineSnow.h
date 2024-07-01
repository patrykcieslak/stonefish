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
//  OpenGLMarineSnow.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/08/19.
//  Copyright (c) 2019-2024 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLMarineSnow__
#define __Stonefish_OpenGLMarineSnow__

#include <random>
#include "graphics/OpenGLParticleSystem.h"

namespace sf
{
	class GLSLShader;
	
	//! A class implementing a particle system simulating underwater snow effect.
	class OpenGLMarineSnow : public OpenGLParticleSystem
	{
	public:
        //! A constructor.
        /*!
         \param numOfParticles the number of simulated particles
		 \param visibleRange a distance at which particles are destroyed/created
         */
        OpenGLMarineSnow(GLuint numOfParticles, GLfloat visibleRange);
		
        //! A destructor.
        ~OpenGLMarineSnow();	
		
		//! A method initializing/resetting the particle system.
		/*!
		 \param cam a pointer to the active camera
		 */
        virtual void Setup(OpenGLCamera* cam);

        //! A method updating the positions/velocities of the particles.
        /*!
         \param cam a pointer to the active camera
         \param dt time step of the simulation [s]
         */
        virtual void Update(OpenGLCamera* cam, GLfloat dt);

        //! A method drawing the particles.
        /*!
         \param cam a pointer to the active camera
         */
		virtual void Draw(OpenGLCamera* cam);
		
		//! A method used to load particle shaders.
		static void Init();
		
		//! A method used to delete particle shaders.
		static void Destroy();
		
	private:
		std::default_random_engine generator;
		std::uniform_real_distribution<GLfloat> uniformd;
		std::normal_distribution<GLfloat> normald;
		bool initialised;
		GLfloat range;
		glm::vec3 lastEyePos;

		GLuint particlePosSSBO; //Includes position and scale
        GLuint particleVelSSBO; //Includes velocity and opacity
        GLuint particleVAO; //Vertex array
        GLuint particleEAB; //Indices of particle triangles

		static GLuint flakeTexture;
		static GLuint noiseTexture; 
		static GLSLShader* updateShader;
		static GLSLShader* renderShader;
	};
}

#endif
