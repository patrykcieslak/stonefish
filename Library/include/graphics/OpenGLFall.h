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
//  OpenGLFall.h
//  Stonefish
//
//  Created by Patryk Cieslak on 01/07/24.
//  Copyright (c) 2024 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLFall__
#define __Stonefish_OpenGLFall__

#include "graphics/OpenGLParticleSystem.h"
#include "core/MaterialManager.h"

namespace sf
{
    class GLSLShader;

	//! A class implementing a particle system simulating falling objects.
    class OpenGLFall : public OpenGLParticleSystem
    {
    public:
        //! A constructor.
        /*!
         \param maxParticles the number of simulated particles
         \param lifetime the lifetime of the particles [s]
         \param emmiterSize the size of the emmiter in XY plane [m]
         \param meshes a vector of meshes representing the particles
         \param material a material of the particles
         \param look a look of the particles
         */
        OpenGLFall(GLuint maxParticles, GLfloat lifetime, glm::vec2 emitterSize, const std::vector<Mesh*>& meshes, 
            const Material& material, const Look& look);
        
        //! A destructor.
        ~OpenGLFall();	
        
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
        glm::vec2 emitterSize; // Size of the emitter in XY plane [m]
        GLfloat lifetime;      // Lifetime of the particles [s]
        GLuint nParticles;     // Number of particles currently active
        Material material;     // Material of the particles (physics)
        Look look;             // Look of the particles (rendering)

        GLuint ageSSBO;     // Particles age buffer
        GLuint particleDIB; // Particles draw indirect buffer
        GLuint particleVAO; // Particle vertex array
        GLuint particleVertexVBO; // Particle vertex buffer 
        GLuint particleIndexVBO;  // Particle index buffer
        bool textured; // True if the particles are textured
        std::vector<DrawElementsIndirectCommand> drawCommands; // Draw commands for particle meshes
        
        static GLSLShader* updateShader;
		static GLSLShader* renderShader;
    };
}

#endif