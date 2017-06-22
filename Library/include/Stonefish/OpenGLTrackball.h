//
//  OpenGLTrackball.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLTrackball__
#define __Stonefish_OpenGLTrackball__

#include "OpenGLView.h"

class OpenGLTrackball : public OpenGLView
{
public:
    OpenGLTrackball(const btVector3& centerPosition, btScalar orbitRadius, const btVector3& up, GLint originX, GLint originY, GLint width, GLint height, GLfloat fov, GLfloat horizon, bool sao = false);
    ~OpenGLTrackball();
    
	void Rotate(const btQuaternion& rot);
    void GlueToEntity(SolidEntity* solid);
	void MouseDown(GLfloat x, GLfloat y);
    void MouseMove(GLfloat x, GLfloat y);
    void MouseUp();
    void MouseScroll(GLfloat s);
    
    glm::mat4 GetViewTransform();
    glm::vec3 GetEyePosition();
    glm::vec3 GetLookingDirection();
    glm::vec3 GetUpDirection();
    ViewType getType();
    
private:
    void UpdateTrackballTransform();
    GLfloat calculateZ(GLfloat x, GLfloat y);
	
    SolidEntity* holdingEntity;
    
	glm::mat4 trackballTransform;
    glm::quat rotation;
    glm::vec3 center;
    glm::vec3 up;
	GLfloat radius;
    
	//Mouse interaction
	GLfloat x_start, y_start, z_start;
	glm::quat rotation_start;
	bool dragging;
};


#endif