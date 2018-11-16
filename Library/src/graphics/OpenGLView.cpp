//
//  OpenGLView.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/05/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLView.h"

OpenGLView::OpenGLView(GLint x, GLint y, GLint width, GLint height)
{
    originX = x;
    originY = y;
    viewportWidth = width;
    viewportHeight = height;
    enabled = true;
}

OpenGLView::~OpenGLView()
{
}

void OpenGLView::setEnabled(bool en)
{
    enabled = en;
}

bool OpenGLView::isEnabled()
{
    return enabled;
}

void OpenGLView::SetViewport()
{
    glViewport(0, 0, viewportWidth, viewportHeight);
}

GLint* OpenGLView::GetViewport() const
{
	GLint* view = new GLint[4];
	view[0] = originX;
	view[1] = originY;
	view[2] = viewportWidth;
	view[3] = viewportHeight;
    return view;
}

GLuint OpenGLView::getRenderFBO()
{
    return renderFBO;
}
