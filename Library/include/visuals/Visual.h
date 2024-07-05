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
//  Visual.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/07/24.
//  Copyright (c) 2024 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Visual__
#define __Stonefish_Visual__

#include "StonefishCommon.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
	class StaticEntity;
    class SolidEntity;
    class AnimatedEntity;

    //! An enum designating a type of the actuator.
    enum class VisualType {LIGHT, FALL, PLUME};
    
    //! A class representing a light of two common types: omni and spot.
    class Visual
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name of the light
		 */
        Visual(std::string uniqueName);
        
        //! A destructor.
        virtual ~Visual();

		//! A method used to attach the visual to the world origin.
        /*!
         \param origin the place where the visual should be attached in the world frame
         */
        void AttachToWorld(const Transform& origin);
        
        //! A method used to attach the visual to a static body.
        /*!
         \param body a pointer to the static body
         \param origin the place where the light should be attached in the body origin frame
         */
        void AttachToStatic(StaticEntity* body, const Transform& origin);
		
        //! A method used to attach the visual to an animated body.
        /*!
         \param body a pointer to the animated body
         \param origin the place where the light should be attached in the body origin frame
         */
        void AttachToAnimated(AnimatedEntity* body, const Transform& origin);

        //! A method used to attach the visual to a specified rigid body.
        /*!
         \param solid a pointer to a rigid body
         \param origin a transformation from the body origin to the actuator origin
         */
        void AttachToSolid(SolidEntity* body, const Transform& origin);
        
        //! A method returning visuals frame in the world frame.
		Transform getVisualFrame() const;

        //! A method that updates visuals position.
        virtual void UpdateTransform() = 0;
        
        //! A method implementing the rendering of the visuals dummy.
        virtual std::vector<Renderable> Render() = 0;
        
        //! A method returning the type of the visual.
        virtual VisualType getType() const = 0;

        //! A method returning the name of the visual.
        std::string getName() const;
        
    protected:
        virtual void InitGraphics() = 0;
        
        std::string name;
        Transform o2a;
        SolidEntity* attach;
		StaticEntity* attach2;
        AnimatedEntity* attach3;
    };
}

#endif
