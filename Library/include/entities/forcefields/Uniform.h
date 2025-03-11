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
//  Uniform.h
//  Stonefish
//
//  Created by Patryk Cieslak on 2/05/19.
//  Copyright(c) 2019-2021 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Uniform__
#define __Stonefish_Uniform__

#include "entities/forcefields/VelocityField.h"

namespace sf
{
    //! Uniform velocity field class.
    /*!
     Class implements a uniform velocity field, i.e., the velocity is constant in the whole domain.
     */
    class Uniform : public VelocityField
    {
    public:
        //! A constructor.
        /*!
         \param velocity the velocity at every point of the field, in the world frame [m/s]
         */
        Uniform(const Vector3& velocity);
        
        //! A method returning velocity at a specified point.
        /*!
         \param p a point at which the velocity is requested
         \return velocity [m/s]
         */
        Vector3 GetVelocityAtPoint(const Vector3& p) const;
        
        //! A method implementing the rendering of the uniform field.
        std::vector<Renderable> Render(VelocityFieldUBO& ubo);

        //! A method to change the flow velocity.
        /*!
         \param x new velocity [m/s]
         */
        void setVelocity(const Vector3& x);

        //! A method returning the type of the velocity field.
        VelocityFieldType getType() const;
        
    private:
        Vector3 v;
    };
}

#endif
