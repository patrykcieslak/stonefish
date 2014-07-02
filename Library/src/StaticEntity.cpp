//
//  StaticEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "StaticEntity.h"

StaticEntity::StaticEntity(std::string uniqueName, Material* mat, Look l) : Entity(uniqueName)
{
    material = mat;
    look = l;
    wireframe = false;
}

StaticEntity::~StaticEntity()
{
    if(displayList != 0)
        glDeleteLists(displayList, 1);
    if(look.texture != 0)
        glDeleteTextures(1, &look.texture);
        
    material = NULL;
}

EntityType StaticEntity::getType()
{
    return ENTITY_STATIC;
}

void StaticEntity::GetAABB(btVector3& min, btVector3& max)
{
    rigidBody->getAabb(min, max);
}

void StaticEntity::Render()
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
        if(wireframe)
        {
            glPolygonMode(GL_FRONT, GL_LINE);
            glCallList(displayList);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else
        {
            glCallList(displayList);
        }
        glPopMatrix();
    }
}

Material* StaticEntity::getMaterial()
{
    return material;
}

void StaticEntity::AddToDynamicsWorld(btMultiBodyDynamicsWorld *world)
{
    world->addRigidBody(rigidBody, MASK_STATIC, MASK_DEFAULT);
}

btRigidBody* StaticEntity::getRigidBody()
{
    return rigidBody;
}

void StaticEntity::SetLook(Look newLook)
{
    if(look.texture != 0)
        glDeleteTextures(1, &look.texture);
    
    look = newLook;
}

void StaticEntity::SetWireframe(bool enabled)
{
    wireframe = enabled;
}