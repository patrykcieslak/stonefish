//
//  CompoundEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/31/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "CompoundEntity.h"
#include "OpenGLSolids.h"

CompoundEntity::CompoundEntity(std::string uniqueName):SolidEntity(uniqueName, NULL)
{
}

CompoundEntity::~CompoundEntity()
{
}

SolidEntityType CompoundEntity::getSolidType()
{
    return COMPOUND;
}

btCollisionShape* CompoundEntity::BuildCollisionShape()
{
    btCompoundShape* compoundShape = new btCompoundShape();
    for(int i=0; i<solids.size(); i++)
    {
        btTransform childTrans = localTransform.inverse()*solidLocations[i]*solids[i]->getLocalTransform();
        compoundShape->addChildShape(childTrans, solids[i]->BuildCollisionShape());
    }
    
    return compoundShape;
}

void CompoundEntity::BuildRigidBody()
{
    if(rigidBody == NULL)
    {
        //1. Calculate compound mass and compound COG (sum of location - local * m / M)
        //2. Calculate inertia of part in global frame and compound COG (rotate and translate inertia tensor) 3x3
        //   and calculate compound inertia 3x3 (sum of parts inertia)
        //3. Find primary axes of inertia 3x1
        //4. Rotate frame to match primary axes and move to COG
        //5. Create rigid body with inverse transformation
        
        //1. Calculate compound mass and COG
        btVector3 compoundCOG = btVector3(0,0,0);
        btScalar compoundMass = 0;
        localTransform = btTransform::getIdentity();
        
        for(int i=0; i<solids.size(); i++)
        {
            compoundMass += solids[i]->getMass();
            compoundCOG += (solidLocations[i].getOrigin()+solids[i]->getLocalTransform().getOrigin())*solids[i]->getMass();
        }
        compoundCOG /= compoundMass;
        localTransform.setOrigin(compoundCOG);
        
        //2. Calculate compound inertia matrix
        btMatrix3x3 compoundInertia = btMatrix3x3(0,0,0,0,0,0,0,0,0);
        
        for (int i=0; i<solids.size(); i++)
        {
            //Calculate inertia matrix 3x3 of solid in the global frame and COG
            btVector3 solidPriInertia = solids[i]->getMomentsOfInertia();
            btMatrix3x3 solidInertia = btMatrix3x3(solidPriInertia.x(), 0, 0, 0, solidPriInertia.y(), 0, 0, 0, solidPriInertia.z());
            
            //Rotate inertia tensor from local to global
            btMatrix3x3 rotation = solidLocations[i].getBasis()*solids[i]->getLocalTransform().getBasis();
            solidInertia = rotation * solidInertia * rotation.transpose();
            
            //Translate inertia tensor from local COG to global COG
            btVector3 translation = solidLocations[i].getOrigin()+solids[i]->getLocalTransform().getOrigin()-compoundCOG;
            solidInertia = solidInertia +  btMatrix3x3(translation.y()*translation.y()+translation.z()*translation.z(), -translation.x()*translation.y(), -translation.x()*translation.z(),
                                                       -translation.y()*translation.x(), translation.x()*translation.x()+translation.z()*translation.z(), -translation.y()*translation.z(),
                                                       -translation.z()*translation.x(), -translation.z()*translation.y(), translation.x()*translation.x()+translation.y()*translation.y()).scaled(btVector3(solids[i]->getMass(), solids[i]->getMass(), solids[i]->getMass()));
            
            //Accumulate inertia tensor
            compoundInertia += solidInertia;
        }
        
        //3. Calculate primary moments of inertia and rotation matrix
        btMatrix3x3 rotation;
        compoundInertia.diagonalize(rotation, 0.0001, 100);
        btVector3 compoundPriInertia = btVector3(compoundInertia.getRow(0).getX(), compoundInertia.getRow(1).getY(), compoundInertia.getRow(2).getZ());
        localTransform.setBasis(rotation);
        
        //4. Build collision shape
        btCompoundShape* compoundShape = (btCompoundShape*)BuildCollisionShape();
        
        //5. Build rigid body
        btDefaultMotionState* motionState = new btDefaultMotionState(localTransform);
        
        btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(compoundMass, motionState, compoundShape, compoundPriInertia);
        rigidBodyCI.m_friction = 0.5;
        rigidBodyCI.m_rollingFriction = 0.1;
        rigidBodyCI.m_restitution = 0.5;
        rigidBodyCI.m_linearDamping = 0.0;
        rigidBodyCI.m_angularDamping = 0.0;
        rigidBodyCI.m_linearSleepingThreshold = 0.001;
        rigidBodyCI.m_angularSleepingThreshold = 0.01;
        
        rigidBody = new btRigidBody(rigidBodyCI);
        rigidBody->setActivationState(DISABLE_DEACTIVATION);
        rigidBody->setUserPointer(this);
    }
}

