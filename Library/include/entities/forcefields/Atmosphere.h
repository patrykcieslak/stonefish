//
//  Atmosphere.h
//  Stonefish
//
//  Created by Patryk Cieślak on 02/12/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Atmosphere__
#define __Stonefish_Atmosphere__

#include "core/MaterialManager.h"
#include "entities/ForcefieldEntity.h"
#include "graphics/OpenGLAtmosphere.h"

namespace sf
{
    
    class Atmosphere : public ForcefieldEntity
    {
    public:
        Atmosphere(std::string uniqueName, Fluid* f);
        ~Atmosphere();
        
        void InitGraphics(const RenderSettings& s);
        
        ForcefieldType getForcefieldType();
        OpenGLAtmosphere* getOpenGLAtmosphere();
        
    private:
        Fluid* gas;
        OpenGLAtmosphere* glAtmosphere;
    };
}

#endif
