//
//  OpenGLView.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/05/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "OpenGLView.h"

OpenGLView::OpenGLView(GLint x, GLint y, GLint width, GLint height)
{
    originX = x;
    originY = y;
    viewportWidth = width;
    viewportHeight = height;
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