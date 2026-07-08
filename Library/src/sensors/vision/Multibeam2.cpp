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
//  Multibeam2.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 21/01/2019.
//  Copyright (c) 2019-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/Multibeam2.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLDepthCamera.h"

namespace sf
{

Multibeam2::Multibeam2(const std::string& uniqueName, unsigned int horizontalRes, unsigned int verticalRes, Scalar horizontalFOVDeg, Scalar verticalFOVDeg,
                       Scalar minRange, Scalar maxRange, Scalar frequency) : Camera(uniqueName, horizontalRes, verticalRes, horizontalFOVDeg, frequency)
{
    fovV_ = verticalFOVDeg > Scalar(0) ? verticalFOVDeg : Scalar(90);
    range_.x = minRange < Scalar(0.01) ? 0.01f : (GLfloat)minRange;
    range_.y = maxRange > Scalar(0.01) ? (GLfloat)maxRange : 1.f;
    newDataCallback_ = nullptr;
    dataCounter_ = 0;
    imageData_.resize(resX_*resY_); // Buffer for storing image data
    rangeData_.resize(resX_*resY_); // Buffer for storing final data
}
    
void* Multibeam2::getImageDataPointer(unsigned int index)
{
    if(cameras_.size() > index)
        return &imageData_[cameras_[index].dataOffset];
    else
        return nullptr;
}
    
float* Multibeam2::getRangeDataPointer()
{
    return rangeData_.data();
}
    
glm::vec2 Multibeam2::getRangeLimits() const
{
    return range_;
}

Scalar Multibeam2::getVerticalFOV() const
{
    return fovV_;
}

VisionSensorType Multibeam2::getVisionSensorType() const
{
    return VisionSensorType::MULTIBEAM2;
}

OpenGLView* Multibeam2::getOpenGLView() const
{
    if(cameras_.size() > 0)
        return cameras_[0].cam;
    else
        return nullptr;
}
    
void Multibeam2::InitGraphics(bool& seesParticles)
{
    seesParticles = false;

    if(fovH_ <= Scalar(MULTIBEAM_MAX_SINGLE_FOV))
    {
        CamData cd;
        cd.cam = nullptr;
        cd.fovH = (GLfloat)fovH_;
        cd.width = resX_;
        cd.dataOffset = 0;
        cameras_.push_back(cd);
    }
    else
    {
        //Find number of cameras needed not to exceed max FOV for a single camera
        int nCam = (int)ceil(fovH_/Scalar(MULTIBEAM_MAX_SINGLE_FOV));
        Scalar fovH1 = fovH_/Scalar(nCam);
        
        //Check if resolution in X can be divided evenly for this number of cameras
        int resmod = resX_ % nCam;
        if(resmod == 0)
        {
            for(int i=0; i<nCam; ++i)
            {
                CamData cd;
                cd.cam = nullptr;
                cd.fovH = (GLfloat)fovH1;
                cd.width = resX_/nCam;
                cd.dataOffset = 0;
                cameras_.push_back(cd);
            }
        }
        else
        {
            //Correct FOV and resolution of the cameras
            int resX1 = resX_/nCam;
            int resXc = resX1 + resmod;
            Scalar fovHc = fovH_/(Scalar(1) + (nCam-1)*Scalar(resX1)/Scalar(resXc));
            fovH1 = fovHc * Scalar(resX1)/Scalar(resXc);
            
            CamData cd;
            cd.cam = nullptr;
            cd.dataOffset = 0;
            
            cd.fovH = (GLfloat)fovHc;
            cd.width = resXc;
            cameras_.push_back(cd);
            
            cd.fovH = (GLfloat)fovH1;
            cd.width = resX1;
            for(int i=0; i<nCam-1; ++i) cameras_.push_back(cd);
        }
    }
    
    //Create depth cameras
    GLint accResX = 0;
    for(size_t i=0; i<cameras_.size(); ++i)
    {
        std::unique_ptr<OpenGLDepthCamera> camera = std::make_unique<OpenGLDepthCamera>(
            glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0),
            accResX, 0, cameras_[i].width, resY_, cameras_[i].fovH, range_.x, range_.y, true, (GLfloat)fovV_
        );

        cameras_[i].cam = camera.get();
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(std::move(camera));

        cameras_[i].cam->setCamera(this, (unsigned int)i);
        cameras_[i].dataOffset = accResX*resY_;
        accResX += cameras_[i].width;
    }
    
    //Update camera transformations
    UpdateTransform();
    
    for(size_t i=0; i<cameras_.size(); ++i)
    {
        cameras_[i].cam->UpdateTransform();
        cameras_[i].cam->Update();
    }
}

void Multibeam2::InternalUpdate(Scalar dt)
{
    for(size_t i=0; i<cameras_.size(); ++i)
        cameras_[i].cam->Update();
}
    
