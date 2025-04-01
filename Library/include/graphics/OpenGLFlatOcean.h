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
//  Copyright (c) 2020-2024 Patryk Cieslak. All rights reserved.
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
         \param view a pointer to the active view
         */
        void UpdateSurface(OpenGLView* view) override;
        
        //! A method that draws the surface of the ocean.
        /*!
         \param view a pointer to the active view
         */
        void DrawSurface(OpenGLView* view) override;

        //! A method that draws the surface of the ocean as thermal image.
        /*!
         \param view a pointer to the active view
         */
        void DrawSurfaceTemperature(OpenGLView* view) override;
        
        //! A method that draws the surface of the ocean, seen from underwater.
        /*!
         \param view a pointer to the active view
         */
        void DrawBacksurface(OpenGLView* view) override;
        
        //! A method that generates the stencil mask.
        /*!
         \param view a pointer to the active view
         */
        void DrawUnderwaterMask(OpenGLView* view) override;
        
    private:
        GLuint vao;
        GLuint vbo;
    };
}

#endif
