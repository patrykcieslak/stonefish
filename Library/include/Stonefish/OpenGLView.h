//
//  OpenGLView.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/05/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLView__
#define __Stonefish_OpenGLView__

#include "OpenGLPipeline.h"
#include "GLSLShader.h"

class OpenGLView
{
public:
    OpenGLView(GLint originX, GLint originY, GLint width, GLint height);
    virtual ~OpenGLView();
    
    virtual void DrawLDR(GLuint destinationFBO) = 0; //Draw the final image to the screen
    void setEnabled(bool en);
    bool isEnabled();
    
protected:
    GLint originX;
    GLint originY;
    GLint viewportWidth;
    GLint viewportHeight;
    bool enabled;
};

#endif