void Multibeam2::UpdateTransform()
{
    Transform mbTransform = getSensorFrame();
    Vector3 eyePosition = mbTransform.getOrigin(); //O
    Vector3 direction = mbTransform.getBasis().getColumn(2); //Z
    Vector3 cameraUp = -mbTransform.getBasis().getColumn(1); //-Y
    Scalar accFov(0);
    Scalar offset = fovH_/Scalar(360)*M_PI;
   
    for(size_t i=0; i<cameras_.size(); ++i)
    {
        //Calculate i-th camera transform
        Scalar halfFov = cameras_[i].fovH/Scalar(360)*M_PI;
        Vector3 dir = direction.rotate(cameraUp, offset - accFov - halfFov);
        accFov += Scalar(2)*halfFov;
        
        //Setup camera transformation
        SetupCamera(i, eyePosition, dir, cameraUp);
    }
}

void Multibeam2::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
}
    
void Multibeam2::SetupCamera(size_t index, const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    cameras_[index].cam->SetupCamera(eye_, dir_, up_);
}
    
void Multibeam2::InstallNewDataHandler(std::function<void(Multibeam2*)> callback)
{
    newDataCallback_ = callback;
}
    
void Multibeam2::NewDataReady(void* data, unsigned int index)
{
    if(index >= cameras_.size())
        return;

    memcpy(getImageDataPointer(index), data, cameras_[index].width * resY_ * sizeof(GLfloat));
    dataCounter_ += index;
    int lastIndex = (int)cameras_.size()-1;
    int nSum = lastIndex*(lastIndex+1)/2;
    
    if(dataCounter_ == nSum) //Check if data received from all cameras
    {
        dataCounter_ = 0;
        
        //Copy (rearange) data from temp to final sonar image
        for(size_t i=0; i<cameras_.size(); ++i)
        {
            size_t xoffset = cameras_[i].dataOffset/resY_;
            
            for(size_t h=0; h<resY_; ++h)
            {
                memcpy(&rangeData_[xoffset + h*resX_],
                       &imageData_[cameras_[i].dataOffset + h*cameras_[i].width],
                       sizeof(GLfloat)*cameras_[i].width);
            }
        }
        
        //Call callback
        if(newDataCallback_ != nullptr)
            newDataCallback_(this);
    }
}
    
std::vector<Renderable> Multibeam2::Render()
{
    std::vector<Renderable> items = Sensor::Render();
    if(isRenderable())
    {
        Renderable item;
        item.model = glMatrixFromTransform(getSensorFrame());
        item.type = RenderableType::SENSOR_LINES;
        item.data = std::make_shared<std::vector<glm::vec3>>();
        auto points = item.getDataAsPoints();
        
        unsigned int div = (unsigned int)ceil(fovH_/5.0);
        GLfloat iconSize = 0.5f;
        GLfloat cosFovV2 = cosf(fovV_/360.f*M_PI);
        GLfloat sinFovV2 = sinf(fovV_/360.f*M_PI);
        GLfloat r = iconSize/cosFovV2;
        GLfloat thetaDiv = fovH_/180.f * M_PI/(GLfloat)div;
        GLfloat offset = -fovH_/360.f * M_PI;
        GLfloat y = sinFovV2 * r;
        
        for(unsigned int i=0; i<div; ++i)
        {
            GLfloat theta1 = i*thetaDiv + offset;
            GLfloat theta2 = theta1 + thetaDiv;
            GLfloat d1 = cosf(theta1) * r;
            GLfloat d2 = cosf(theta2) * r;
            GLfloat z1 = cosFovV2 * d1;
            GLfloat z2 = cosFovV2 * d2;
            GLfloat x1 = sinf(theta1) * r;
            GLfloat x2 = sinf(theta2) * r;
            
            points->push_back(glm::vec3(x1,y,z1));
            points->push_back(glm::vec3(x2,y,z2));
            points->push_back(glm::vec3(x1,-y,z1));
            points->push_back(glm::vec3(x2,-y,z2));
            
            if(i == 0) //End 1
            {
                points->push_back(glm::vec3(x1,y,z1));
                points->push_back(glm::vec3(x1,-y,z1));
                points->push_back(glm::vec3(x1,y,z1));
                points->push_back(glm::vec3(0,0,0));
                points->push_back(glm::vec3(x1,-y,z1));
                points->push_back(glm::vec3(0,0,0));
            }
            else if(i == div-1) //End 2
            {
                points->push_back(glm::vec3(x2,y,z2));
                points->push_back(glm::vec3(x2,-y,z2));
                points->push_back(glm::vec3(x2,y,z2));
                points->push_back(glm::vec3(0,0,0));
                points->push_back(glm::vec3(x2,-y,z2));
                points->push_back(glm::vec3(0,0,0));
            }
        }
        
        items.push_back(item);
    }
    return items;
}
    
}
