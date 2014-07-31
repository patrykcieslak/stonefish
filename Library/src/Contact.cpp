//
//  Contact.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "Contact.h"
#include "OpenGLPipeline.h"
#include "SolidEntity.h"
#include "ScientificFileUtil.h"
#include "SimulationApp.h"

#pragma mark Constructors
Contact::Contact(Entity* entityA, Entity* entityB, unsigned int inclusiveHistoryLength)
{
    A = entityA;
    B = entityB;
    historyLen = inclusiveHistoryLength;
    displayMask = CONTACT_DISPLAY_NONE;
}

#pragma mark - Destructor
Contact::~Contact()
{
    A = NULL;
    B = NULL;
    points.clear();
}

#pragma mark - Accessors
const Entity* Contact::getEntityA()
{
    return A;
}

const Entity* Contact::getEntityB()
{
    return B;
}

void Contact::setDisplayMask(int16_t mask)
{
    displayMask = mask;
}

#pragma mark - Methods
void Contact::AddContactPoint(const btPersistentManifold* manifold, bool swapped)
{
    ContactPoint p;
    const btManifoldPoint* mp = NULL;
    
    if(points.size() == 0)
    {
        mp = &manifold->getContactPoint(0);
    }
    else
    {
        ContactPoint lastContact = points.back();
        btScalar closestDistance = BT_LARGE_FLOAT;
    
        for(int i = 0; i < manifold->getNumContacts(); i++)
        {

            btVector3 candidate = swapped ? manifold->getContactPoint(i).getPositionWorldOnB() : manifold->getContactPoint(i).getPositionWorldOnA();
            btScalar distance = ((swapped ? lastContact.locationB : lastContact.locationA) - candidate).length2();
            if(distance < closestDistance)
            {
                mp = &manifold->getContactPoint(i);
                closestDistance = distance;
            }
        }
    }
    
    if(mp->m_userPersistentData != NULL)
    {
        p.locationA = swapped ? mp->getPositionWorldOnB() : mp->getPositionWorldOnA();
        p.locationB = swapped ? mp->getPositionWorldOnA() : mp->getPositionWorldOnB();
        p.slippingVelocityA = (swapped ? btScalar(-1.) : btScalar(1.)) * btVector3(*((btVector3*)mp->m_userPersistentData));
        p.normalForceA = (swapped ? btScalar(1.) : btScalar(-1.)) * mp->m_normalWorldOnB * mp->m_appliedImpulse * SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond();
        AddContactPoint(p);
    }
}

void Contact::AddContactPoint(ContactPoint p)
{
    //historyLen = 0 means "full history"
    if(historyLen > 0 && points.size() == historyLen)
        points.pop_front();
    
    p.timeStamp = SimulationApp::getApp()->getSimulationManager()->getSimulationTime();
    points.push_back(p);
}

void Contact::ClearHistory()
{
    points.clear();
}

void Contact::SaveContactDataToOctaveFile(const char* path, bool includeTime)
{
    if(points.size() == 0)
        return;
    
    //build data structure
    ScientificData data("");
    
    ScientificDataItem* it = new ScientificDataItem();
    it->name = A->getName() + "_" + B->getName();
    it->type = DATA_MATRIX;
    
    btMatrixXu* matrix = new btMatrixXu((unsigned int)points.size(), includeTime ? 10 : 9);
    it->value = matrix;
    
    int offset = includeTime ? 1 : 0;
    
    for(unsigned int i = 0; i < points.size(); ++i)
    {
        if(includeTime)
            matrix->setElem(i, 0, points[i].timeStamp);
        
        matrix->setElem(i, offset, UnitSystem::GetLength(points[i].locationA.x()));
        matrix->setElem(i, offset + 1, UnitSystem::GetLength(points[i].locationA.y()));
        matrix->setElem(i, offset + 2, UnitSystem::GetLength(points[i].locationA.z()));
        matrix->setElem(i, offset + 3, UnitSystem::GetVelocity(points[i].slippingVelocityA.x()));
        matrix->setElem(i, offset + 4, UnitSystem::GetVelocity(points[i].slippingVelocityA.y()));
        matrix->setElem(i, offset + 5, UnitSystem::GetVelocity(points[i].slippingVelocityA.z()));
        matrix->setElem(i, offset + 6, UnitSystem::GetForce(points[i].normalForceA.x()));
        matrix->setElem(i, offset + 7, UnitSystem::GetForce(points[i].normalForceA.y()));
        matrix->setElem(i, offset + 8, UnitSystem::GetForce(points[i].normalForceA.z()));
    }
    
    data.addItem(it);
    
    //save data structure to file
    SaveOctaveData(path, data);
}

#pragma mark - Graphics
void Contact::Render()
{
    if(points.size() == 0)
        return;
    
    if(displayMask & CONTACT_DISPLAY_LAST_A)
    {
        glContactColor();
        glBegin(GL_POINTS);
        glBulletVertex(points.back().locationA);
        glEnd();
    }
    
    if(displayMask & CONTACT_DISPLAY_LAST_B)
    {
        glContactColor();
        glBegin(GL_POINTS);
        glBulletVertex(points.back().locationB);
        glEnd();
    }
    
    if(displayMask & CONTACT_DISPLAY_PATH_A)
    {
        glContactColor();
        glBegin(GL_LINE_STRIP);
        for(size_type i = 0; i < points.size(); i++)
            glBulletVertex(points[i].locationA);
        glEnd();
    }
    
    if(displayMask & CONTACT_DISPLAY_PATH_B)
    {
        glContactColor();
        glBegin(GL_LINE_STRIP);
        for(size_type i = 0; i < points.size(); i++)
            glBulletVertex(points[i].locationB);
        glEnd();
    }
    
    if(displayMask & CONTACT_DISPLAY_LAST_SLIP_VELOCITY_A)
    {
        glContactColor();
        glBegin(GL_LINES);
        glBulletVertex(points.back().locationA);
        glBulletVertex(points.back().locationA + points.back().slippingVelocityA);
        glEnd();
    }
    
    if(displayMask & CONTACT_DISPLAY_LAST_SLIP_VELOCITY_B)
    {
        glContactColor();
        glBegin(GL_LINES);
        glBulletVertex(points.back().locationB);
        glBulletVertex(points.back().locationB - points.back().slippingVelocityA);
        glEnd();
    }
    
    if(displayMask & CONTACT_DISPLAY_NORMAL_FORCE_A)
    {
        glContactColor();
        glBegin(GL_LINES);
        glBulletVertex(points.back().locationA);
        glBulletVertex(points.back().locationA + points.back().normalForceA);
        glEnd();
    }
    
    if(displayMask & CONTACT_DISPLAY_NORMAL_FORCE_B)
    {
        glContactColor();
        glBegin(GL_LINES);
        glBulletVertex(points.back().locationB);
        glBulletVertex(points.back().locationB - points.back().normalForceA);
        glEnd();
    }
}