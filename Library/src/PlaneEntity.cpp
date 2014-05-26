//
//  PlaneEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "PlaneEntity.h"

#include "SimulationApp.h"

PlaneEntity::PlaneEntity(std::string uniqueName, btScalar planeSize, Material* mat, Look l, const btTransform& worldTransform) : StaticEntity(uniqueName, mat, l)
{
    size = UnitSystem::SetLength(planeSize);
    
    btDefaultMotionState* motionState = new btDefaultMotionState(UnitSystem::SetTransform(worldTransform));
    btCollisionShape* shape = new btStaticPlaneShape(SimulationApp::getApp()->getSimulationManager()->isZAxisUp() ? btVector3(0,0,1.) : btVector3(0,0,-1.),0);
    
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0, motionState, shape, btVector3(0,0,0));
    rigidBodyCI.m_friction = rigidBodyCI.m_rollingFriction = rigidBodyCI.m_restitution = btScalar(1.); //not used
    rigidBody = new btRigidBody(rigidBodyCI);
    rigidBody->setUserPointer(this);
    rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
    
    GLfloat halfSize = (GLfloat)size/2.f;
    int uDiv = (int)floor(size)/10;
    int vDiv = (int)floor(size)/10;
    if(uDiv < 1) uDiv = 1;
    if(vDiv < 1) vDiv = 1;
    bool zUp = SimulationApp::getApp()->getSimulationManager()->isZAxisUp();
    
    displayList = glGenLists(1);
    glNewList(displayList, GL_COMPILE);
    for(int i=0; i<uDiv; i++)
    {
        GLfloat ut0 = (GLfloat)i/(GLfloat)uDiv*(GLfloat)size;
        GLfloat ut1 = (GLfloat)(i+1)/(GLfloat)uDiv *(GLfloat)size;
        GLfloat u0 = -halfSize + ut0;
        GLfloat u1 = -halfSize + ut1;
        
        glBegin(GL_TRIANGLE_STRIP);
        glNormal3f(0, 0, (zUp ? 1.f : -1.f));
        
        for (int h=0; h<=vDiv; h++)
        {
            GLfloat vt = (GLfloat)h/(GLfloat)vDiv*(GLfloat)size;
            GLfloat v = -halfSize + vt;
            
            if(zUp)
            {
                glTexCoord2f(vt, ut1);
                glVertex3f(v, u1, 0);
                glTexCoord2f(vt, ut0);
                glVertex3f(v, u0, 0);
            }
            else
            {
                glTexCoord2f(vt, ut0);
                glVertex3f(v, u0, 0);
                glTexCoord2f(vt, ut1);
                glVertex3f(v, u1, 0);
            }
        }
        glEnd();
    }
    glEndList();
}

PlaneEntity::~PlaneEntity()
{
}

StaticEntityType PlaneEntity::getStaticType()
{
    return STATIC_PLANE;
}
