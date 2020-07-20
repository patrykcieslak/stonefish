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
//  OpenGLFlatOcean.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/05/2020.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLFlatOcean__
#define __Stonefish_OpenGLFlatOcean__

#include "graphics/OpenGLOcean.h"

namespace sf
{
    //! A class implementing flat ocean in OpenGL.
    class OpenGLFlatOcean : public OpenGLOcean
    {
    public:
        //! A constructor.
        /*!
         \param size the size of the ocean surface mesh [m]
         */
        OpenGLFlatOcean(GLfloat size);
        
        //! A destructor.
        ~OpenGLFlatOcean();
         
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
        
    private:
        GLuint vao;
        GLuint vbo;
    };
}

#endif
