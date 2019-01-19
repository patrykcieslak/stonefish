//
//  Atmosphere.h
//  Stonefish
//
//  Created by Patryk Cieślak on 02/12/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Atmosphere__
#define __Stonefish_Atmosphere__

#include <ctime>
#include "core/MaterialManager.h"
#include "entities/ForcefieldEntity.h"
#include "graphics/OpenGLAtmosphere.h"

namespace sf
{
    //! A class...
    class Atmosphere : public ForcefieldEntity
    {
    public:
        Atmosphere(std::string uniqueName, Fluid* f);
        ~Atmosphere();
        
        void InitGraphics(const RenderSettings& s);
        void SetupSunPosition(Scalar longitudeDeg, Scalar latitudeDeg, std::tm& utc);
        void SetupSunPosition(Scalar azimuthDeg, Scalar elevationDeg);
        void GetSunPosition(Scalar& azimuthDeg, Scalar& elevationDeg);
        
        ForcefieldType getForcefieldType();
        OpenGLAtmosphere* getOpenGLAtmosphere();
        
        static int JulianDay(std::tm& tm);
        
    private:
        Fluid* gas;
        OpenGLAtmosphere* glAtmosphere;
    };
}

#endif
