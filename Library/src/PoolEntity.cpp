//
//  PoolEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright(c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "PoolEntity.h"
#include "OpenGLSolids.h"

PoolEntity::PoolEntity(std::string uniqueName, btScalar extent1, btScalar extent2, btScalar ddepth, const btTransform& worldTransform, Fluid* fld, Material* wallMat, Look wallLook) : FluidEntity(uniqueName, fld)
{
    material = wallMat;
    look = wallLook;
    depth = UnitSystem::SetLength(ddepth);
    btVector3 halfExtents = btVector3(UnitSystem::SetLength(extent1/2.0), UnitSystem::SetLength(extent2/2.0), depth/2.0);
    ghost->setWorldTransform(UnitSystem::SetTransform(worldTransform));
    ghost->setCollisionShape(new btBoxShape(halfExtents));
    
    surfaceDisplayList = glGenLists(1);
    glNewList(surfaceDisplayList, GL_COMPILE);
    glColor4f(1.f,1.f,1.f,1.f);
    glNormal3f(0, 0, -1.f);
    glBegin(GL_QUADS);
    glVertex3f(-halfExtents.x(), -halfExtents.y(), -halfExtents.z());
    glVertex3f(-halfExtents.x(), halfExtents.y(), -halfExtents.z());
    glVertex3f(halfExtents.x(), halfExtents.y(), -halfExtents.z());
    glVertex3f(halfExtents.x(), -halfExtents.y(), -halfExtents.z());
    glEnd();
    glEndList();
    
    float wallGrow = 1.1f;
    
    volumeDisplayList = glGenLists(1);
    glNewList(volumeDisplayList, GL_COMPILE);
    
    //Bottom
    glBegin(GL_QUADS);
    glVertex3f(-halfExtents.x(), -halfExtents.y(), halfExtents.z());
    glVertex3f(-halfExtents.x(), halfExtents.y(), halfExtents.z());
    glVertex3f(halfExtents.x(), halfExtents.y(), halfExtents.z());
    glVertex3f(halfExtents.x(), -halfExtents.y(), halfExtents.z());
    
    glVertex3f(halfExtents.x()*wallGrow, -halfExtents.y()*wallGrow, halfExtents.z()*wallGrow);
    glVertex3f(halfExtents.x()*wallGrow, halfExtents.y()*wallGrow, halfExtents.z()*wallGrow);
    glVertex3f(-halfExtents.x()*wallGrow, halfExtents.y()*wallGrow, halfExtents.z()*wallGrow);
    glVertex3f(-halfExtents.x()*wallGrow, -halfExtents.y()*wallGrow, halfExtents.z()*wallGrow);
    glEnd();
    
    //Inside
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(-halfExtents.x(), halfExtents.y(), -halfExtents.z());
    glVertex3f(-halfExtents.x(), halfExtents.y(), halfExtents.z());
    
    glVertex3f(-halfExtents.x(), -halfExtents.y(), -halfExtents.z());
    glVertex3f(-halfExtents.x(), -halfExtents.y(), halfExtents.z());
    
    glVertex3f(halfExtents.x(), -halfExtents.y(), -halfExtents.z());
    glVertex3f(halfExtents.x(), -halfExtents.y(), halfExtents.z());
    
    glVertex3f(halfExtents.x(), halfExtents.y(), -halfExtents.z());
    glVertex3f(halfExtents.x(), halfExtents.y(), halfExtents.z());
    
    glVertex3f(-halfExtents.x(), halfExtents.y(), -halfExtents.z());
    glVertex3f(-halfExtents.x(), halfExtents.y(), halfExtents.z());
    glEnd();
    
    //Outside
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(-halfExtents.x()*wallGrow, halfExtents.y()*wallGrow, halfExtents.z()*wallGrow);
    glVertex3f(-halfExtents.x()*wallGrow, halfExtents.y()*wallGrow, -halfExtents.z());
    
    glVertex3f(-halfExtents.x()*wallGrow, -halfExtents.y()*wallGrow, halfExtents.z()*wallGrow);
    glVertex3f(-halfExtents.x()*wallGrow, -halfExtents.y()*wallGrow, -halfExtents.z());
    
    glVertex3f(halfExtents.x()*wallGrow, -halfExtents.y()*wallGrow, halfExtents.z()*wallGrow);
    glVertex3f(halfExtents.x()*wallGrow, -halfExtents.y()*wallGrow, -halfExtents.z());
    
    glVertex3f(halfExtents.x()*wallGrow, halfExtents.y()*wallGrow, halfExtents.z()*wallGrow);
    glVertex3f(halfExtents.x()*wallGrow, halfExtents.y()*wallGrow, -halfExtents.z());
    
    glVertex3f(-halfExtents.x()*wallGrow, halfExtents.y()*wallGrow, halfExtents.z()*wallGrow);
    glVertex3f(-halfExtents.x()*wallGrow, halfExtents.y()*wallGrow, -halfExtents.z());
    glEnd();
    
    //Top
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(-halfExtents.x()*wallGrow, -halfExtents.y()*wallGrow, -halfExtents.z());
    glVertex3f(-halfExtents.x(), -halfExtents.y(), -halfExtents.z());
    
    glVertex3f(halfExtents.x()*wallGrow, -halfExtents.y()*wallGrow, -halfExtents.z());
    glVertex3f(halfExtents.x(), -halfExtents.y(), -halfExtents.z());
    
    glVertex3f(halfExtents.x()*wallGrow, halfExtents.y()*wallGrow, -halfExtents.z());
    glVertex3f(halfExtents.x(), halfExtents.y(), -halfExtents.z());
    
    glVertex3f(-halfExtents.x()*wallGrow, halfExtents.y()*wallGrow, -halfExtents.z());
    glVertex3f(-halfExtents.x(), halfExtents.y(), -halfExtents.z());
    
    glVertex3f(-halfExtents.x()*wallGrow, -halfExtents.y()*wallGrow, -halfExtents.z());
    glVertex3f(-halfExtents.x(), -halfExtents.y(), -halfExtents.z());
    glEnd();
    glEndList();
}