void CompoundEntity::BuildCollisionList()
{
    if(rigidBody != NULL)
    {
        if(collisionList != 0)
            glDeleteLists(collisionList, 1);
        
        btCompoundShape* shape = (btCompoundShape*)rigidBody->getCollisionShape();
        collisionList = glGenLists(1);
        glNewList(collisionList, GL_COMPILE);
        for(int i=0; i<solids.size(); i++)
        {
            btCollisionShape* ch = shape->getChildShape(i);
            btTransform chT = shape->getChildTransform(i);
            btScalar openglTrans[16];
            chT.getOpenGLMatrix(openglTrans);

            glPushMatrix();
#ifdef BT_USE_DOUBLE_PRECISION
            glMultMatrixd(openglTrans);
#else
            glMultMatrixf(openglTrans);
#endif
            switch(ch->getShapeType())
            {
                case CONVEX_HULL_SHAPE_PROXYTYPE:
                {
                    btVector3 v1;
                    btConvexHullShape* hull = (btConvexHullShape*)ch;
                    
                    glBegin(GL_POINTS);
                    for(int h=0; h<hull->getNumVertices(); h++)
                    {
                        hull->getVertex(h, v1);
                        glVertex3f((GLfloat)v1.x(), (GLfloat)v1.y(), (GLfloat)v1.z());
                    }
                    glEnd();
                }
                break;
                    
                case SPHERE_SHAPE_PROXYTYPE:
                {
                    btSphereShape* sph = (btSphereShape*)ch;
                    OpenGLSolids::DrawPointSphere(sph->getRadius());
                }
                break;
            }
            
            glPopMatrix();
        }
        glEndList();
    }
}

void CompoundEntity::BuildDisplayList()
{
    if(rigidBody != NULL)
    {
        if(displayList != 0)
            glDeleteLists(displayList, 1);
        
        displayList = glGenLists(1);
        glNewList(displayList, GL_COMPILE);
        for(int i=0; i<solids.size(); i++)
        {
            glPushMatrix();
            
            btTransform trans = localTransform.inverse()*solidLocations[i]*solids[i]->getLocalTransform();
            btScalar openglTrans[16];
            trans.getOpenGLMatrix(openglTrans);
#ifdef BT_USE_DOUBLE_PRECISION
            glMultMatrixd(openglTrans);
#else
            glMultMatrixf(openglTrans);
#endif
            UseLook(solids[i]->getLook());
            glCallList(solids[i]->getDisplayList());
            
            glPopMatrix();
        }
        glEndList();
    }
}

void CompoundEntity::AddSolid(SolidEntity* solid, const btTransform& location)
{
    solids.push_back(solid);
    solidLocations.push_back(UnitSystem::SetTransform(location));
}

SolidEntity* CompoundEntity::GetSolid(unsigned int index)
{
    if(index < solids.size())
        return solids[index];
    else
        return NULL;
}

unsigned long CompoundEntity::SolidsCount()
{
    return (unsigned long)solids.size();
}

void CompoundEntity::CalculateFluidDynamics(const btVector3 &surfaceN, const btVector3 &surfaceD, const btVector3 &fluidV, const Fluid *fluid,
                                          btScalar& submergedVolume, btVector3 &COB, btVector3 &drag, btVector3 &angularDrag,
                                          btTransform* worldTransform, const btVector3& velocity, const btVector3& angularVelocity)
{
    COB = btVector3(0,0,0);
    submergedVolume = 0;
    drag = btVector3(0,0,0);
    angularDrag = btVector3(0,0,0);
    
    for(int i=0; i<solids.size(); i++)
    {
        btVector3 scob, sdrag, sangularDrag, slift;
        btScalar ssubmergedVolume = 0.f;
        btTransform localSolidTransform = localTransform.inverse()*solidLocations[i]*solids[i]->getLocalTransform();
        btTransform worldSolidTransform = rigidBody->getWorldTransform()*localSolidTransform;
        
        solids[i]->CalculateFluidDynamics(surfaceN, surfaceD, fluidV, fluid, ssubmergedVolume, scob, sdrag, sangularDrag, &worldSolidTransform, rigidBody->getLinearVelocity(), rigidBody->getAngularVelocity());
        COB += localSolidTransform*scob*ssubmergedVolume;
        submergedVolume += ssubmergedVolume;
        drag += sdrag;
        angularDrag += sangularDrag;
    }
    
    if(submergedVolume > 0)
    {
        COB /= submergedVolume;
    }
    
    //test
    //COB = rigidBody->getWorldTransform().getBasis()*COB;
    centerOfBuoyancy = COB;
}