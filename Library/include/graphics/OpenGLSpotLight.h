//
//  OpenGLSpotLight.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLSpotLight__
#define __Stonefish_OpenGLSpotLight__

#include "graphics/OpenGLLight.h"

namespace sf
{
    //!
    class OpenGLSpotLight : public OpenGLLight
    {
    public:
        OpenGLSpotLight(const Vector3& position, const Vector3& target, GLfloat cone, glm::vec4 color);
        ~OpenGLSpotLight();
        
        //Rendering
        void InitShadowmap(GLint shadowmapLayer);
        void BakeShadowmap(OpenGLPipeline* pipe);
        void SetupShader(GLSLShader* shader, unsigned int lightId);
        
        //Debugging
        void RenderDummy();
        void ShowShadowMap(GLfloat x, GLfloat y, GLfloat w, GLfloat h);
        
        //Field access
        LightType getType();
        glm::vec3 getDirection();
        glm::mat4 getClipSpace();
        GLfloat getAngle();
        
    private:
        glm::vec3 direction;
        GLfloat coneAngle;
        GLfloat zNear;
        GLfloat zFar;
        glm::mat4 clipSpace;
        GLuint shadowFBO;
    };
}

#endif
