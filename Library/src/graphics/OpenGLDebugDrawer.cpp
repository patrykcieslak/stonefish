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
//  OpenGLDebugDrawer.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 28/06/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLDebugDrawer.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLPipeline.h"

namespace sf
{

OpenGLDebugDrawer::OpenGLDebugDrawer(int debugMode)
{
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
    glm::vec3 p1 = glm::vec3(from.getX(), from.getY(), from.getZ());
    glm::vec3 p2 = glm::vec3(to.getX(), to.getY(), to.getZ());
    lineVertices.push_back(p1);
    lineVertices.push_back(p2);
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

void OpenGLDebugDrawer::Render()
{
    glm::vec4 glcolor(1.f, 1.f, 0.f, 1.f);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawPrimitives(PrimitiveType::LINES, lineVertices, glcolor);
    lineVertices.clear();
}

}
