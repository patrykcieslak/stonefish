//
//  VelocityField.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/07/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_VelocityField__
#define __Stonefish_VelocityField__

#include <common.h>
#include <graphics/OpenGLContent.h>

class VelocityField
{
public:
    VelocityField();
    virtual ~VelocityField();
    
    virtual btVector3 GetVelocityAtPoint(const btVector3& p) = 0;
    virtual std::vector<Renderable> Render() = 0;
};

#endif