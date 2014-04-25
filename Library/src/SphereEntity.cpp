//
//  SphereEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "SphereEntity.h"
#include "OpenGLSolids.h"
#include "GeometryUtil.h"

SphereEntity::SphereEntity(std::string uniqueName, btScalar sphereRadius, Material* mat, Look l, bool isStatic) : SolidEntity(uniqueName, mat, isStatic)
{
    radius = UnitSystem::SetLength(sphereRadius);
    
    //Calculate physical properties
    volume = 4.0/3.0*M_PI*radius*radius*radius;
    dragCoeff = btVector3(0.47*M_PI*radius*radius, 0.47*M_PI*radius*radius, 0.47*M_PI*radius*radius);
    mass = volume * material->density;
    Ipri = btVector3(2.0/5.0*mass*radius*radius,
                     2.0/5.0*mass*radius*radius,
                     2.0/5.0*mass*radius*radius);
    
    //Build mesh
    mesh = new TriangleMesh();
    //TriangleFace f;
    //OpenGLNormal gln;
    btVector3 v;
    
    /////VERTICES
    v = btVector3(1,GOLDEN_RATIO,0);
    mesh->vertices.push_back(v);
    v = btVector3(-1,GOLDEN_RATIO,0);
    mesh->vertices.push_back(v);
    v = btVector3(1,-GOLDEN_RATIO,0);
    mesh->vertices.push_back(v);
    v = btVector3(-1,-GOLDEN_RATIO,0);
    mesh->vertices.push_back(v);
    v = btVector3(0,1,GOLDEN_RATIO);
    mesh->vertices.push_back(v);
    v = btVector3(0,-1,GOLDEN_RATIO);
    mesh->vertices.push_back(v);
    v = btVector3(0,1,-GOLDEN_RATIO);
    mesh->vertices.push_back(v);
    v = btVector3(0,-1,-GOLDEN_RATIO);
    mesh->vertices.push_back(v);
    v = btVector3(GOLDEN_RATIO,0,1);
    mesh->vertices.push_back(v);
    v = btVector3(-GOLDEN_RATIO,0,1);
    mesh->vertices.push_back(v);
    v = btVector3(GOLDEN_RATIO,0,-1);
    mesh->vertices.push_back(v);
    v = btVector3(-GOLDEN_RATIO,0,-1);
    mesh->vertices.push_back(v);
    
    SetLook(l);
}

SphereEntity::~SphereEntity()
{
}

SolidEntityType SphereEntity::getSolidType()
{
    return SPHERE;
}

btCollisionShape* SphereEntity::BuildCollisionShape()
{
    return new btSphereShape(radius);
}

void SphereEntity::BuildDisplayList()
{
    if(displayList != 0)
        glDeleteLists(displayList, 1);
    
    displayList = glGenLists(1);
    glNewList(displayList, GL_COMPILE);
    DrawSolidSphere(radius);
    glEndList();
}

void SphereEntity::BuildCollisionList()
{
    if(collisionList != 0)
        glDeleteLists(collisionList, 1);
    
    collisionList = glGenLists(1);
    glNewList(collisionList, GL_COMPILE);
    DrawPointSphere(radius);
    glEndList();
}

void SphereEntity::CalculateFluidDynamics(const btVector3& surfaceN, const btVector3&surfaceD, const btVector3&fluidV, const Fluid* fluid,
                                          btScalar& submergedVolume, btVector3& cob,  btVector3& drag, btVector3& angularDrag,
                                          btTransform* worldTransform, const btVector3& velocity,
                                          const btVector3& angularVelocity)
{
    //0.Initialize
    cob = btVector3(0,0,0);
    submergedVolume = 0;
    drag = btVector3(0,0,0);
    angularDrag = btVector3(0,0,0);
    
    btTransform solidTrans;
    btVector3 linVelocity;
    btVector3 angVelocity;
    
    if(worldTransform != NULL)
    {
        solidTrans = btTransform(*worldTransform);
        linVelocity = velocity;
        angVelocity = angularVelocity;
    }
    else if(rigidBody != NULL)
    {
        solidTrans = rigidBody->getWorldTransform();
        linVelocity = rigidBody->getLinearVelocity();
        angVelocity = rigidBody->getAngularVelocity();
    }
    else
        return;
    
    //1.Transform plane into rigid body local coordinate system
    btVector3 localSurfaceN = solidTrans.getBasis().inverse() * surfaceN;
    btVector3 localSurfaceD = solidTrans.inverse() * surfaceD;
    btVector3 localFluidV = solidTrans.getBasis().inverse() * (fluidV - linVelocity);
    //btVector3 localAngularV = solidTrans.getBasis().inverse() * angVelocity;
    
    //2.Calculate distance from sphere center to plane
    btVector3 sphereCenter = -localSurfaceD;
    btScalar d = distanceFromCenteredPlane(localSurfaceN, sphereCenter);
    
    //3.Calculate submerged volume
    if(d <= -radius) //fully submerged
    {
        submergedVolume = volume;
        drag = localFluidV*localFluidV.length()*0.5*0.5*M_PI*radius*radius;
    }
    else if(fabsf(d) < radius) //partially submerged
    {
        btScalar h = radius - d;
        submergedVolume = M_PI*h*h/3.0*(3*radius-h);
        cob = -localSurfaceN*3*(2*radius-h)*(2*radius-h)/(4*(3*radius-h));
        
        //btScalar r = d < 0 ? radius : sqrt(h*(2*radius - h));
        //drag = -localFluidV*localFluidV*0.5*0.5*M_PI*r*r;
    }
    
    drag = solidTrans.getBasis()*drag;
    centerOfBuoyancy = cob;
}