//
//  OpenGLCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/12/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "OpenGLCamera.h"
#include "GeometryUtil.hpp"

OpenGLCamera::OpenGLCamera(const btVector3& eyePosition, const btVector3& targetPosition, const btVector3& cameraUp, GLint x, GLint y, GLint width, GLint height, GLfloat fov, GLfloat horizon, bool sao) : OpenGLView(x, y, width, height, horizon, sao)
{
    btVector3 _eye = UnitSystem::SetPosition(eyePosition);
	btVector3 _dir = UnitSystem::SetPosition(targetPosition - eyePosition);
	
	eye = glm::vec3((GLfloat)_eye.getX(), (GLfloat)_eye.getY(), (GLfloat)_eye.getZ());
    dir = glm::normalize(glm::vec3((GLfloat)_dir.getX(), (GLfloat)_dir.getY(), (GLfloat)_dir.getZ()));
	up = glm::normalize(glm::vec3((GLfloat)cameraUp.getX(), (GLfloat)cameraUp.getY(), (GLfloat)cameraUp.getZ()));
    lookingDir = dir;
    
	pan = 0;
    tilt = 0;
    fovx = UnitSystem::SetAngle(fov);
    
    holdingEntity = NULL;
    
    GLfloat aspect = (GLfloat)viewportWidth/(GLfloat)viewportHeight;
    GLfloat fovy = fovx/aspect;
    projection = glm::perspective(fovy, aspect, near, far);
    
    SetupCamera();
}

OpenGLCamera::~OpenGLCamera()
{
    holdingEntity = NULL;
}

ViewType OpenGLCamera::getType()
{
    return CAMERA;
}

void OpenGLCamera::MoveCamera(const btVector3& move)
{
	btVector3 _move = UnitSystem::SetPosition(move);
    eye = eye + glm::vec3((GLfloat)_move.getX(), (GLfloat)_move.getY(), (GLfloat)_move.getZ());  
    SetupCamera();
}

void OpenGLCamera::MoveCamera(btScalar step)
{
    eye = eye + lookingDir * UnitSystem::SetLength(step);
    SetupCamera();
}
void OpenGLCamera::RotateCamera(btScalar panStep, btScalar tiltStep)
{
    pan += UnitSystem::SetAngle(panStep);
    tilt += UnitSystem::SetAngle(tiltStep);
    SetupCamera();
}

void OpenGLCamera::setPanAngle(GLfloat newPanAngle)
{
    newPanAngle = UnitSystem::SetAngle(newPanAngle);
    pan = newPanAngle;
    SetupCamera();
}

void OpenGLCamera::setTiltAngle(GLfloat newTiltAngle)
{
    newTiltAngle = UnitSystem::SetAngle(newTiltAngle);
    tilt = newTiltAngle;
    SetupCamera();
}


GLfloat OpenGLCamera::getPanAngle()
{
    return UnitSystem::GetAngle(pan);
}

GLfloat OpenGLCamera::getTiltAngle()
{
    return UnitSystem::GetAngle(tilt);
}

glm::vec3 OpenGLCamera::GetEyePosition()
{
    if(holdingEntity != NULL)
    {
		glm::mat4 trans = glMatrixFromBtTransform(holdingEntity->getTransform());
        return glm::vec3(trans * glm::vec4(eye,1.f));
    }
    else
        return eye;
}

glm::vec3 OpenGLCamera::GetLookingDirection()
{
    if(holdingEntity != NULL)
    {
		glm::mat4 trans = glMatrixFromBtTransform(holdingEntity->getTransform());
        return glm::normalize(glm::mat3(trans) * lookingDir);
    }
    else
        return lookingDir;
}

glm::vec3 OpenGLCamera::GetUpDirection()
{
    if(holdingEntity != NULL)
    {
		glm::mat4 trans = glMatrixFromBtTransform(holdingEntity->getTransform());
        return glm::normalize(glm::mat3(trans) * up);
    }
    else
        return up;
}

void OpenGLCamera::GlueToEntity(SolidEntity *ent)
{
    holdingEntity = ent;
}

void OpenGLCamera::SetupCamera()
{
    lookingDir = dir;
    
    //additional camera rotation
    glm::vec3 tiltAxis = glm::normalize(glm::cross(dir, up));
    glm::vec3 panAxis = glm::normalize(glm::cross(tiltAxis, dir));
    
    //rotate
	lookingDir = glm::rotate(lookingDir, tilt, tiltAxis);
	lookingDir = glm::rotate(lookingDir, pan, panAxis);
    lookingDir = glm::normalize(lookingDir);
    
    glm::vec3 newUp = glm::rotate(panAxis, tilt, tiltAxis);
    newUp = glm::rotate(newUp, pan, panAxis);
	newUp = glm::normalize(newUp);
    
	cameraTransform = glm::lookAt(eye, eye+lookingDir, newUp);
	cameraRender = glm::inverse(cameraTransform);
}

glm::mat4 OpenGLCamera::GetViewTransform()
{
    if(holdingEntity != NULL)
    {
		/////CHECK AND FIX////////////
        //btTransform entTrans = holdingEntity->getTransform();
        /*btTransform trans =  cameraTransform * entTrans.inverse();
        btVector3 translate = entTrans.getBasis() * eye;
        trans.getOrigin() = trans.getOrigin() - translate;
        return trans;*/
		glm::mat4 entTrans = glMatrixFromBtTransform(holdingEntity->getTransform());
		glm::mat4 trans = cameraTransform * glm::inverse(entTrans);
		return trans * glm::translate(eye);
    }
    else
        return cameraTransform;
}

void OpenGLCamera::RenderDummy()
{
    glm::mat4 model;
	
    //transformation
    if(holdingEntity != NULL)
    {
        btTransform trans = holdingEntity->getTransform();
		model = glMatrixFromBtTransform(trans);
    }
    
    model = glm::translate(model, eye);
	model *= cameraRender;
   
    //rendering
    GLfloat iconSize = 5.f;
    GLfloat x = iconSize*tanf(fovx/2.f);
    GLfloat aspect = (GLfloat)viewportWidth/(GLfloat)viewportHeight;
    GLfloat y = x/aspect;
	
	std::vector<glm::vec3> vertices;
	vertices.push_back(glm::vec3(0,0,0));
	vertices.push_back(glm::vec3(-x,y,-iconSize));
	vertices.push_back(glm::vec3(0,0,0));
	vertices.push_back(glm::vec3(x,y,-iconSize));
	vertices.push_back(glm::vec3(0,0,0));
	vertices.push_back(glm::vec3(-x,-y,-iconSize));
	vertices.push_back(glm::vec3(0,0,0));
	vertices.push_back(glm::vec3(x,-y,-iconSize));
	
	vertices.push_back(glm::vec3(-x,y,-iconSize));
	vertices.push_back(glm::vec3(x,y,-iconSize));
	vertices.push_back(glm::vec3(x,y,-iconSize));
	vertices.push_back(glm::vec3(x,-y,-iconSize));
	vertices.push_back(glm::vec3(x,-y,-iconSize));
	vertices.push_back(glm::vec3(-x,-y,-iconSize));
	vertices.push_back(glm::vec3(-x,-y,-iconSize));
	vertices.push_back(glm::vec3(-x,y,-iconSize));
	OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINES, vertices, DUMMY_COLOR, model);
}
