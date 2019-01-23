//
//  Multibeam2.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 21/01/2019.
//  Copyright (c) 2019 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/Multibeam2.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLDepthCamera.h"

namespace sf
{

Multibeam2::Multibeam2(std::string uniqueName, unsigned int horizontalRes, unsigned int verticalRes, Scalar horizontalFOVDeg, Scalar verticalFOVDeg,
                       Scalar minDepth, Scalar maxDepth, Scalar frequency) : Camera(uniqueName, horizontalRes, verticalRes, horizontalFOVDeg, frequency)
{
    fovV = verticalFOVDeg > Scalar(0) ? verticalFOVDeg : Scalar(90);
    depthRange.x = minDepth < Scalar(0.01) ? 0.01f : (GLfloat)minDepth;
    depthRange.y = maxDepth > Scalar(0.01) ? (GLfloat)maxDepth : 1.f;
    newDataCallback = NULL;
    dataCounter = 0;
    imageData = new GLfloat[resX*resY]; //Buffer for storing image data
    memset(imageData, 0, resX*resY*sizeof(GLfloat));
    rangeData = new GLfloat[resX*resY]; //Buffer for storing final data
    memset(rangeData, 0, resX*resY*sizeof(GLfloat));
}

Multibeam2::~Multibeam2()
{
    if(imageData != NULL)
        delete imageData;
    if(rangeData != NULL)
        delete rangeData;
    cameras.clear();
}
    
void* Multibeam2::getImageDataPointer(unsigned int index)
{
    if(cameras.size() > index)
        return &imageData[cameras[index].dataOffset];
    else
        return NULL;
}
    
float* Multibeam2::getRangeDataPointer()
{
    return rangeData;
}
    
glm::vec2 Multibeam2::getDepthRange()
{
    return depthRange;
}
    
void Multibeam2::InitGraphics()
{
    if(fovH <= Scalar(MULTIBEAM_MAX_SINGLE_FOV))
    {
        CamData cd;
        cd.cam = NULL;
        cd.fovH = (GLfloat)fovH;
        cd.width = resX;
        cd.dataOffset = 0;
        cameras.push_back(cd);
    }
    else
    {
        //Find number of cameras needed not to exceed max FOV for a single camera
        int nCam = (int)ceil(fovH/Scalar(MULTIBEAM_MAX_SINGLE_FOV));
        Scalar fovH1 = fovH/Scalar(nCam);
        
        //Check if resolution in X can be divided evenly for this number of cameras
        int resmod = resX % nCam;
        if(resmod == 0)
        {
            for(int i=0; i<nCam; ++i)
            {
                CamData cd;
                cd.cam = NULL;
                cd.fovH = (GLfloat)fovH1;
                cd.width = resX/nCam;
                cd.dataOffset = 0;
                cameras.push_back(cd);
            }
        }
        else
        {
            //Correct FOV and resolution of the cameras
            int resX1 = resX/nCam;
            int resXc = resX1 + resmod;
            Scalar fovHc = fovH/(Scalar(1) + (nCam-1)*Scalar(resX1)/Scalar(resXc));
            fovH1 = fovHc * Scalar(resX1)/Scalar(resXc);
            
            CamData cd;
            cd.cam = NULL;
            cd.dataOffset = 0;
            
            cd.fovH = (GLfloat)fovHc;
            cd.width = resXc;
            cameras.push_back(cd);
            
            cd.fovH = (GLfloat)fovH1;
            cd.width = resX1;
            for(int i=0; i<nCam-1; ++i) cameras.push_back(cd);
        }
    }
    
    //Create depth cameras
    GLint accResX = 0;
    for(int i=0; i<cameras.size(); ++i)
    {
        cameras[i].cam = new OpenGLDepthCamera(glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0),
                                                            accResX, 0, cameras[i].width, resY, cameras[i].fovH, depthRange.x, depthRange.y, true, (GLfloat)fovV);
        cameras[i].cam->setCamera(this, i);
        cameras[i].dataOffset = accResX*resY;
        accResX += cameras[i].width;
    }
    
    //Update camera transformations
    UpdateTransform();
    
    for(size_t i=0; i<cameras.size(); ++i)
    {
        cameras[i].cam->UpdateTransform();
        cameras[i].cam->Update();
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(cameras[i].cam);
    }
}

void Multibeam2::InternalUpdate(Scalar dt)
{
    for(size_t i=0; i<cameras.size(); ++i)
        cameras[i].cam->Update();
}
    
