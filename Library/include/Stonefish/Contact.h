//
//  Contact.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Contact__
#define __Stonefish_Contact__

#include "Entity.h"
#include "OpenGLPipeline.h"

typedef enum {  CONTACT_DISPLAY_NONE = 0,
                CONTACT_DISPLAY_LAST_A = 1 << 0,
                CONTACT_DISPLAY_PATH_A = 1 << 1,
                CONTACT_DISPLAY_LAST_SLIP_VELOCITY_A = 1 << 2,
                CONTACT_DISPLAY_SLIP_VELOCITY_SEQUENCE_A = 1 << 3,
                CONTACT_DISPLAY_NORMAL_FORCE_A = 1 << 4,
                CONTACT_DISPLAY_LAST_B = 1 << 5,
                CONTACT_DISPLAY_PATH_B = 1 << 6,
                CONTACT_DISPLAY_LAST_SLIP_VELOCITY_B = 1 << 7,
                CONTACT_DISPLAY_SLIP_VELOCITY_SEQUENCE_B = 1 << 8,
                CONTACT_DISPLAY_NORMAL_FORCE_B = 1 << 9,
             } ContactDisplayType;

struct ContactPoint
{
    btScalar timeStamp;
    btVector3 locationA;
    btVector3 locationB;
    btVector3 slippingVelocityA;
    btVector3 normalForceA;
};

class Contact
{
public:
    Contact(Entity* entityA, Entity* entityB, unsigned int inclusiveHistoryLength = 1);
    ~Contact();
    
    void AddContactPoint(const btPersistentManifold* manifold, bool swapped);
    void AddContactPoint(ContactPoint p);
    void ClearHistory();
    void SaveContactDataToOctaveFile(const char* path, bool includeTime = true);
    std::vector<Renderable> Render();
    
    void setDisplayMask(int16_t mask);
    const Entity* getEntityA();
    const Entity* getEntityB();
    const std::deque<ContactPoint>& getHistory();
    
private:
    Entity* A;
    Entity* B;
    std::deque<ContactPoint> points;
    unsigned int historyLen;
    int16_t displayMask;
};


#endif
