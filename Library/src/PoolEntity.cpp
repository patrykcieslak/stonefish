//
//  PoolEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "PoolEntity.h"
#include "SolidEntity.h"
#include "CableEntity.h"
#include "OpenGLSolids.h"

PoolEntity::PoolEntity(std::string uniqueName, btScalar extent1, btScalar extent2, btScalar ddepth, const btTransform& worldTransform, Fluid* fld) : FluidEntity(uniqueName, fld)
{
    fluidVelocity = btVector3(0,0,0);
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
    
    //volumeDisplayList
    
    
    
    glEndList();
}

PoolEntity::PoolEntity(std::string uniqueName, btScalar rradius, btScalar ddepth, const btTransform& worldTransform, Fluid* fld) : FluidEntity(uniqueName, fld)
{
    fluidVelocity = btVector3(0,0,0);
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
        
        if((circle.length() <= halfExtents.getX()) && (fabsf(localPoint.z()) <= halfExtents.getZ()))
            return true;
    }
    else //box
    {
        btVector3 halfExtents = ((btBoxShape*)ghost->getCollisionShape())->getHalfExtentsWithMargin();
        if((fabsf(localPoint.x()) <= halfExtents.x()) && (fabsf(localPoint.y()) <= halfExtents.y()) && (fabsf(localPoint.z()) <= halfExtents.z()))
           return true;
    }
    
    return false;
}

void PoolEntity::ApplyFluidForces(btDynamicsWorld* world, btCollisionObject* co)
{
    //1.Check if object is an Entity
    btRigidBody* rb = btRigidBody::upcast(co);
    if(rb == NULL || rb->isStaticObject())
        return;
    
    Entity* ent = (Entity*)rb->getUserPointer();
    if(ent == NULL) 
        return;
    
    //2.Determine fluid surface coordinates
    btVector3 surfaceN, surfaceD;
    GetSurface(surfaceN, surfaceD);
    
    //3.Calculate fluid forces and buoyancy center based on entity type
    btVector3 cob;
    btScalar submergedV = 0;
    btVector3 dForce;
    btVector3 adTorque;
    
    switch (ent->getType())
    {
        case ENTITY_SOLID:
        {
            SolidEntity* solid = (SolidEntity*)ent;
            solid->CalculateFluidDynamics(surfaceN, surfaceD, fluidVelocity, fluid, submergedV, cob, dForce, adTorque);
            rb->applyForce(-world->getGravity()*submergedV*fluid->density, cob);
            rb->applyCentralForce(dForce);
            rb->applyTorque(adTorque);
            rb->setDamping(submergedV/solid->getVolume()*fluid->viscousity, submergedV/solid->getVolume()*fluid->viscousity);
        }
        break;
    
        case ENTITY_CABLE:
        {
            CableEntity* cable = (CableEntity*)ent;
            cable->CalculateFluidDynamics(surfaceN, surfaceD, fluidVelocity, fluid, submergedV, cob, dForce, adTorque, rb->getWorldTransform(), rb->getLinearVelocity(), rb->getAngularVelocity());
            rb->applyForce(-world->getGravity()*submergedV*fluid->density, cob);
            rb->applyCentralForce(dForce);
            rb->applyTorque(adTorque);
            rb->setDamping(submergedV/cable->getPartVolume()*fluid->viscousity, submergedV/cable->getPartVolume()*fluid->viscousity);
        }
            break;
            
        default:
            return;
    }
}