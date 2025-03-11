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
//  OpenGLConsole.h
//  Stonefish
//
//  Created by Patryk Cieślak on 02/12/2018.
//  Copyright (c) 2018-2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLConsole__
#define __Stonefish_OpenGLConsole__

#include "core/Console.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    class GLSLShader;
    class OpenGLPrinter;
    
    //! A class implementing a graphical console.
    class OpenGLConsole : public Console
    {
    public:
        //! A constructor.
        OpenGLConsole();
        
        //! A destructor.
        ~OpenGLConsole();
        
        //! A method that allocates buffers and textures
        /*!
         \param windowW width of the window in pixels
         \param windowH height of the window in pixels
         */
        void Init(int windowW, int windowH);
        
        //! A method which renders the console.
        /*!
         \param overlay defines if the console should be overlayed on the scene rendering
         */
        void Render(bool overlay);
        
        //! A method that scrolls the console.
        /*!
         \param amount number of lines to scroll
         */
        void Scroll(float amount);
        
        //! A method that resets the position of the console.
        void ResetScroll();
        
    private:
        int windowW, windowH;
        float scrollOffset;
        float scrollVelocity;
        int64_t lastTime;
        OpenGLPrinter* printer;
        GLuint logoTexture;
        GLuint consoleVAO;
        GLSLShader* texQuadShader;
    };
}

#endif
