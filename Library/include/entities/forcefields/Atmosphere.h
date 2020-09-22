/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  Atmosphere.h
//  Stonefish
//
//  Created by Patryk Cieślak on 02/12/2018.
//  Copyright (c) 2018-2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Atmosphere__
#define __Stonefish_Atmosphere__

#include <ctime>
#include "core/MaterialManager.h"
#include "entities/ForcefieldEntity.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    class VelocityField;
    class OpenGLAtmosphere;
    struct RenderSettings;
    
    //! A class representing the atmosphere.
    class Atmosphere : public ForcefieldEntity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName the name for the atmosphere
         \param f a pointer to the gas filling the atmosphere (normally air)
         */
        Atmosphere(std::string uniqueName, Fluid g);
        
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
        
        //! A method used to add a velocity field to the atmosphere.
        /*!
         \param field a pointer to a velocity field object
         */
        void AddVelocityField(VelocityField* field);
        
        //! A method running the aerodynamics computation.
        /*!
         \param world a pointer to the dynamics world
         \param co a pointer to the collision object
         \param recompute a flag deciding if hydrodynamic forces need to be recomputed
         */
        void ApplyFluidForces(btDynamicsWorld* world, btCollisionObject* co, bool recompute);
        
        //! A method returning the position of the sun in the sky.
        /*!
         \param azimuthDeg a reference to the variable that will store the azimuth of the sun [deg]
         \param elevationDeg a reference to the variable that will store the elevation of the sun [deg]
         */
        void GetSunPosition(Scalar& azimuthDeg, Scalar& elevationDeg);
        
        //! A method returning the air velocity.
        /*!
         \param point the point in the atmosphere where the velocity should be measured [m]
         \return air velocity at specified point [m/s]
         */
        Vector3 GetFluidVelocity(const Vector3& point) const;
        glm::vec3 GetFluidVelocity(const glm::vec3& point) const;
        
        //! A method checking if a point is inside atmosphere.
        /*!
         \param point the position of a point to be checked [m]
         \return is the point inside atmosphere?
         */
        bool IsInsideFluid(const Vector3& point) const;
        
        //! A method returning a pointer to the gas filling the atmosphere.
        Fluid getGas() const;
        
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
        Fluid gas;
        std::vector<VelocityField*> wind;
        OpenGLAtmosphere* glAtmosphere;
    };
}

#endif