void Multibeam2::UpdateTransform()
{
    Transform mbTransform = getSensorFrame();
    Vector3 eyePosition = mbTransform.getOrigin(); //O
    Vector3 direction = mbTransform.getBasis().getColumn(2).normalized(); //Z
    Vector3 cameraUp = -mbTransform.getBasis().getColumn(1).normalized(); //-Y
    
    Matrix3 rotation;
    rotation.setEulerYPR(0,M_PI,0);
    Vector3 rotEyePosition = rotation * eyePosition;
    Vector3 rotCameraUp = rotation * cameraUp;
    Scalar accFov(0);
    Scalar offset = fovH/Scalar(360)*M_PI;
    
    for(size_t i=0; i<cameras.size(); ++i)
    {
        //Calculate i-th camera transform
        Scalar halfFov = cameras[i].fovH/Scalar(360)*M_PI;
        Vector3 dir = direction.rotate(cameraUp, offset - accFov - halfFov);
        accFov += Scalar(2)*halfFov;
        
        //Setup camera transformation
        Vector3 rotDirection = rotation * dir;
        SetupCamera(i, rotEyePosition, rotDirection, rotCameraUp);
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
    cameras[index].cam->SetupCamera(eye_, dir_, up_);
}
    
void Multibeam2::InstallNewDataHandler(std::function<void(Multibeam2*)> callback)
{
    newDataCallback = callback;
}
    
void Multibeam2::NewDataReady(unsigned int index)
{
    dataCounter += index;
    int lastIndex = (int)cameras.size()-1;
    int nSum = lastIndex*(lastIndex+1)/2;
    
    if(dataCounter == nSum) //Check if data received from all cameras
    {
        dataCounter = 0;
        
        //Copy (rearange) data from temp to final sonar image
        for(size_t i=0; i<cameras.size(); ++i)
        {
            size_t xoffset = cameras[i].dataOffset/resY;
            
            for(size_t h=0; h<resY; ++h)
            {
                memcpy(&rangeData[xoffset + h*resX],
                       &imageData[cameras[i].dataOffset + h*cameras[i].width],
                       sizeof(GLfloat)*cameras[i].width);
            }
        }
        
        //Call callback
        if(newDataCallback != NULL)
            newDataCallback(this);
    }
}
    
std::vector<Renderable> Multibeam2::Render()
{
    std::vector<Renderable> items(0);
    
    Renderable item;
    item.model = glMatrixFromTransform(getSensorFrame());
    item.type = RenderableType::SENSOR_LINES;
    
    int div = (int)ceil(fovH/5.0);
    GLfloat iconSize = 0.5f;
    GLfloat cosFovV2 = cosf(fovV/360.f*M_PI);
    GLfloat sinFovV2 = sinf(fovV/360.f*M_PI);
    GLfloat r = iconSize/cosFovV2;
    GLfloat thetaDiv = fovH/180.f * M_PI/(GLfloat)div;
    GLfloat offset = -fovH/360.f * M_PI;
    GLfloat y = sinFovV2 * r;
    
    for(int i=0; i<div; ++i)
    {
        GLfloat theta1 = i*thetaDiv + offset;
        GLfloat theta2 = theta1 + thetaDiv;
        GLfloat d1 = cosf(theta1) * r;
        GLfloat d2 = cosf(theta2) * r;
        GLfloat z1 = cosFovV2 * d1;
        GLfloat z2 = cosFovV2 * d2;
        GLfloat x1 = sinf(theta1) * r;
        GLfloat x2 = sinf(theta2) * r;
        
        item.points.push_back(glm::vec3(x1,y,z1));
        item.points.push_back(glm::vec3(x2,y,z2));
        item.points.push_back(glm::vec3(x1,-y,z1));
        item.points.push_back(glm::vec3(x2,-y,z2));
        
        if(i == 0) //End 1
        {
            item.points.push_back(glm::vec3(x1,y,z1));
            item.points.push_back(glm::vec3(x1,-y,z1));
            item.points.push_back(glm::vec3(x1,y,z1));
            item.points.push_back(glm::vec3(0,0,0));
            item.points.push_back(glm::vec3(x1,-y,z1));
            item.points.push_back(glm::vec3(0,0,0));
        }
        else if(i == div-1) //End 2
        {
            item.points.push_back(glm::vec3(x2,y,z2));
            item.points.push_back(glm::vec3(x2,-y,z2));
            item.points.push_back(glm::vec3(x2,y,z2));
            item.points.push_back(glm::vec3(0,0,0));
            item.points.push_back(glm::vec3(x2,-y,z2));
            item.points.push_back(glm::vec3(0,0,0));
        }
    }
    
    items.push_back(item);
    return items;
}
    
}
