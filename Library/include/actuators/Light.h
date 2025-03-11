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

#include "actuators/LinkActuator.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    class OpenGLLight;
	class StaticEntity;
    class AnimatedEntity;
    
    //! A class representing a light of two common types: omni and spot.
    class Light : public LinkActuator
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
        
		//! A method used to attach the comm device to the world origin.
        /*!
         \param origin the place where the comm should be attached in the world frame
         */
        void AttachToWorld(const Transform& origin);
        
        //! A method used to attach the light to a static body.
        /*!
         \param body a pointer to the static body
         \param origin the place where the light should be attached in the body origin frame
         */
        void AttachToStatic(StaticEntity* body, const Transform& origin);
		
        //! A method used to attach the light to an animated body.
        /*!
         \param body a pointer to the animated body
         \param origin the place where the light should be attached in the body origin frame
         */
        void AttachToAnimated(AnimatedEntity* body, const Transform& origin);

        //! A method used to attach the actuator to a specified rigid body.
        /*!
         \param solid a pointer to a rigid body
         \param origin a transformation from the body origin to the actuator origin
         */
        void AttachToSolid(SolidEntity* body, const Transform& origin);
        
        //! A method which updates the pose of the light
        /*!
         \param dt a time step of the simulation
         */
        void Update(Scalar dt);
        
        //! A method that updates light position.
        void UpdateTransform();
        
        //! A method implementing the rendering of the light dummy.
        std::vector<Renderable> Render();
        
		//! A method returning actuator frame in the world frame.
		Transform getActuatorFrame() const;
		
        //! A method returning the type of the actuator.
        ActuatorType getType() const;
        
        OpenGLLight* getGLLight();
        
    private:
        void InitGraphics();
        
        //attach -> SolidEntity
		StaticEntity* attach2;
        AnimatedEntity* attach3;
        Color c;
		Scalar R;
        Scalar Fi;
        Scalar coneAngle;
        OpenGLLight* glLight;
    };
}

#endif
