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
//  OpenGLLight.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/12/12.
//  Copyright (c) 2012-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLLight__
#define __Stonefish_OpenGLLight__

#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    //! An enum defining supported light types.
    typedef enum {POINT_LIGHT, SPOT_LIGHT} LightType;
    
    class GLSLShader;
    class OpenGLPipeline;
    class OpenGLCamera;
    class SimulationManager;
    
    //! An abstract class implementing an OpenGL light.
    class OpenGLLight
    {
    public:
        //! A constructor.
        /*!
         \param position the position of the light in world frame
         \param color the color of the light
         \param illuminance the brightness of the light [lx]
         */
        OpenGLLight(glm::vec3 position, glm::vec3 color, GLfloat illuminance);
        
        //! A destructor.
        virtual ~OpenGLLight();
        
        //! A method creating shadowmap textures.
        /*!
         \param shadowmapLayer the shadowmap texture layer related to this light
         */
        virtual void InitShadowmap(GLint shadowmapLayer);
        
        //! A method implementing rendering of shadowmaps.
        /*!
         \param pipe a pointer to the OpenGL pipeline
         */
        virtual void BakeShadowmap(OpenGLPipeline* pipe);
        
        //! A method to display generated shadow map on screen.
        /*!
         \param rect a rectangle in which to display the texture
         */
        virtual void ShowShadowMap(glm::vec4 rect);
        
        //! A method to set up light data in a shader.
        /*!
         \param shader a pointer to a GLSL shader
         \param lightId an id of the light
         */
        virtual void SetupShader(GLSLShader* shader, unsigned int lightId) = 0;
        
        //! A method returning the type of the light.
        virtual LightType getType() = 0;
        
        //! A method used to update position of light.
        /*!
         \param p a new position of the light in world frame
         */
        void UpdatePosition(glm::vec3 p);
        
        //! A method that updates light transformation.
        virtual void UpdateTransform();
        
        //! A method to switch on the light.
        void Activate();
        
        //! A method to switch off the light
        void Deactivate();
        
        //! A method to set how far is the light surface from its center.
        /*!
         \param dist the distance from light center to its surface
         */
        void setLightSurfaceDistance(GLfloat dist);
        
        //! A method returning the color of the light.
        glm::vec3 getColor();
        
        //! A method returning the position of the light.
        glm::vec3 getPosition();
        
        //! A method returning the orientation of the light.
        glm::quat getOrientation();
        
        //! A method informing if the light is active.
        bool isActive();
        
        //! A static method to initialize data buffers for all lights in the scene.
        /*!
         \param lights a list of OpenGL lights in the scene
         */
        static void Init(std::vector<OpenGLLight*>& lights);
        
        //! A static method to destroy the common data.
        static void Destroy();
        
        //! A static method to setup light data in shaders.
        /*!
         \param shader a pointer to a GLSL shader object
         */
        static void SetupShader(GLSLShader* shader);
        
        //! A static method to set the current view.
        /*!
         \param view a pointer to the current view
         */
        static void SetCamera(OpenGLCamera* view);
        
    protected:
        GLfloat surfaceDistance;
        
        static GLuint spotShadowArrayTex; //2D array texture for storing shadowmaps of all spot lights (using only one texture unit for all spotlights!)
        static GLuint spotShadowSampler;
        static GLuint spotDepthSampler;
        static OpenGLCamera* activeView;
        
    private:
        bool active;
        glm::vec3 pos;
        glm::vec3 tempPos;
        glm::vec3 color;
    };
}

#endif
