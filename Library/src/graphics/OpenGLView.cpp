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
//  OpenGLView.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/05/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLView.h"

namespace sf
{

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

}
