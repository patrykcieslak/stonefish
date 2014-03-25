//
//  OpenGLTrackball.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLTrackball__
#define __Stonefish_OpenGLTrackball__

#include "OpenGLView.h"

class OpenGLTrackball : public OpenGLView
{
public:
    OpenGLTrackball(const btVector3& centerPosition, btScalar orbitRadius, const btVector3& up, GLint originX, GLint originY, GLint width, GLint height, GLuint ssaoSize, GLfloat fov);
    ~OpenGLTrackball();
    
    btTransform GetViewTransform();
    btVector3 GetEyePosition();
    btVector3 GetLookingDirection();
    void Rotate(const btQuaternion& rot);
    ViewType getType();
    
    void MouseDown(GLfloat x, GLfloat y);
    void MouseMove(GLfloat x, GLfloat y);
    void MouseUp();
    void MouseScroll(GLfloat s);
    
private:
    btQuaternion rotation;
    btVector3 center;
    btScalar radius;
    btVector3 up;
};


#endif