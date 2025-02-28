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
//  Copyright (c) 2018-2020 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLView.h"

#include "graphics/OpenGLState.h"

namespace sf
{

OpenGLView::OpenGLView(GLint x, GLint y, GLint width, GLint height)
{
    originX = x;
    originY = y;
    viewportWidth = width + width % 2;
    viewportHeight = height + height % 2;
    enabled = true;
	continuous = false;
    viewUBOData.VP = glm::mat4(1.f);
    viewUBOData.eye = glm::vec3(0.f);
    ExtractFrustumFromVP(viewUBOData.frustum, viewUBOData.VP);
}

OpenGLView::~OpenGLView()
{
}

GLuint OpenGLView::getRenderFBO() const
{
    return renderFBO;
}

const ViewUBO* OpenGLView::getViewUBOData() const
{
	return &viewUBOData;
}

void OpenGLView::setEnabled(bool en)
{
    enabled = en;
}

bool OpenGLView::isEnabled()
{
    return enabled;
}

bool OpenGLView::isContinuous()
{
	return continuous;
}

void OpenGLView::SetViewport()
{
    OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
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

GLfloat OpenGLView::GetLogDepthConstant() const
{
    return 1.f/glm::log2(GetFarClip() + 1.f);
}

void OpenGLView::ExtractFrustumFromVP(glm::vec4 frustum[6], const glm::mat4& VP) 
{
	// left
	frustum[0].x = VP[0][3] + VP[0][0];
	frustum[0].y = VP[1][3] + VP[1][0];
	frustum[0].z = VP[2][3] + VP[2][0];
	frustum[0].w = VP[3][3] + VP[3][0];
	frustum[0] = glm::normalize(frustum[0]);

	// right
	frustum[1].x = VP[0][3] - VP[0][0];
	frustum[1].y = VP[1][3] - VP[1][0];
	frustum[1].z = VP[2][3] - VP[2][0];
	frustum[1].w = VP[3][3] - VP[3][0];
	frustum[1] = glm::normalize(frustum[1]);

	// top
	frustum[2].x = VP[0][3] + VP[0][1];
	frustum[2].y = VP[1][3] + VP[1][1];
	frustum[2].z = VP[2][3] + VP[2][1];
	frustum[2].w = VP[3][3] + VP[3][1];
	frustum[2] = glm::normalize(frustum[2]);

	// bottom
	frustum[3].x = VP[0][3] - VP[0][1];
	frustum[3].y = VP[1][3] - VP[1][1];
	frustum[3].z = VP[2][3] - VP[2][1];
	frustum[3].w = VP[3][3] - VP[3][1];
	frustum[3] = glm::normalize(frustum[3]);

	// near
	frustum[4].x = VP[0][3] + VP[0][2];
	frustum[4].y = VP[1][3] + VP[1][2];
	frustum[4].z = VP[2][3] + VP[2][2];
	frustum[4].w = VP[3][3] + VP[3][2];
	frustum[4] = glm::normalize(frustum[4]);

	// far
	frustum[5].x = VP[0][3] - VP[0][2];
	frustum[5].y = VP[1][3] - VP[1][2];
	frustum[5].z = VP[2][3] - VP[2][2];
	frustum[5].w = VP[3][3] - VP[3][2];
	frustum[5] = glm::normalize(frustum[5]);
}

}
