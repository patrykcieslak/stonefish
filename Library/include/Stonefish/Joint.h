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
#include "NameManager.h"
#include "SolidEntity.h"

#define CONSTRAINT_ERP 0.2
#define CONSTRAINT_CFM 0.0
#define CONSTRAINT_STOP_ERP 1.0
#define CONSTRAINT_STOP_CFM 0.0

typedef enum {FIXED, REVOLUTE, SPHERICAL, PRISMATIC, CYLINDRICAL, GEAR, BELT} JointType;

//abstract class
class Joint
{
public:
    Joint(std::string uniqueName, bool collideLinkedEntities = true);
    virtual ~Joint();
    
	virtual btVector3 Render() = 0;
    virtual JointType getType() = 0;
    
    void AddToDynamicsWorld(btDynamicsWorld* world);
    
    void setRenderable(bool render);
    btTypedConstraint* getConstraint();
    std::string getName();
    
    bool isRenderable();

protected:
    void setConstraint(btTypedConstraint* constr);
    
private:
    std::string name;
    bool renderable;
    bool collisionEnabled;
    btTypedConstraint* constraint;
    
    static NameManager nameManager;
};



#endif
