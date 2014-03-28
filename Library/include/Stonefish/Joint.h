//
//  Joint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/13/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Joint__
#define __Stonefish_Joint__

#include "common.h"
#include "SolidEntity.h"

typedef enum {FIXED, REVOLUTE, SPHERICAL, PRISMATIC, CYLINDRICAL, GEAR, BELT} JointType;

//abstract class
class Joint
{
public:
    Joint(std::string uniqueName, bool collideLinkedEntities = true);
    virtual ~Joint();
    
	virtual void Render() = 0;
    virtual JointType getType() = 0;
    
    void AddToDynamicsWorld(btDynamicsWorld* world);
    
    void setRenderable(bool render);
    btTypedConstraint* getConstraint();
    bool isRenderable();
    std::string getName();

protected:
    void setConstraint(btTypedConstraint* constr);
    
private:
    std::string name;
    bool renderable;
    bool collisionEnabled;
    btTypedConstraint* constraint;
};



#endif
