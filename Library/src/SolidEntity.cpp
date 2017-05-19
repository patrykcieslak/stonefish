//
//  SolidEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "SolidEntity.h"
#include "OpenGLSolids.h"

#pragma mark Constructors
SolidEntity::SolidEntity(std::string uniqueName, Material* mat) : Entity(uniqueName)
{
    material = mat;
    localTransform = btTransform::getIdentity();
    Ipri = btVector3(0,0,0);
    mass = 0;
    volume = 0;
    dragCoeff = btVector3(0,0,0);
    centerOfBuoyancy = btVector3(0,0,0);
    addedMass = btVector3(0,0,0);
    addedInertia = btVector3(0,0,0);
    
    linearAcc = btVector3(0,0,0);
    angularAcc = btVector3(0,0,0);
    
    rigidBody = NULL;
    multibodyCollider = NULL;
    displayList = 0;
    collisionList = 0;
    dispCoordSys = false;
    fullyImmersed = false;
}

#pragma mark - Destructor
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
    multibodyCollider = NULL;
}

#pragma mark - Accessors
EntityType SolidEntity::getType()
{
    return ENTITY_SOLID;
}

void SolidEntity::SetHydrodynamicProperties(btVector3 dragCoefficients, btVector3 aMass, btVector3 aInertia)
{
    dragCoeff = dragCoefficients;
    addedMass = aMass;
    addedInertia = aInertia;
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
    else if(rigidBody == NULL && multibodyCollider == NULL)
    {
        this->mass = UnitSystem::SetMass(mass);
        Ipri = UnitSystem::SetInertia(inertia);
        localTransform = UnitSystem::SetTransform(cogTransform);
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

btRigidBody* SolidEntity::getRigidBody()
{
    return rigidBody;
}

btMultiBodyLinkCollider* SolidEntity::getMultibodyLinkCollider()
{
    return multibodyCollider;
}

void SolidEntity::GetAABB(btVector3& min, btVector3& max)
{
    rigidBody->getAabb(min, max);
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
    else if(multibodyCollider != NULL)
    {
        return multibodyCollider->getWorldTransform();
    }
    else
        return btTransform::getIdentity();
}

btVector3 SolidEntity::getLinearVelocity()
{
    if(rigidBody != NULL)
    {
        return rigidBody->getLinearVelocity();
    }
    else if(multibodyCollider != NULL)
    {
        //Get multibody and link id
        btMultiBody* multiBody = multibodyCollider->m_multiBody;
        int index = multibodyCollider->m_link;
        
        //Start with base velocity
        btVector3 linVelocity = multiBody->getBaseVel(); //Global
        
        if(index >= 0) //If collider is not base
        {
            for(unsigned int i = 0; i <= index; i++) //Accumulate velocity resulting from joints
            {
                if(multiBody->getLink(i).m_jointType == btMultibodyLink::ePrismatic) //Just add linear velocity
                {
                    btVector3 axis = multiBody->getLink(i).getAxisBottom(0); //Local axis
                    btVector3 vel = multiBody->getJointVel(i) * axis; //Local velocity
                    btVector3 gvel = multiBody->localDirToWorld(i, vel); //Global velocity
                    linVelocity += gvel;
                }
                else if(multiBody->getLink(i).m_jointType == btMultibodyLink::eRevolute) //Add linear velocity due to rotation
                {
                    btVector3 axis = multiBody->getLink(i).getAxisBottom(0); //Local linear motion
                    btVector3 vel = multiBody->getJointVel(i) * axis; //Local velocity
                    btVector3 gvel = multiBody->localDirToWorld(i, vel); //Global velocity
                    linVelocity += gvel;
                }
            }
        }
        
        return linVelocity;
    }
    else
        return btVector3(0.,0.,0.);
}

btVector3 SolidEntity::getAngularVelocity()
{
    if(rigidBody != NULL)
    {
        return rigidBody->getAngularVelocity();
    }
    else if(multibodyCollider != NULL)
    {
        //Get multibody and link id
        btMultiBody* multiBody = multibodyCollider->m_multiBody;
        int index = multibodyCollider->m_link;
        
        //Start with base velocity
        btVector3 angVelocity = multiBody->getBaseOmega(); //Global
        
        if(index >= 0)
        {
            for(unsigned int i = 0; i <= index; i++) //Accumulate velocity resulting from joints
                if(multiBody->getLink(i).m_jointType == btMultibodyLink::eRevolute) //Only revolute joints can change angular velocity
                {
                    btVector3 axis = multiBody->getLink(i).getAxisTop(0); //Local axis
                    btVector3 vel = multiBody->getJointVel(i) * axis; //Local velocity
                    btVector3 gvel = multiBody->localDirToWorld(i, vel); //Global velocity
                    angVelocity += gvel;
                }
        }
        
        return angVelocity;
    }
    else
        return btVector3(0.,0.,0.);
}


btVector3 SolidEntity::getLinearVelocityInLocalPoint(const btVector3& relPos)
{
    if(rigidBody != NULL)
    {
        return rigidBody->getVelocityInLocalPoint(relPos);
    }
    else if(multibodyCollider != NULL)
    {
        return getLinearVelocity() + getAngularVelocity().cross(relPos);
    }
    else
        return btVector3(0.,0.,0.);
}

btVector3 SolidEntity::getLinearAcceleration()
{
    return linearAcc;
}

btVector3 SolidEntity::getAngularAcceleration()
{
    return angularAcc;
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

#pragma mark - Methods
void SolidEntity::BuildRigidBody()
{
    if(rigidBody == NULL)
    {
        btDefaultMotionState* motionState = new btDefaultMotionState();
        
        btCompoundShape* colShape = new btCompoundShape();
        colShape->addChildShape(localTransform.inverse(), BuildCollisionShape());
        colShape->setMargin(UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), 0.001));
        
        btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionState, colShape, Ipri);
        rigidBodyCI.m_friction = rigidBodyCI.m_rollingFriction = rigidBodyCI.m_restitution = btScalar(0.); //not used
        rigidBodyCI.m_linearDamping = rigidBodyCI.m_angularDamping = btScalar(0.); //not used
        rigidBodyCI.m_linearSleepingThreshold = rigidBodyCI.m_angularSleepingThreshold = btScalar(0.); //not used
        rigidBodyCI.m_additionalDamping = false;
        
        rigidBody = new btRigidBody(rigidBodyCI);
        rigidBody->setUserPointer(this);
        rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
        rigidBody->setActivationState(DISABLE_DEACTIVATION);
        rigidBody->setFlags(rigidBody->getFlags() | BT_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_BODY);
        
        //rigidBody->setCcdMotionThreshold(0.01);
        //rigidBody->setCcdSweptSphereRadius(0.9);
    }
}

