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
//  OpenGLPointLight.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLPointLight__
#define __Stonefish_OpenGLPointLight__

#include "graphics/OpenGLLight.h"

namespace sf
{
    //! A structure representing point light in the Lights UBO.
    #pragma pack(1)
    struct PointLightUBO : public LightUBO
    {
        glm::vec3 position;
        GLfloat radius;
        glm::vec3 color;
        uint8_t pad[4];
    };
    #pragma pack(0)

    //! A class implementing an OpenGL point light (shadow not supported).
    class OpenGLPointLight : public OpenGLLight
    {
    public:
        //! A constructor.
        /*!
         \param position the position of the light in the world frame [m]
		 \param radius the radius of the light source [m]
         \param color the color of the light
         \param lum the luminous power of the light [lm]
         */
        OpenGLPointLight(glm::vec3 position, GLfloat radius, glm::vec3 color, GLfloat lum);
        		
        //! A method to set up light data in Lights UBO.
        /*!
         \param ubo a pointer to the light UBO structure
         */
        void SetupShader(LightUBO* ubo);
        
        //! A method returning the type of the light.
        LightType getType() const;
    };
}

#endif
