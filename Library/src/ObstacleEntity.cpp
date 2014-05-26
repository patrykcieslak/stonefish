//
//  ObstacleEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "ObstacleEntity.h"
#include "GeometryFileUtil.h"

ObstacleEntity::ObstacleEntity(std::string uniqueName, const char* modelFilename, btScalar scale, Material* mat, Look l, const btTransform& worldTransform, bool smoothNormals) : StaticEntity(uniqueName, mat, l)
{
    //Load mesh
    scale = UnitSystem::SetLength(scale);
    mesh = LoadModel(modelFilename, scale, smoothNormals);
    
    //Build collision shape
    int* indices = new int[mesh->faces.size()*3];
    for(int i=0; i<mesh->faces.size(); i++)
    {
        indices[i*3+0] = mesh->faces[i].vertexIndex[0];
        indices[i*3+1] = mesh->faces[i].vertexIndex[1];
        indices[i*3+2] = mesh->faces[i].vertexIndex[2];
    }
    triangleArray = new btTriangleIndexVertexArray(mesh->faces.size(), indices, 3*sizeof(int),
                                                   mesh->vertices.size(), (btScalar*)&mesh->vertices[0].x(), sizeof(btVector3));
    btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(triangleArray, true);
    
    //Build rigid body
    btDefaultMotionState* motionState = new btDefaultMotionState(UnitSystem::SetTransform(worldTransform));
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0, motionState, shape, btVector3(0,0,0));
    rigidBodyCI.m_friction = rigidBodyCI.m_rollingFriction = rigidBodyCI.m_restitution = btScalar(1.); //not used
    rigidBody = new btRigidBody(rigidBodyCI);
    rigidBody->setUserPointer(this);
    rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
    
    //Build display list
    displayList = glGenLists(1);
    glNewList(displayList, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for(int h=0; h<mesh->faces.size(); h++)
    {
        btVector3 v1 = mesh->vertices[mesh->faces[h].vertexIndex[0]];
        btVector3 v2 = mesh->vertices[mesh->faces[h].vertexIndex[1]];
        btVector3 v3 = mesh->vertices[mesh->faces[h].vertexIndex[2]];
        OpenGLNormal n1 = mesh->glnormals[mesh->faces[h].glnormalIndex[0]];
        OpenGLNormal n2 = mesh->glnormals[mesh->faces[h].glnormalIndex[1]];
        OpenGLNormal n3 = mesh->glnormals[mesh->faces[h].glnormalIndex[2]];
            
        if(mesh->uvs.size() > 0)
        {
            glNormal3f(n1.x, n1.y, n1.z);
            glTexCoord2f(mesh->uvs[mesh->faces[h].uvIndex[0]].u, mesh->uvs[mesh->faces[h].uvIndex[0]].v);
            glVertex3f((GLfloat)v1.x(), (GLfloat)v1.y(), (GLfloat)v1.z());
                
            glNormal3f(n2.x, n2.y, n2.z);
            glTexCoord2f(mesh->uvs[mesh->faces[h].uvIndex[1]].u, mesh->uvs[mesh->faces[h].uvIndex[1]].v);
            glVertex3f((GLfloat)v2.x(), (GLfloat)v2.y(), (GLfloat)v2.z());
                
            glNormal3f(n3.x, n3.y, n3.z);
            glTexCoord2f(mesh->uvs[mesh->faces[h].uvIndex[2]].u, mesh->uvs[mesh->faces[h].uvIndex[2]*2].v);
            glVertex3f((GLfloat)v3.x(), (GLfloat)v3.y(), (GLfloat)v3.z());
        }
        else
        {
            glNormal3f(n1.x, n1.y, n1.z);
            glVertex3f((GLfloat)v1.x(), (GLfloat)v1.y(), (GLfloat)v1.z());
                
            glNormal3f(n2.x, n2.y, n2.z);
            glVertex3f((GLfloat)v2.x(), (GLfloat)v2.y(), (GLfloat)v2.z());
                
            glNormal3f(n3.x, n3.y, n3.z);
            glVertex3f((GLfloat)v3.x(), (GLfloat)v3.y(), (GLfloat)v3.z());
        }
    }
    glEnd();
    glEndList();
}

ObstacleEntity::~ObstacleEntity()
{
    delete mesh;
}

StaticEntityType ObstacleEntity::getStaticType()
{
    return STATIC_OBSTACLE;
}