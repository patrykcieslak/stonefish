//
//  Contact.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "Contact.h"
#include "OpenGLPipeline.h"

Contact::Contact(Entity* e0, Entity* e1, unsigned int inclusiveHistoryLength)
{
    ent0 = e0;
    ent1 = e1;
    historyLen = inclusiveHistoryLength;
    renderType = CONTACT_RENDER_NONE;
}

Contact::~Contact()
{
    ent0 = NULL;
    ent1 = NULL;
    points.clear();
}

const Entity* Contact::getEntity0()
{
    return ent0;
}

const Entity* Contact::getEntity1()
{
    return ent1;
}

void Contact::AddContactPoint(const btPersistentManifold* manifold)
{
    if(historyLen == 0)
        return;
    
    btVector3 newContact;
    
    if(points.size() == 0)
    {
        newContact = manifold->getContactPoint(0).getPositionWorldOnA();
    }
    else
    {
        btVector3 lastContact = points.back();
        btScalar closestDistance = BT_LARGE_FLOAT;
        
        for(int i = 0; i < manifold->getNumContacts(); i++)
        {
            btVector3 candidate = manifold->getContactPoint(i).getPositionWorldOnA();
            btScalar distance = (lastContact - candidate).length2();
            if(distance < closestDistance)
            {
                newContact = candidate;
                closestDistance = distance;
            }
        }
    }
    
    AddContactPoint(newContact);
}

void Contact::AddContactPoint(btVector3 p)
{
    if(points.size() == historyLen)
        points.pop_front();
    
    points.push_back(p);
}

void Contact::ClearHistory()
{
    points.clear();
}

void Contact::Render()
{
    if(renderType == CONTACT_RENDER_LAST)
        RenderLastPoint();
    else if(renderType == CONTACT_RENDER_PATH)
        RenderPath();
}

void Contact::RenderPath()
{
    glContactColor();
    glBegin(GL_LINE_STRIP);
    for(unsigned int i=0; i<points.size(); i++)
        glBulletVertex(points[i]);
    glEnd();
}

void Contact::RenderLastPoint()
{
    glContactColor();
    glBegin(GL_POINTS);
    glBulletVertex(points.back());
    glEnd();
}

void Contact::setRenderType(ContactRenderType type)
{
    renderType = type;
}
