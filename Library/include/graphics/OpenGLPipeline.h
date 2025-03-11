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
//  OpenGLPipeline.h
//  Stonefish
//
//  Created by Patryk Cieslak on 30/03/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLPipeline__
#define __Stonefish_OpenGLPipeline__

#include <SDL2/SDL_thread.h>
#include <deque>
#include "StonefishCommon.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    class SimulationManager;
    class OpenGLContent;
    class OpenGLCamera;

    //! A class implementing the OpenGL rendering pipeline.
    class OpenGLPipeline
    {
    public:
        //! A constructor.
        /*!
         \param s a structure containing the render settings
         \param h a structure containing the helper rendering options
         */
        OpenGLPipeline(RenderSettings s, HelperSettings h);
        
        //! A destructor.
        ~OpenGLPipeline();
        
        //! A method that constitutes the main rendering pipeline.
        /*!
         \param sim a pointer to the simulation manager
         */
        void Render(SimulationManager* sim);
        
        //! A method to add renderable objects to the rendering queue.
        /*!
         \param r a renderable object
         */
        void AddToDrawingQueue(const Renderable& r);
		
		//! A method to add multiple renderable objects to the rendering queue.
		/*!
		 \param r a vector of renderable objects
		 */
        void AddToDrawingQueue(const std::vector<Renderable>& r);

        //! A method to add multiple renderable objects to the selected objects rendering queue.
		/*!
		 \param r a vector of renderable objects
		 */
        void AddToSelectedDrawingQueue(const std::vector<Renderable>& r);
		
        //! A method that draws all normal objects.
        void DrawObjects();
		
		//! A method that draws all lights.
		void DrawLights();
        
        //! A method that blits the screen FBO to the main framebuffer.
        void DrawDisplay();
        
        //! A method that clears the drawing queue.
        void PurgeDrawingQueue();

        //! A method that clears the drawing queue for selected objects.
        void PurgeSelectedDrawingQueue();

        //! A method that informs if the drawing queue is empty.
        bool isDrawingQueueEmpty();
        
        //! A method to get mutex of the drawing queue for thread safeness.
        SDL_mutex* getDrawingQueueMutex();
        
        //! A method returning a copy of the render settings.
        RenderSettings getRenderSettings() const;
        
        //! A method returning a reference to the helper object settings.
        HelperSettings& getHelperSettings();

        //! A method returning the screen texture, used for generating GUI background.
        GLuint getScreenTexture();
        
        //! A method returning a pointer to the OpenGL content manager.
        OpenGLContent* getContent();
        
    private:
        void PerformDrawingQueueCopy(SimulationManager* sim);
        void DrawHelpers();
        
        RenderSettings rSettings;
        HelperSettings hSettings;
        std::vector<Renderable> drawingQueue;
        std::vector<Renderable> drawingQueueCopy;
        std::vector<Renderable> selectedDrawingQueue;
        std::vector<Renderable> selectedDrawingQueueCopy;
        SDL_mutex* drawingQueueMutex;
        std::deque<unsigned int> viewsQueue;
        GLuint screenFBO;
        GLuint screenTex;
        OpenGLContent* content;
        Scalar lastSimTime;
    };
}

#endif
