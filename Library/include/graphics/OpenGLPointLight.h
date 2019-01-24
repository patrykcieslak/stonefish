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
    //! A class implementing an OpenGL point light (shadow not supported).
    class OpenGLPointLight : public OpenGLLight
    {
    public:
        //! A constructor.
        /*!
         \param position the position of the light in the world frame [m]
         \param color the color of the light
         \param illuminance the brightness of the light [lx]
         */
        OpenGLPointLight(glm::vec3 position, glm::vec3 color, GLfloat illuminance);
        
        //! A method to set up light data in a shader.
        /*!
         \param shader a pointer to a GLSL shader
         \param lightId an id of the light
         */
        void SetupShader(GLSLShader* shader, unsigned int lightId);
        
        //! A method returning the type of the light.
        LightType getType();
    };
}

#endif
