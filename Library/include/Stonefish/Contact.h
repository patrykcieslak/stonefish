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

class Contact
{
public:
    Contact(Entity* e0, Entity* e1);
    ~Contact();
    
    const Entity* getEntity0();
    const Entity* getEntity1();
    
private:
    Entity* ent0;
    Entity* ent1;
};


#endif
