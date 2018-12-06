//
//  OpenGLConsole.h
//  Stonefish
//
//  Created by Patryk Cieślak on 02/12/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
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
        GLuint texQuadVBO;
        GLSLShader* texQuadShader;
    };
}

#endif
