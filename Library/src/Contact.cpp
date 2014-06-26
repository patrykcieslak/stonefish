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

Contact::Contact(Entity* entityA, Entity* entityB, size_type inclusiveHistoryLength)
{
    A = entityA;
    B = entityB;
    historyLen = inclusiveHistoryLength;
    displayMask = CONTACT_DISPLAY_NONE;
}

Contact::~Contact()
{
    A = NULL;
    B = NULL;
    points.clear();
}

const Entity* Contact::getEntityA()
{
    return A;
}

const Entity* Contact::getEntityB()
{
    return B;
}

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
        p.impulseA = (swapped ? btScalar(1.) : btScalar(-1.)) * mp->m_normalWorldOnB * mp->m_appliedImpulse;
        AddContactPoint(p);
    }
}

void Contact::AddContactPoint(ContactPoint p)
{
    //historyLen = 0 means "full history"
    if(historyLen > 0 && points.size() == historyLen)
        points.pop_front();
    
    points.push_back(p);
}

void Contact::ClearHistory()
{
    points.clear();
}

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
    
    if(displayMask & CONTACT_DISPLAY_IMPULSE_A)
    {
        glContactColor();
        glBegin(GL_LINES);
        glBulletVertex(points.back().locationA);
        glBulletVertex(points.back().locationA + points.back().impulseA);
        glEnd();
    }
    
    if(displayMask & CONTACT_DISPLAY_IMPULSE_B)
    {
        glContactColor();
        glBegin(GL_LINES);
        glBulletVertex(points.back().locationB);
        glBulletVertex(points.back().locationB - points.back().impulseA);
        glEnd();
    }
}

void Contact::setDisplayMask(int16_t mask)
{
    displayMask = mask;
}