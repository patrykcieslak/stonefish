//
//  Light.h
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Light__
#define __Stonefish_Light__

#include "graphics/OpenGLLight.h"
#include "actuators/LinkActuator.h"

namespace sf
{

class Light : public LinkActuator
{
public:
    Light(std::string uniqueName, const btVector3& initialPos, glm::vec4 color);
    Light(std::string uniqueName, const btVector3& initialPos, const btVector3& initialDir, btScalar coneAngle, glm::vec4 color);
    
    void Update(btScalar dt);
    
private:
    OpenGLLight* glLight;
};

}

#endif