PoolEntity::PoolEntity(std::string uniqueName, btScalar rradius, btScalar ddepth, const btTransform& worldTransform, Fluid* fld) : FluidEntity(uniqueName, fld)
{
    depth = UnitSystem::SetLength(ddepth);
    btScalar radius = UnitSystem::SetLength(rradius);
    ghost->setWorldTransform(UnitSystem::SetTransform(worldTransform));
    ghost->setCollisionShape(new btCylinderShapeZ(btVector3(radius,radius,depth/2.0)));
    
    surfaceDisplayList = glGenLists(1);
    glNewList(surfaceDisplayList, GL_COMPILE);
    glNormal3f(0, 0, -1.f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, 0, -depth/2.f);
    for(int i=0; i<= CIRCULAR_TANK_DIV; i++)
        glVertex3f(radius*sinf(((GLfloat)i/(GLfloat)CIRCULAR_TANK_DIV)*2.f*M_PI), radius*cosf(((GLfloat)i/(GLfloat)CIRCULAR_TANK_DIV)*2.f*M_PI), -depth/2.f);
    glEnd();
    glEndList();
    
    volumeDisplayList = glGenLists(1);
    glNewList(volumeDisplayList, GL_COMPILE);
    glNormal3f(0, 0, 1.f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, 0, depth/2.f);
    for(int i=0; i<= CIRCULAR_TANK_DIV; i++)
        glVertex3f(radius*sinf(((GLfloat)i/(GLfloat)CIRCULAR_TANK_DIV)*2.f*M_PI), radius*cosf(((GLfloat)i/(GLfloat)CIRCULAR_TANK_DIV)*2.f*M_PI), depth/2.f);
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<=CIRCULAR_TANK_DIV; i++)
    {
        glNormal3f(-sinf(((GLfloat)i/(GLfloat)CIRCULAR_TANK_DIV)*2.f*M_PI), -cosf(((GLfloat)i/(GLfloat)CIRCULAR_TANK_DIV)*2.f*M_PI), 0.f);
        glVertex3f(radius*sinf(((GLfloat)i/(GLfloat)CIRCULAR_TANK_DIV)*2.f*M_PI), radius*cosf(((GLfloat)i/(GLfloat)CIRCULAR_TANK_DIV)*2.f*M_PI), -depth/2.f);
        glVertex3f(radius*sinf(((GLfloat)i/(GLfloat)CIRCULAR_TANK_DIV)*2.f*M_PI), radius*cosf(((GLfloat)i/(GLfloat)CIRCULAR_TANK_DIV)*2.f*M_PI), depth/2.f);
    }
    glEnd();
    glEndList();
}

PoolEntity::~PoolEntity()
{
}

bool PoolEntity::IsInsideFluid(const btVector3 &point)
{
    btTransform trans = ghost->getWorldTransform();
    btVector3 localPoint = trans.inverse()*point;
    
    if(ghost->getCollisionShape()->getShapeType() == CYLINDER_SHAPE_PROXYTYPE)
    {
        btVector3 halfExtents = ((btBoxShape*)ghost->getCollisionShape())->getHalfExtentsWithMargin();
        btVector3 circle = localPoint;
        circle.setZ(0);
        
        if((circle.length() <= halfExtents.getX()) && (btFabs(localPoint.z()) <= halfExtents.getZ()))
            return true;
    }
    else //box
    {
        btVector3 halfExtents = ((btBoxShape*)ghost->getCollisionShape())->getHalfExtentsWithMargin();
        if((btFabs(localPoint.x()) <= halfExtents.x()) && (btFabs(localPoint.y()) <= halfExtents.y()) && (btFabs(localPoint.z()) <= halfExtents.z()))
           return true;
    }
    
    return false;
}

btScalar PoolEntity::GetPressure(const btVector3& point)
{
    btTransform trans = ghost->getWorldTransform();
    btVector3 localPoint = trans.inverse()*point;
    btScalar g = 9.81;
    btScalar d = depth/btScalar(2)+localPoint.z();
    
    btScalar pressure = d > btScalar(0) ? d*fluid->density*g : btScalar(0);
    return pressure;
}

void PoolEntity::Render()
{
    btTransform trans = ghost->getWorldTransform();
    btScalar openglTrans[16];
    trans.getOpenGLMatrix(openglTrans);
    
    glPushMatrix();
#ifdef BT_USE_DOUBLE_PRECISION
    glMultMatrixd(openglTrans);
#else
    glMultMatrixf(openglTrans);
#endif
    UseLook(look);
    glCallList(volumeDisplayList);
    glPopMatrix();
}