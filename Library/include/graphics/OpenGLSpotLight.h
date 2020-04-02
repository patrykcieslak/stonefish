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
//  OpenGLSpotLight.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLSpotLight__
#define __Stonefish_OpenGLSpotLight__

#include "graphics/OpenGLLight.h"

namespace sf
{
    //! A structure representing a spot light in the Ligths UBO (std140 aligned).
    #pragma pack(1)
    struct SpotLightUBO : public LightUBO
    {
        glm::mat4 clipSpace;
        glm::vec3 position;
        GLfloat frustumNear;
        glm::vec3 direction;
        GLfloat frustumFar;
        glm::vec3 color;
        GLfloat cone;
        glm::vec2 radius;
        uint8_t pad[8];
    };
    #pragma pack(0)

    //! A class implementing an OpenGL spot light with shadow.
    class OpenGLSpotLight : public OpenGLLight
    {
    public:
        //! A constructor.
        /*!
         \param position the position of the light in the world frame [m]
         \param direction a unit vector parallel to the light cone axis
		 \param radius a radius of the light source [m]
         \param coneAngleDeg the angle of the light cone [deg]
         \param color the color of the light
         \param illuminance the brightness of the light [lx]
         */
        OpenGLSpotLight(glm::vec3 position, glm::vec3 direction, GLfloat radius, GLfloat coneAngleDeg, glm::vec3 color, GLfloat illuminance);
        
        //! A destructor.
        ~OpenGLSpotLight();
        
        //! A method creating shadowmap textures.
        /*!
         \param shadowmapLayer the shadowmap texture layer related to this light
         */
        void InitShadowmap(GLint shadowmapLayer);
        
        //! A method implementing rendering of shadowmaps.
        /*!
         \param pipe a pointer to the OpenGL pipeline
         */
        void BakeShadowmap(OpenGLPipeline* pipe);
        
        //! A method to display generated shadow map on screen.
        /*!
         \param rect a rectangle in which to display the texture
         */
        void ShowShadowMap(glm::vec4 rect);
		
		//! A method implementing rendering of light surface.
		void DrawLight();
    
        //! A method to set up light data in Lights UBO.
        /*!
         \param ubo a pointer to the light UBO structure
         */
        void SetupShader(LightUBO* ubo);
        
        //! A method used to update direction of the spot light.
        /*!
         \param d a new direction of the light cone
         */
        void UpdateDirection(glm::vec3 d);
        
        //! A method that updates light transformation.
        void UpdateTransform();
        
        //! A method returning the type of the light.
        LightType getType();
        
        //! A method returning the direction vector of the light.
        glm::vec3 getDirection();
        
        //! A method returning light clip space information.
        glm::mat4 getClipSpace();
        
        //! A method returning the spot light cone angle.
        GLfloat getAngle();
        
    private:
        glm::vec3 dir;
        glm::vec3 tempDir;
        GLfloat coneAngle;
        GLfloat zNear;
        GLfloat zFar;
        glm::mat4 clipSpace;
        GLuint shadowFBO;
    };
}

#endif
