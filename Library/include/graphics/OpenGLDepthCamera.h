//
//  OpenGLDepthCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/05/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLDepthCamera__
#define __Stonefish_OpenGLDepthCamera__

#include "graphics/OpenGLView.h"

namespace sf
{
    class DepthCamera;
    class SolidEntity;
    
    //!
    class OpenGLDepthCamera : public OpenGLView
    {
    public:
        OpenGLDepthCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp, GLint originX, GLint originY, GLint width, GLint height, GLfloat fovH, GLfloat minDepth, GLfloat maxDepth);
        ~OpenGLDepthCamera();
        
        void DrawLDR(GLuint destinationFBO);
        glm::vec3 GetEyePosition() const;
        glm::vec3 GetLookingDirection() const;
        glm::vec3 GetUpDirection() const;
        glm::mat4 GetProjectionMatrix() const;
        glm::mat4 GetViewMatrix() const;
        void SetupCamera();
        void SetupCamera(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up);
        void UpdateTransform();
        void Update();
        bool needsUpdate();
        void setCamera(DepthCamera* cam);
        ViewType getType();
        
        static void Init();
        static void Destroy();
        
    protected:
        void LinearizeDepth();
        
        DepthCamera* camera;
        glm::mat4 cameraTransform;
        glm::vec3 eye;
        glm::vec3 dir;
        glm::vec3 up;
        glm::vec3 tempEye;
        glm::vec3 tempDir;
        glm::vec3 tempUp;
        glm::mat4 projection;
        bool _needsUpdate;
        bool update;
        glm::vec2 range;
        GLuint renderDepthTex;
        GLuint linearDepthTex;
        GLuint linearDepthFBO;
        static GLSLShader* depthLinearizeShader;
        static GLSLShader* depthVisualizeShader;
    };
}

#endif
