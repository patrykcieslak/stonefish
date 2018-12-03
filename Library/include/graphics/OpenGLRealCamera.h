//
//  OpenGLRealCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/12/12.
//  Copyright (c) 2012-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLRealCamera__
#define __Stonefish_OpenGLRealCamera__

#include "graphics/OpenGLCamera.h"

namespace sf
{
    class ColorCamera;
    
    class OpenGLRealCamera : public OpenGLCamera
    {
    public:
        OpenGLRealCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp, GLint originX, GLint originY, GLint width, GLint height, GLfloat fovH, GLfloat horizon, GLuint spp = 1, bool ao = false);
        ~OpenGLRealCamera();
        
        void DrawLDR(GLuint destinationFBO);
        void SetupCamera();
        void UpdateTransform();
        void SetupCamera(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up);
        void Update();
        void RenderDummy();
        
        glm::mat4 GetViewMatrix() const;
        glm::vec3 GetEyePosition() const;
        glm::vec3 GetLookingDirection() const;
        glm::vec3 GetUpDirection() const;
        ViewType getType();
        void setCamera(ColorCamera* cam);
        bool needsUpdate();
        
    private:
        ColorCamera* camera;
        GLuint cameraFBO;
        GLuint cameraColorTex;
        
        glm::mat4 cameraTransform;
        glm::vec3 eye;
        glm::vec3 dir;
        glm::vec3 up;
        glm::vec3 tempEye;
        glm::vec3 tempDir;
        glm::vec3 tempUp;
        bool _needsUpdate;
        bool update;
    };
}

#endif
