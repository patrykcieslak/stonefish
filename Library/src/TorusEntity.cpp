//
//  TorusEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "TorusEntity.h"
#include "OpenGLSolids.h"
#include "btTorusShape.h"

TorusEntity::TorusEntity(std::string uniqueName, btScalar torusMajorRadius, btScalar torusMinorRadius, Material* mat, Look l) : SolidEntity(uniqueName, mat)
{
    majorRadius = UnitSystem::SetLength(torusMajorRadius);
    minorRadius = UnitSystem::SetLength(torusMinorRadius);
    material = mat;
    
    //Calculate physical properties
    dragCoeff = btVector3(0.5, 0.5, 0.5);//btVector3(radius*halfHeight*4.0*0.5, M_PI*radius*radius*0.9, radius*halfHeight*4.0*0.5);
    volume = M_PI*minorRadius*minorRadius*0.5*2.0*M_PI*majorRadius;
    mass = volume * material->density;
    btScalar idiam, ivert;
	idiam = btScalar(1.)/btScalar(8.)*(btScalar(4.)*majorRadius*majorRadius + btScalar(5.)*minorRadius*minorRadius)*mass;
	ivert = (majorRadius*majorRadius + btScalar(3.)/btScalar(4.)*minorRadius*minorRadius)*mass;
	Ipri = btVector3(ivert, idiam, ivert);
    
    //Build mesh
    /*mesh = new TriangleMesh();
    
    //vertices
    for(int i=0; i<MESH_RESOLUTION; i++)
    {
        btVector3 v;
        v = btVector3(sin(i/(double)MESH_RESOLUTION*M_PI*2.0)*radius, halfHeight, cos(i/(double)MESH_RESOLUTION*M_PI*2.0)*radius);
        mesh->vertices.push_back(v);
        v.setY(-halfHeight);
        mesh->vertices.push_back(v);
    }
    
    //faces
    //SIDE
    
    for(int i=0; i<MESH_RESOLUTION-1; i++)
    {
        btVector3 n = btVector3(sin(i/(double)MESH_RESOLUTION*M_PI*2.0), 0.0, cos(i/(double)MESH_RESOLUTION*M_PI*2.0));
        mesh->normals.push_back(n);
        OpenGLNormal gln;
        gln.x = n.x();
        gln.y = n.y();
        gln.z = n.z();
        mesh->glnormals.push_back(gln);
        
        TriangleFace f;
        f.vertexIndex[0] = i*2;
        f.vertexIndex[1] = i*2+1;
        f.vertexIndex[2] = i*2+2;
        f.normalIndex = i;
        f.glnormalIndex[0] = f.glnormalIndex[1] = f.glnormalIndex[2] = i;
        f.neighbourIndex[0] = f.neighbourIndex[1] = f.neighbourIndex[2] = -1;
        f.uvIndex[0] = f.uvIndex[1] = f.uvIndex[2] = 0;
        mesh->faces.push_back(f);
        f.vertexIndex[0] = i*2+1;
        f.vertexIndex[1] = i*2+3;
        f.vertexIndex[2] = i*2+2;
        mesh->faces.push_back(f);
    }
    
    btVector3 n = btVector3(sin((double)(MESH_RESOLUTION-1)/(double)MESH_RESOLUTION*M_PI*2.0), 0.0, cos((double)(MESH_RESOLUTION-1)/(double)MESH_RESOLUTION*M_PI*2.0));
    mesh->normals.push_back(n);
    OpenGLNormal gln;
    gln.x = n.x();
    gln.y = n.y();
    gln.z = n.z();
    mesh->glnormals.push_back(gln);
    
    TriangleFace f;
    int i = MESH_RESOLUTION-1;
    f.vertexIndex[0] = i*2;
    f.vertexIndex[1] = i*2+1;
    f.vertexIndex[2] = 0;
    f.normalIndex = i;
    f.glnormalIndex[0] = f.glnormalIndex[1] = f.glnormalIndex[2] = i;
    f.neighbourIndex[0] = f.neighbourIndex[1] = f.neighbourIndex[2] = -1;
    f.uvIndex[0] = f.uvIndex[1] = f.uvIndex[2] = 0;
    mesh->faces.push_back(f);
    f.vertexIndex[0] = i*2+1;
    f.vertexIndex[1] = 1;
    f.vertexIndex[2] = 0;
    mesh->faces.push_back(f);
    
    //CAPS
    n = btVector3(0,1,0);
    mesh->normals.push_back(n);
    gln.x = n.x();
    gln.y = n.y();
    gln.z = n.z();
    mesh->glnormals.push_back(gln);
    
    n = btVector3(0,-1,0);
    mesh->normals.push_back(n);
    gln.x = n.x();
    gln.y = n.y();
    gln.z = n.z();
    mesh->glnormals.push_back(gln);
    int n1index = mesh->normals.size()-2;
    int n2index = n1index+1;
    
    btVector3 v;
    v = btVector3(0, halfHeight, 0);
    mesh->vertices.push_back(v);
    mesh->vertices.push_back(-v);
    int v1index = mesh->vertices.size()-2;
    int v2index = v1index+1;
    
    for(int i=0; i<MESH_RESOLUTION-1; i++)
    {
        TriangleFace f;
        f.vertexIndex[0] = i*2;
        f.vertexIndex[1] = (i+1)*2;
        f.vertexIndex[2] = v1index;
        f.normalIndex = n1index;
        f.glnormalIndex[0] = f.glnormalIndex[1] = f.glnormalIndex[2] = n1index;
        f.neighbourIndex[0] = f.neighbourIndex[1] = f.neighbourIndex[2] = -1;
        f.uvIndex[0] = f.uvIndex[1] = f.uvIndex[2] = 0;
        mesh->faces.push_back(f);
        f.vertexIndex[0] = (i+1)*2+1;
        f.vertexIndex[1] = i*2+1;
        f.vertexIndex[2] = v2index;
        f.normalIndex = n2index;
        f.glnormalIndex[0] = f.glnormalIndex[1] = f.glnormalIndex[2] = n2index;
        mesh->faces.push_back(f);
    }
    
    i = MESH_RESOLUTION-1;
    f.vertexIndex[0] = i*2;
    f.vertexIndex[1] = 0;
    f.vertexIndex[2] = v1index;
    f.normalIndex = n1index;
    f.glnormalIndex[0] = f.glnormalIndex[1] = f.glnormalIndex[2] = n1index;
    f.neighbourIndex[0] = f.neighbourIndex[1] = f.neighbourIndex[2] = -1;
    f.uvIndex[0] = f.uvIndex[1] = f.uvIndex[2] = 0;
    mesh->faces.push_back(f);
    f.vertexIndex[0] = 1;
    f.vertexIndex[1] = i*2+1;
    f.vertexIndex[2] = v2index;
    f.normalIndex = n2index;
    f.glnormalIndex[0] = f.glnormalIndex[1] = f.glnormalIndex[2] = n2index;
    mesh->faces.push_back(f);
    
    for(int i=0; i<MESH_RESOLUTION*2; i++)
        SubdivideFace(mesh, i, 2);
    
    CalculateNeighbours(mesh, true);
    */
    SetLook(l);
}

TorusEntity::~TorusEntity()
{
}

SolidEntityType TorusEntity::getSolidType()
{
    return TORUS;
}

void TorusEntity::BuildDisplayList()
{
    if(displayList != 0)
        glDeleteLists(displayList, 1);
    
    displayList = glGenLists(1);
    glNewList(displayList, GL_COMPILE);
    OpenGLSolids::DrawSolidTorus(majorRadius, minorRadius);
    glEndList();
}

void TorusEntity::BuildCollisionList()
{
}

btCollisionShape* TorusEntity::BuildCollisionShape()
{
    btTorusShape* colShape = new btTorusShape(majorRadius, minorRadius);
    colShape->setMargin(UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), 0.001));
    return colShape;
}

void TorusEntity::CalculateFluidDynamics(const btVector3& surfaceN, const btVector3&surfaceD, const btVector3&fluidV, const Fluid* fluid,
                                            btScalar& submergedVolume, btVector3& cob,  btVector3& drag, btVector3& angularDrag,
                                            btTransform* worldTransform, const btVector3& velocity,
                                            const btVector3& angularVelocity)
{
}