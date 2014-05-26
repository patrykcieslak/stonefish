//
//  Contact.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Contact__
#define __Stonefish_Contact__

#include "Entity.h"

typedef enum {CONTACT_RENDER_NONE, CONTACT_RENDER_LAST, CONTACT_RENDER_PATH} ContactRenderType;

class Contact
{
public:
    Contact(Entity* e0, Entity* e1, unsigned int inclusiveHistoryLength = 0);
    ~Contact();
    
    void AddContactPoint(const btPersistentManifold* manifold);
    void AddContactPoint(btVector3 p);
    void ClearHistory();
    void Render();
    
    void setRenderType(ContactRenderType type);
    const Entity* getEntity0();
    const Entity* getEntity1();
    const std::deque<btVector3>& getHistory();
    
private:
    void RenderPath();
    void RenderLastPoint();

    Entity* ent0;
    Entity* ent1;
    std::deque<btVector3> points;
    unsigned int historyLen;
    ContactRenderType renderType;
};


#endif
