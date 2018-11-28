//
//  VelocityField.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/07/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_VelocityField__
#define __Stonefish_VelocityField__

#include "StonefishCommon.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

class VelocityField
{
public:
    VelocityField();
    virtual ~VelocityField();
    
    virtual Vector3 GetVelocityAtPoint(const Vector3& p) = 0;
    virtual std::vector<Renderable> Render() = 0;
};

}

#endif
