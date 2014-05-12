//
//  Contact.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "Contact.h"


Contact::Contact(Entity* e0, Entity* e1)
{
    ent0 = e0;
    ent1 = e1;
}

Contact::~Contact()
{
    ent0 = NULL;
    ent1 = NULL;
}

const Entity* Contact::getEntity0()
{
    return ent0;
}

const Entity* Contact::getEntity1()
{
    return ent1;
}