void SolidEntity::BuildMultibodyLinkCollider(btMultiBody *mb, unsigned int child, btMultiBodyDynamicsWorld *world)
{
    if(multibodyCollider == NULL)
    {
        //Shape with offset
        btCompoundShape* colShape = new btCompoundShape();
        colShape->addChildShape(localTransform.inverse(), BuildCollisionShape());
        colShape->setMargin(UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), 0.001));
        
        //Link
        multibodyCollider = new btMultiBodyLinkCollider(mb, child - 1);
        multibodyCollider->setUserPointer(this);
        multibodyCollider->setCollisionShape(colShape);
        multibodyCollider->setFriction(btScalar(0.));
        multibodyCollider->setRestitution(btScalar(0.));
        multibodyCollider->setRollingFriction(btScalar(0.));
        multibodyCollider->setCollisionFlags(multibodyCollider->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
        multibodyCollider->setActivationState(DISABLE_DEACTIVATION);
        
        if(child > 0)
            mb->getLink(child - 1).m_collider = multibodyCollider;
        else
            mb->setBaseCollider(multibodyCollider);
        
        world->addCollisionObject(multibodyCollider, MASK_DEFAULT, MASK_STATIC | MASK_DEFAULT);
        
        //Graphics
        BuildDisplayList();
        BuildCollisionList();
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

void SolidEntity::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world)
{
    AddToDynamicsWorld(world, btTransform::getIdentity());
}

void SolidEntity::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform)
{
    if(rigidBody == NULL)
    {
        BuildRigidBody();
        BuildDisplayList();
        BuildCollisionList();
        
        //rigidBody->setMotionState(new btDefaultMotionState(UnitSystem::SetTransform(worldTransform)));
        rigidBody->setCenterOfMassTransform(localTransform * UnitSystem::SetTransform(worldTransform));
        world->addRigidBody(rigidBody, MASK_DEFAULT, MASK_STATIC | MASK_DEFAULT);
    }
}

void SolidEntity::RemoveFromDynamicsWorld(btMultiBodyDynamicsWorld* world)
{
    if(rigidBody != NULL)
    {
        world->removeRigidBody(rigidBody);
        delete rigidBody;
        rigidBody = NULL;
    }
}

void SolidEntity::UpdateAcceleration()
{
    if(rigidBody != NULL)
    {
        linearAcc = rigidBody->getTotalForce()/mass;
        btVector3 torque = rigidBody->getTotalTorque();
        angularAcc = btVector3(torque.x()/Ipri.x(), torque.y()/Ipri.y(), torque.z()/Ipri.z());
    }
}

void SolidEntity::ApplyGravity()
{
    if(rigidBody != NULL)
    {
        rigidBody->applyGravity();
    }
}

void SolidEntity::ApplyCentralForce(const btVector3& force)
{
    if(rigidBody != NULL)
        rigidBody->applyCentralForce(force);
    else if(multibodyCollider != NULL)
    {
        btMultiBody* multiBody = multibodyCollider->m_multiBody;
        int index = multibodyCollider->m_link;
        
        if(index == -1) //base
            multiBody->addBaseForce(force);
        else
            multiBody->addLinkForce(index, force);
    }
}

void SolidEntity::ApplyTorque(const btVector3& torque)
{
    if(rigidBody != NULL)
        rigidBody->applyTorque(torque);
    else if(multibodyCollider != NULL)
    {
        btMultiBody* multiBody = multibodyCollider->m_multiBody;
        int index = multibodyCollider->m_link;
        
        if(index == -1) //base
            multiBody->addBaseTorque(torque);
        else
            multiBody->addLinkTorque(index, torque);
    }
}

void SolidEntity::ApplyFluidForces(FluidEntity* fluid)
{
}