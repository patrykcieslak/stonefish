//
//  Camera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Camera__
#define __Stonefish_Camera__

#include "Sensor.h"
#include "OpenGLCamera.h"

class Camera : public Sensor
{
public:
    Camera(std::string uniqueName, const btVector3& eyePosition, const btVector3& targetPosition, const btVector3& cameraUp, 
           GLint originX, GLint originY, GLint width, GLint height, GLfloat fov, btScalar frequency = btScalar(-1.), bool advancedRendering = true);
    virtual ~Camera();
    
	virtual void InternalUpdate(btScalar dt);
    virtual std::vector<Renderable> Render();
	virtual SensorType getType();

private:
    OpenGLCamera* glCamera;
};

#endif