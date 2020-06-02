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
//  OpenGLRealOcean.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/05/2020.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLRealOcean__
#define __Stonefish_OpenGLRealOcean__

#include "graphics/OpenGLOcean.h"
#include <SDL2/SDL_mutex.h>

namespace sf
{
    //! A structure hold the quad-tree information for each camera.
    struct OceanQT
    {
        GLuint patchSSBO[4];
        GLuint patchAC;
        GLuint patchDEI;
        GLuint patchDI;
        GLint pingpong;
    };

    //! A class implementing reallistic deformed ocean in OpenGL.
    class OpenGLRealOcean : public OpenGLOcean
    {
    public:
        //! A constructor.
        /*!
         \param size the size of the ocean surface mesh [m]
         \param state the state of the ocean, if >0 the ocean is rendered with geometric waves otherwise as a plane with wave texture
         \param hydrodynamics a pointer to a mutex
         */
        OpenGLRealOcean(GLfloat size, GLfloat state, SDL_mutex* hydrodynamics);
        
        //! A destructor.
        ~OpenGLRealOcean();

        //! A method that simulates wave propagation.
		/*!
		 \param dt time since last update
		 */
        void Simulate(GLfloat dt);
         
        //! A method that updates the wave mesh.
        /*!
         \param cam a pointer to the active camera
         */
        void UpdateSurface(OpenGLCamera* cam);
        
        //! A method that draws the surface of the ocean.
        /*!
         \param cam a pointer to the active camera
         */
        void DrawSurface(OpenGLCamera* cam);
        
        //! A method that draws the surface of the ocean, seen from underwater.
        /*!
         \param cam a pointer to the active camera
         */
        void DrawBacksurface(OpenGLCamera* cam);
        
        //! A method that generates the stencil mask.
        /*!
         \param cam a pointer to the active camera
         */
        void DrawUnderwaterMask(OpenGLCamera* cam);
                
        //! A method to get wave height at a specified coordinate.
        /*!
         \param x the x coordinate in world frame [m]
         \param y the y coordinate in world frame [m]
         \return wave height [m]
         */
        GLfloat ComputeWaveHeight(GLfloat x, GLfloat y);

        //! A method do enable wireframe rendering.
        /*!
         \param enabled a flag to indicating if wireframe should be enabled
         */
        void setWireframe(bool enabled);
        
    private:
        void InitializeSimulation();
        GLfloat ComputeInterpolatedWaveData(GLfloat x, GLfloat y, GLuint channel);

        GLuint vao;
        GLuint oceanBuffers[2];
        GLuint fftPBO;
        std::map<OpenGLCamera*, OceanQT> oceanTrees; 
        SDL_mutex* hydroMutex;
        GLfloat* fftData;
        GLint qtGridTessFactor;
        GLint qtGPUTessFactor;
        GLint qtPatchIndexCount;
        bool wireframe;
    };
}

#endif
