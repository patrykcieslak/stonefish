//
//  OpenGLTrackball.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLTrackball__
#define __Stonefish_OpenGLTrackball__

#include "graphics/OpenGLCamera.h"

namespace sf
{

class OpenGLTrackball : public OpenGLCamera
{
public:
    OpenGLTrackball(const btVector3& centerPosition, btScalar orbitRadius, const btVector3& up, GLint originX, GLint originY, GLint width, GLint height, GLfloat fov, GLfloat horizon, GLuint spp = 1, bool ao = false);
    ~OpenGLTrackball();
    
	void Rotate(const btQuaternion& rot);
    void MoveCenter(const glm::vec3& step);
    void GlueToEntity(SolidEntity* solid);
	void MouseDown(GLfloat x, GLfloat y, bool translate);
    void MouseMove(GLfloat x, GLfloat y);
    void MouseUp();
    void MouseScroll(GLfloat s);
    
    glm::mat4 GetViewMatrix() const;
    glm::vec3 GetEyePosition() const;
    glm::vec3 GetLookingDirection() const;
    glm::vec3 GetUpDirection() const;
    ViewType getType();
    bool needsUpdate();
    
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
    glm::vec3 translation_start;
	bool dragging;
    bool transMode;
};

}

#endif
