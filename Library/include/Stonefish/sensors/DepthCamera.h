//
//  DepthCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 07/05/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_DepthCamera__
#define __Stonefish_DepthCamera__

#include <functional>
#include "Camera.h"
#include <graphics/OpenGLDepthCamera.h>

class DepthCamera : public Camera
{
public:
    DepthCamera(std::string uniqueName, uint32_t resX, uint32_t resY, btScalar horizFOVDeg, btScalar minDepth, btScalar maxDepth, const btTransform& geomToSensor, SolidEntity* attachment = NULL, btScalar frequency = btScalar(-1.));
    virtual ~DepthCamera();
    
    void SetupCamera(const btVector3& eye, const btVector3& dir, const btVector3& up);
    void InstallNewDataHandler(std::function<void(DepthCamera*)> callback);
    void NewDataReady();
    GLfloat* getDataPointer(); //Should be only used in the callback
    glm::vec2 getDepthRange();
     
    //Sensor
	void InternalUpdate(btScalar dt);

private:
    OpenGLDepthCamera* glCamera;
    GLfloat* imageData;
    glm::vec2 depthRange;
    std::function<void(DepthCamera*)> newDataCallback;
};

#endif