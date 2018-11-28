//
//  Torus.h
//  Stonefish
//
//  Created by Patryk Cieslak on 3/5/14.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Torus__
#define __Stonefish_Torus__

#include "entities/SolidEntity.h"

namespace sf
{

class Torus : public SolidEntity
{
public:
    Torus(std::string uniqueName, Scalar torusMajorRadius, Scalar torusMinorRadius, const Transform& originTrans, Material m, int lookId = -1, Scalar thickness = Scalar(-1), bool isBuoyant = true);
    ~Torus();
    
    SolidType getSolidType();
    btCollisionShape* BuildCollisionShape();
    
private:	
    Scalar minorRadius;
    Scalar majorRadius;
};

}

#endif
