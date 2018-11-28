//
//  OpenGLDebugDrawer.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 28/06/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLDebugDrawer.h"

#include "graphics/OpenGLContent.h"
#include "graphics/Console.h"

using namespace sf;

OpenGLDebugDrawer::OpenGLDebugDrawer(int debugMode, bool zUp)
{
	zAxisUp = zUp;
    setDebugMode(debugMode);
}

void OpenGLDebugDrawer::setDebugMode(int debugMode)
{
    mode = debugMode;
}

int OpenGLDebugDrawer::getDebugMode() const
{
    return mode;
}

void OpenGLDebugDrawer::drawLine(const Vector3& from, const Vector3& to, const Vector3& color)
{
	std::vector<glm::vec3> vertices;
	vertices.push_back(glm::vec3(from.getX(), from.getY(), from.getZ()));
	vertices.push_back(glm::vec3(to.getX(), to.getY(), to.getZ()));
	
	if(!zAxisUp)
	{
		vertices[0] = glm::vec3(glm::rotate((float)M_PI, glm::vec3(0,1.f,0)) * glm::vec4(vertices[0], 1.f));
		vertices[1] = glm::vec3(glm::rotate((float)M_PI, glm::vec3(0,1.f,0)) * glm::vec4(vertices[1], 1.f));
	}
	
	glm::vec4 glcolor(color[0], color[1], color[2], 1.f);
	OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINES, vertices, glcolor);
}

void OpenGLDebugDrawer::drawLine(const Vector3& from, const Vector3& to, const Vector3& fromColor, const Vector3& toColor)
{
    drawLine(from, to, fromColor);
}

void OpenGLDebugDrawer::drawContactPoint(const Vector3& PointOnB, const Vector3& normalOnB, Scalar distance, int lifeTime, const Vector3& color)
{
}

void OpenGLDebugDrawer::draw3dText(const Vector3& location, const char* textString)
{
}

void OpenGLDebugDrawer::reportErrorWarning(const char* warningString)
{
    cWarning(warningString);
}


