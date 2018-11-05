//
//  Light.h
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Light__
#define __Stonefish_Light__

#include "Actuator.h"
#include <graphics/OpenGLLight.h>

class Light : public Actuator
{
public:
    Light(std::string uniqueName, const btVector3& position, glm::vec4 color);
    Light(std::string uniqueName, const btVector3& position, const btVector3& direction, btScalar coneAngle, glm::vec4 color);
    virtual ~Light();
    
    void Update(btScalar dt);
    ActuatorType getType();

private:
    OpenGLLight* glLight;
};

#endif