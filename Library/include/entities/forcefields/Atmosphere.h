//
//  Atmosphere.h
//  Stonefish
//
//  Created by Patryk Cieślak on 02/12/2018.
//  Copyright (c) 2018-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Atmosphere__
#define __Stonefish_Atmosphere__

#include <ctime>
#include "core/MaterialManager.h"
#include "entities/ForcefieldEntity.h"
#include "graphics/OpenGLAtmosphere.h"

namespace sf
{
    //! A class representing the atmosphere.
    class Atmosphere : public ForcefieldEntity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName the name for the atmosphere
         \param f a pointer to the fluid filling the atmosphere (normally air)
         */
        Atmosphere(std::string uniqueName, Fluid* f);
        
        //! A destructor.
        ~Atmosphere();
        
        //! A method implementing graphics initialization.
        /*!
         \param s a reference to a structure containing the render settings
         */
        void InitGraphics(const RenderSettings& s);
        
        //! A method used to set the position of the sun in the sky.
        /*!
         \param longitudeDeg the home longitude [deg]
         \param latitudeDeg the home latitude [deg]
         \param utc the UTC time at home
         */
        void SetupSunPosition(Scalar longitudeDeg, Scalar latitudeDeg, std::tm& utc);
        
        //! A method used to set the position of the sun in the sky.
        /*!
         \param azimuthDeg the sun azimuth [deg]
         \param elevationDeg the sun elevation [deg]
         */
        void SetupSunPosition(Scalar azimuthDeg, Scalar elevationDeg);
        
        //! A method returning the position of the sun in the sky.
        /*!
         \param azimuthDeg a reference to the variable that will store the azimuth of the sun [deg]
         \param elevationDeg a reference to the variable that will store the elevation of the sun [deg]
         */
        void GetSunPosition(Scalar& azimuthDeg, Scalar& elevationDeg);
        
        //! A method returning the type of the force field.
        ForcefieldType getForcefieldType();
        
        //! A method returning a pointer to the OpenGL atmosphere object.
        OpenGLAtmosphere* getOpenGLAtmosphere();
        
        //! A static method to compute Julian day form time.
        /*!
         \param tm a reference to a structure containing UTC time
         */
        static int JulianDay(std::tm& tm);
        
    private:
        Fluid* gas;
        OpenGLAtmosphere* glAtmosphere;
    };
}

#endif
