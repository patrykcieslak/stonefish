//
//  PlaneEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "PlaneEntity.h"
#include "OpenGLUtil.h"


PlaneEntity::PlaneEntity(std::string uniqueName, btScalar size, Material* mat, Look l, const btTransform& worldTransform) : Entity(uniqueName)
{
    material = mat;
    look = l;
    size = UnitSystem::SetLength(size);
    
    btDefaultMotionState* motionState = new btDefaultMotionState(UnitSystem::SetTransform(worldTransform));
    btCollisionShape* shape = new btStaticPlaneShape(btVector3(0,0,1.0),0);
    
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0, motionState, shape, btVector3(0,0,0));
    rigidBodyCI.m_friction = rigidBodyCI.m_rollingFriction = rigidBodyCI.m_restitution = btScalar(1.); //not used
    rigidBody = new btRigidBody(rigidBodyCI);
    rigidBody->setUserPointer(this);
    rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
    
    GLfloat halfSize = size/2.f;
    GLfloat texCoord = size;
    
    displayList = glGenLists(1);
    glNewList(displayList, GL_COMPILE);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1.f);
    
    glTexCoord2f(0, 0);
    glVertex3f(-halfSize, -halfSize, 0);
    
    glTexCoord2f(texCoord, 0);
    glVertex3f(halfSize, -halfSize, 0);
    
    glTexCoord2f(texCoord, texCoord);
    glVertex3f(halfSize, halfSize, 0);
    
    glTexCoord2f(0, texCoord);
    glVertex3f(-halfSize, halfSize, 0);
    
    /*glNormal3f(0, 0, -1.f);
    
    glTexCoord2f(0, 0);
    glVertex3f(-halfSize, -halfSize, 0);
    
    glTexCoord2f(texCoord, 0);
    glVertex3f(halfSize, -halfSize, 0);
    
    glTexCoord2f(texCoord, texCoord);
    glVertex3f(halfSize, halfSize, 0);
    
    glTexCoord2f(0, texCoord);
    glVertex3f(-halfSize, halfSize, 0);*/
    
    glEnd();
    
    glEndList();
}

PlaneEntity::~PlaneEntity()
{
    if(displayList != 0)
        glDeleteLists(displayList, 1);
    if(look.texture != 0)
        glDeleteTextures(1, &look.texture);
    
    material = NULL;
}

EntityType PlaneEntity::getType()
{
    return PLANE;
}

void PlaneEntity::Render()
{
    if(isRenderable())
    {
        btTransform trans;
        btScalar openglTrans[16];
        rigidBody->getMotionState()->getWorldTransform(trans);
        trans.getOpenGLMatrix(openglTrans);
        
        glPushMatrix();
#ifdef BT_USE_DOUBLE_PRECISION
        glMultMatrixd(openglTrans);
#else
        glMultMatrixf(openglTrans);
#endif
        
        UseLook(look);
        glCallList(displayList);
        glPopMatrix();
    }
}

btTransform PlaneEntity::getTransform()
{
    btTransform trans;
    rigidBody->getMotionState()->getWorldTransform(trans);
    return trans;
}

void PlaneEntity::setTransform(const btTransform &trans)
{
    btDefaultMotionState* motionState = new btDefaultMotionState(trans);
    rigidBody->setMotionState(motionState);
}

Material* PlaneEntity::getMaterial()
{
    return material;
}

void PlaneEntity::AddToDynamicsWorld(btDynamicsWorld *world)
{
    world->addRigidBody(rigidBody, DEFAULT, DEFAULT | CABLE_EVEN | CABLE_ODD);
}

btRigidBody* PlaneEntity::getRigidBody()
{
    return rigidBody;
}

void PlaneEntity::SetLook(Look newLook)
{
    if(look.texture != 0)
        glDeleteTextures(1, &look.texture);
    
    look = newLook;
}