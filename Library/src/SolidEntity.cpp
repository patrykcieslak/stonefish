//
//  SolidEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "SolidEntity.h"
#include "OpenGLSolids.h"

SolidEntity::SolidEntity(std::string uniqueName, Material* mat) : Entity(uniqueName)
{
    material = mat;
    look = CreateMatteLook(1.f, 1.f, 1.f, 0.5f);
    
    localTransform = btTransform::getIdentity();
    Ipri = btVector3(0,0,0);
    mass = 0;
    
    volume = 0;
    dragCoeff = btVector3(0,0,0);
    centerOfBuoyancy = btVector3(0,0,0);
    addInertia = btVector3(0,0,0);
    addMass = btVector3(0,0,0);
    
    rigidBody = NULL;
    displayList = 0;
    collisionList = 0;
    dispCoordSys = false;
    
    fullyImmersed = false;
}

SolidEntity::~SolidEntity()
{
    if(displayList != 0)
        glDeleteLists(displayList, 1);
    if(collisionList != 0)
        glDeleteLists(collisionList, 1);
    if(look.texture != 0)
        glDeleteTextures(1, &look.texture);
    
    material = NULL;
    rigidBody = NULL;
}

EntityType SolidEntity::getType()
{
    return ENTITY_SOLID;
}

void SolidEntity::SetHydrodynamicProperties(btVector3 dragCoefficients, btVector3 addedMass, btVector3 addedInertia)
{
    dragCoeff = dragCoefficients;
    addMass = addedMass;
    addInertia = addedInertia;
}

void SolidEntity::SetArbitraryPhysicalProperties(btScalar mass, const btVector3& inertia, const btTransform& cogTransform)
{
    if(rigidBody != NULL)
    {
        this->mass = UnitSystem::SetMass(mass);
        Ipri = UnitSystem::SetInertia(inertia);
        rigidBody->setMassProps(this->mass, Ipri);
        
        btTransform oldLocalTransform = localTransform;
        localTransform = UnitSystem::SetTransform(cogTransform);
        btCompoundShape* colShape = (btCompoundShape*)rigidBody->getCollisionShape();
        rigidBody->setCenterOfMassTransform(oldLocalTransform.inverse() * localTransform * rigidBody->getCenterOfMassTransform());
        colShape->updateChildTransform(0, localTransform.inverse());
    }
}

void SolidEntity::SetLook(Look newLook)
{
    if(look.texture != 0)
        glDeleteTextures(1, &look.texture);
    
    look = newLook;
}

void SolidEntity::setDisplayCoordSys(bool enabled)
{
    dispCoordSys = enabled;
}

bool SolidEntity::isCoordSysVisible()
{
    return dispCoordSys;
}

Look SolidEntity::getLook()
{
    return look;
}

GLint SolidEntity::getDisplayList()
{
    return displayList;
}

void SolidEntity::Render()
{
    if(rigidBody != NULL && isRenderable())
    {
        btTransform trans =  getTransform() * localTransform.inverse();
        btScalar openglTrans[16];
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

btTransform SolidEntity::getTransform()
{
    if(rigidBody != NULL)
    {
        btTransform trans;
        rigidBody->getMotionState()->getWorldTransform(trans);
        return trans;
    }
    
    return btTransform::getIdentity();
}

void SolidEntity::setTransform(const btTransform &trans)
{
    if(rigidBody != NULL)
    {
        btDefaultMotionState* motionState = new btDefaultMotionState(trans);
        rigidBody->setMotionState(motionState);
    }
}

btScalar SolidEntity::getVolume()
{
    return volume;
}

btVector3 SolidEntity::getDragCoefficients()
{
    return dragCoeff;
}

btTransform SolidEntity::getLocalTransform()
{
    return localTransform;
}

btVector3 SolidEntity::getMomentsOfInertia()
{
    return Ipri;
}

btScalar SolidEntity::getMass()
{
    return mass;
}

Material* SolidEntity::getMaterial()
{
    return material;
}

void SolidEntity::BuildRigidBody()
{
    if(rigidBody == NULL)
    {
        btDefaultMotionState* motionState = new btDefaultMotionState();
        
        btCompoundShape* colShape = new btCompoundShape();
        colShape->addChildShape(localTransform.inverse(), BuildCollisionShape());
        colShape->setMargin(UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), 0.001));
        
        btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionState, colShape, Ipri);
        rigidBodyCI.m_friction = rigidBodyCI.m_rollingFriction = rigidBodyCI.m_restitution = btScalar(0.0); //not used
        rigidBodyCI.m_linearDamping = 0;
        rigidBodyCI.m_angularDamping = 0;
        rigidBodyCI.m_linearSleepingThreshold = UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), 0.0001);
        rigidBodyCI.m_angularSleepingThreshold = 0.0001;
        
        rigidBody = new btRigidBody(rigidBodyCI);
        rigidBody->setUserPointer(this);
        rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
        rigidBody->setFlags(rigidBody->getFlags() | BT_ENABLE_GYROPSCOPIC_FORCE);
        rigidBody->forceActivationState(DISABLE_DEACTIVATION);
        //rigidBody->setCcdMotionThreshold(0.01);
        //rigidBody->setCcdSweptSphereRadius(0.9);
    }
}

void SolidEntity::BuildDisplayList()
{
    if(displayList != 0)
        glDeleteLists(displayList, 1);
    
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
        
        if(look.texture > 0 && mesh->uvs.size() > 0)
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

void SolidEntity::AddToDynamicsWorld(btDynamicsWorld* world)
{
    AddToDynamicsWorld(world, btTransform::getIdentity());
}

void SolidEntity::AddToDynamicsWorld(btDynamicsWorld* world, const btTransform& worldTransform)
{
    if(rigidBody == NULL)
    {
        BuildRigidBody();
        BuildDisplayList();
        BuildCollisionList();
        
        //rigidBody->setMotionState(new btDefaultMotionState(UnitSystem::SetTransform(worldTransform)));
        rigidBody->setCenterOfMassTransform(localTransform * UnitSystem::SetTransform(worldTransform));
        world->addRigidBody(rigidBody, DEFAULT, DEFAULT | STATIC | CABLE_EVEN | CABLE_ODD);
    }
}

void SolidEntity::RemoveFromDynamicsWorld(btDynamicsWorld* world)
{
    if(rigidBody != NULL)
    {
        world->removeRigidBody(rigidBody);
        delete rigidBody;
        rigidBody = NULL;
    }
}

void SolidEntity::ApplyGravity()
{
    if(rigidBody != NULL)
    {
        ///added mass!
        rigidBody->applyGravity();
    }
}

btRigidBody* SolidEntity::getRigidBody()
{
    return rigidBody;
}

void SolidEntity::GetAABB(btVector3& min, btVector3& max)
{
    rigidBody->getAabb(min, max);
}