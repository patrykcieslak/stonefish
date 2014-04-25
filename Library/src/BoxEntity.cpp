//
//  BoxEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "BoxEntity.h"
#include "OpenGLSolids.h"

BoxEntity::BoxEntity(std::string uniqueName, const btVector3& dimensions, Material* mat, Look l, bool isStatic) : SolidEntity(uniqueName, mat, isStatic)
{
    halfExtents = UnitSystem::SetPosition(dimensions * btScalar(0.5));
    
    //Calculate physical properties
    volume = halfExtents.x()*halfExtents.y()*halfExtents.z()*8;
    dragCoeff = btVector3(halfExtents.y()*halfExtents.z()*4*1.05, halfExtents.x()*halfExtents.z()*4*1.05, halfExtents.y()*halfExtents.x()*4*1.05);
    mass = volume * material->density;
    Ipri = btVector3(1.0/12.0*mass*((halfExtents.y()*2)*(halfExtents.y()*2)+(halfExtents.z()*2)*(halfExtents.z()*2)),
                     1.0/12.0*mass*((halfExtents.x()*2)*(halfExtents.x()*2)+(halfExtents.z()*2)*(halfExtents.z()*2)),
                     1.0/12.0*mass*((halfExtents.x()*2)*(halfExtents.x()*2)+(halfExtents.y()*2)*(halfExtents.y()*2)));
    
    //Build mesh
    mesh = new TriangleMesh();
    TriangleFace f;
    OpenGLNormal gln;
    btVector3 v;
    
    /////VERTICES
    v = btVector3(-halfExtents.x(), -halfExtents.y(), -halfExtents.z());
    mesh->vertices.push_back(v);
    v = btVector3(-halfExtents.x(), halfExtents.y(), -halfExtents.z());
    mesh->vertices.push_back(v);
    v = btVector3(halfExtents.x(), halfExtents.y(), -halfExtents.z());
    mesh->vertices.push_back(v);
    v = btVector3(halfExtents.x(), -halfExtents.y(), -halfExtents.z());
    mesh->vertices.push_back(v);
    v = btVector3(halfExtents.x(), halfExtents.y(), halfExtents.z());
    mesh->vertices.push_back(v);
    v = btVector3(halfExtents.x(), -halfExtents.y(), halfExtents.z());
    mesh->vertices.push_back(v);
    v = btVector3(-halfExtents.x(), -halfExtents.y(), halfExtents.z());
    mesh->vertices.push_back(v);
    v = btVector3(-halfExtents.x(), halfExtents.y(), halfExtents.z());
    mesh->vertices.push_back(v);
    
    /////FRONT
    //normal
    mesh->normals.push_back(btVector3(0,0,-1));
    gln.x = 0;
    gln.y = 0;
    gln.z = -1.f;
    mesh->glnormals.push_back(gln);
    //faces
    f.vertexIndex[0] = 0;
    f.vertexIndex[1] = 1;
    f.vertexIndex[2] = 2;
    f.normalIndex = 0;
    f.glnormalIndex[0] = f.glnormalIndex[1] = f.glnormalIndex[2] = 0;
    f.uvIndex[0] = f.uvIndex[1] = f.uvIndex[2] = 0;
    f.neighbourIndex[0] = f.neighbourIndex[1] = f.neighbourIndex[2] = -1;
    mesh->faces.push_back(f);
    f.vertexIndex[0] = 0;
    f.vertexIndex[1] = 2;
    f.vertexIndex[2] = 3;
    mesh->faces.push_back(f);
    
    /////LEFT
    //normal
    mesh->normals.push_back(btVector3(1,0,0));
    gln.x = 1.f;
    gln.y = 0;
    gln.z = 0;
    mesh->glnormals.push_back(gln);
    //faces
    f.vertexIndex[0] = 3;
    f.vertexIndex[1] = 2;
    f.vertexIndex[2] = 4;
    f.normalIndex = 1;
    f.glnormalIndex[0] = f.glnormalIndex[1] = f.glnormalIndex[2] = 1;
    f.uvIndex[0] = f.uvIndex[1] = f.uvIndex[2] = 0;
    f.neighbourIndex[0] = f.neighbourIndex[1] = f.neighbourIndex[2] = -1;
    mesh->faces.push_back(f);
    f.vertexIndex[0] = 3;
    f.vertexIndex[1] = 4;
    f.vertexIndex[2] = 5;
    mesh->faces.push_back(f);
    
    /////RIGHT
    //normal
    mesh->normals.push_back(btVector3(-1,0,0));
    gln.x = -1.f;
    gln.y = 0;
    gln.z = 0;
    mesh->glnormals.push_back(gln);
    //faces
    f.vertexIndex[0] = 6;
    f.vertexIndex[1] = 7;
    f.vertexIndex[2] = 1;
    f.normalIndex = 2;
    f.glnormalIndex[0] = f.glnormalIndex[1] = f.glnormalIndex[2] = 2;
    f.uvIndex[0] = f.uvIndex[1] = f.uvIndex[2] = 0;
    f.neighbourIndex[0] = f.neighbourIndex[1] = f.neighbourIndex[2] = -1;
    mesh->faces.push_back(f);
    f.vertexIndex[0] = 6;
    f.vertexIndex[1] = 1;
    f.vertexIndex[2] = 0;
    mesh->faces.push_back(f);
    
    /////BACK
    //normal
    mesh->normals.push_back(btVector3(0,0,1));
    gln.x = 0;
    gln.y = 0;
    gln.z = 1.f;
    mesh->glnormals.push_back(gln);
    //faces
    f.vertexIndex[0] = 5;
    f.vertexIndex[1] = 4;
    f.vertexIndex[2] = 7;
    f.normalIndex = 3;
    f.glnormalIndex[0] = f.glnormalIndex[1] = f.glnormalIndex[2] = 3;
    f.uvIndex[0] = f.uvIndex[1] = f.uvIndex[2] = 0;
    f.neighbourIndex[0] = f.neighbourIndex[1] = f.neighbourIndex[2] = -1;
    mesh->faces.push_back(f);
    f.vertexIndex[0] = 5;
    f.vertexIndex[1] = 7;
    f.vertexIndex[2] = 6;
    mesh->faces.push_back(f);
    
    //////TOP
    //normal
    mesh->normals.push_back(btVector3(0,1,0));
    gln.x = 0;
    gln.y = 1.f;
    gln.z = 0;
    mesh->glnormals.push_back(gln);
    //faces
    f.vertexIndex[0] = 4;
    f.vertexIndex[1] = 2;
    f.vertexIndex[2] = 1;
    f.normalIndex = 4;
    f.glnormalIndex[0] = f.glnormalIndex[1] = f.glnormalIndex[2] = 4;
    f.uvIndex[0] = f.uvIndex[1] = f.uvIndex[2] = 0;
    f.neighbourIndex[0] = f.neighbourIndex[1] = f.neighbourIndex[2] = -1;
    mesh->faces.push_back(f);
    f.vertexIndex[0] = 4;
    f.vertexIndex[1] = 1;
    f.vertexIndex[2] = 7;
    mesh->faces.push_back(f);
    
    /////BOTTOM
    //normal
    mesh->normals.push_back(btVector3(0,-1,0));
    gln.x = 0;
    gln.y = -1.f;
    gln.z = 0;
    mesh->glnormals.push_back(gln);
    //faces
    f.vertexIndex[0] = 3;
    f.vertexIndex[1] = 5;
    f.vertexIndex[2] = 6;
    f.normalIndex = 5;
    f.glnormalIndex[0] = f.glnormalIndex[1] = f.glnormalIndex[2] = 5;
    f.uvIndex[0] = f.uvIndex[1] = f.uvIndex[2] = 0;
    f.neighbourIndex[0] = f.neighbourIndex[1] = f.neighbourIndex[2] = -1;
    mesh->faces.push_back(f);
    f.vertexIndex[0] = 3;
    f.vertexIndex[1] = 6;
    f.vertexIndex[2] = 0;
    mesh->faces.push_back(f);
    
    for(int i=0; i<11; i++)
        SubdivideFace(mesh, i, 2);
    SubdivideFace(mesh, 0, 2);
    
    CalculateNeighbours(mesh, false);
    
    SetLook(l);
}

BoxEntity::~BoxEntity()
{
}

SolidEntityType BoxEntity::getSolidType()
{
    return BOX;
}

void BoxEntity::BuildCollisionList()
{
}

btCollisionShape* BoxEntity::BuildCollisionShape()
{
    return new btBoxShape(halfExtents);
}

void BoxEntity::CalculateFluidDynamics(const btVector3& surfaceN, const btVector3&surfaceD, const btVector3&fluidV, const Fluid* fluid,
                                            btScalar& submergedVolume, btVector3& cob,  btVector3& drag, btVector3& angularDrag,
                                            btTransform* worldTransform, const btVector3& velocity,
                                            const btVector3& angularVelocity)
{
}

