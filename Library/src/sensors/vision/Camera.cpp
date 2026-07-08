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
//  Camera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/Camera.h"

#include "entities/SolidEntity.h"

namespace sf
{

Camera::Camera(const std::string& uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar horizFOVDeg, Scalar frequency) : VisionSensor(uniqueName, frequency)
{
    fovH_ = horizFOVDeg <= Scalar(0) ? Scalar(90) : (horizFOVDeg > Scalar(360) ? Scalar(360) : horizFOVDeg);
    resX_ = resolutionX > 0 ? (resolutionX + resolutionX % 2) : 2;
    resY_ = resolutionY > 0 ? (resolutionY + resolutionY % 2) : 2;
    setDisplayOnScreen(false, 0, 0, 1.f);
}

Scalar Camera::getHorizontalFOV() const
{
    return fovH_;
}

void Camera::getResolution(unsigned int& x, unsigned int& y) const
{
    x = resX_;
    y = resY_;
}

void Camera::setDisplayOnScreen(bool display, unsigned int x, unsigned int y, float scale)
{
    screen_ = display;
    screenX_ = x;
    screenY_ = y;
    screenScale_ = scale;
}

bool Camera::getDisplayOnScreen(unsigned int& x, unsigned int& y, float& scale) const
{
    x = screenX_;
    y = screenY_;
    scale = screenScale_;
    return screen_;
}

void Camera::UpdateTransform()
{
    Transform cameraTransform_ = getSensorFrame();
    Vector3 eyePosition = cameraTransform_.getOrigin(); //O
    Vector3 direction = cameraTransform_.getBasis().getColumn(2); //Z
    Vector3 cameraUp = -cameraTransform_.getBasis().getColumn(1); //-Y
    SetupCamera(eyePosition, direction, cameraUp);
}

std::vector<Renderable> Camera::Render()
{
    std::vector<Renderable> items = Sensor::Render();
    if(isRenderable())
    {
        Renderable item;
        item.model = glMatrixFromTransform(getSensorFrame());
        item.type = RenderableType::SENSOR_LINES;
        item.data = std::make_shared<std::vector<glm::vec3>>();
        auto points = item.getDataAsPoints();
        
        //Create camera dummy
        GLfloat iconSize = 0.5f;
        GLfloat x = iconSize*tanf(fovH_/360.f*M_PI);
        GLfloat aspect = (GLfloat)resX_/(GLfloat)resY_;
        GLfloat y = x/aspect;
        
        points->push_back(glm::vec3(0,0,0));
        points->push_back(glm::vec3(x, -y, iconSize));
        points->push_back(glm::vec3(0,0,0));
        points->push_back(glm::vec3(x,  y, iconSize));
        points->push_back(glm::vec3(0,0,0));
        points->push_back(glm::vec3(-x, -y, iconSize));
        points->push_back(glm::vec3(0,0,0));
        points->push_back(glm::vec3(-x,  y, iconSize));
        
        points->push_back(glm::vec3(x, -y, iconSize));
        points->push_back(glm::vec3(x, y, iconSize));
        points->push_back(glm::vec3(x, y, iconSize));
        points->push_back(glm::vec3(-x, y, iconSize));
        points->push_back(glm::vec3(-x, y, iconSize));
        points->push_back(glm::vec3(-x, -y, iconSize));
        points->push_back(glm::vec3(-x, -y, iconSize));
        points->push_back(glm::vec3(x, -y, iconSize));
        
        points->push_back(glm::vec3(-0.5f*x, -y, iconSize));
        points->push_back(glm::vec3(0.f, -1.5f*y, iconSize));
        points->push_back(glm::vec3(0.f, -1.5f*y, iconSize));
        points->push_back(glm::vec3(0.5f*x, -y, iconSize));
        
        items.push_back(item);
    }
    return items;
}

}
