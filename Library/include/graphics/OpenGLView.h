//
//  OpenGLView.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/05/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLView__
#define __Stonefish_OpenGLView__

#include "graphics/OpenGLPipeline.h"
#include "graphics/GLSLShader.h"

typedef enum {CAMERA, TRACKBALL, DEPTH_CAMERA} ViewType;

class OpenGLView
{
public:
    OpenGLView(GLint originX, GLint originY, GLint width, GLint height);
    virtual ~OpenGLView();
    
    virtual void DrawLDR(GLuint destinationFBO) = 0; //Draw the final image to the screen
    virtual glm::vec3 GetEyePosition() const = 0;
    virtual glm::vec3 GetLookingDirection() const = 0;
    virtual glm::vec3 GetUpDirection() const = 0;
    virtual glm::mat4 GetProjectionMatrix() const = 0;
	virtual glm::mat4 GetViewMatrix() const = 0;
    virtual bool needsUpdate() = 0;
    virtual ViewType getType() = 0;
    
    void SetViewport();
    GLint* GetViewport() const;
    GLuint getRenderFBO();
    void setEnabled(bool en);
    bool isEnabled();
    
protected:
    GLint originX;
    GLint originY;
    GLint viewportWidth;
    GLint viewportHeight;
    GLuint renderFBO;
    bool enabled;
};

#endif
