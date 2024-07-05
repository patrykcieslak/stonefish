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
//  Light.h
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017-2024 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Light__
#define __Stonefish_Light__

#include "visuals/Visual.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    class OpenGLLight;
	class StaticEntity;
    class AnimatedEntity;
    
    //! A class representing a light of two common types: omni and spot.
    class Light : public Visual
    {
    public:
        //! A constructor of an omni light.
        /*!
         \param uniqueName a name of the light
		 \param radius a radius of the light source [m]
         \param color a color of the light
         \param lum the luminous power of the light [lm]
         */
        Light(std::string uniqueName, Scalar radius, Color color, Scalar lum);
        
        //! A constructor of a spot light.
        /*!
         \param uniqueName a name of the light
		 \param radius a radius of the light source [m]
         \param coneAngleDeg a cone angle of the spot light in degrees [deg]
         \param color a color of the light
         \param lum the luminous power of the light [lm]
         */
        Light(std::string uniqueName, Scalar radius, Scalar coneAngleDeg, Color color, Scalar lum);
        
        //! A method that updates light position.
        void UpdateTransform() override;
        
        //! A method implementing the rendering of the light dummy.
        std::vector<Renderable> Render() override;
        
        //! A method returning the type of the visual.
        VisualType getType() const override;
        
    private:
        void InitGraphics();
        
        Color c;
		Scalar R;
        Scalar Fi;
        Scalar coneAngle;
        OpenGLLight* glLight;
    };
}

#endif
