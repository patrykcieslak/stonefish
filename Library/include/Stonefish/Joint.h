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

typedef enum { FIXED, SPHERICAL, TRANSLATIONAL, REVOLUTE, CYLINDRICAL, PLANAR } JointType;

//pure virtual class
class Joint
{
public:
    Joint();
    virtual ~Joint();
    
	void setRenderable(bool render);
    bool isRenderable();
    void AddToDynamicsWorld(btDynamicsWorld* world);
    btTypedConstraint* getConstraint();
    
    virtual JointType getType() = 0;
    virtual void Render() = 0;

protected:
    void setConstraint(btTypedConstraint* constr);
    
private:
    bool renderable;
    btTypedConstraint* constraint;
};



#endif
