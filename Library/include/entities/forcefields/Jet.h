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
//  Jet.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/07/18.
//  Copyright(c) 2018-2021 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Jet__
#define __Stonefish_Jet__

#include "entities/forcefields/VelocityField.h"

namespace sf
{
    //! Jet velocity field class.
    /*!
     Class implements a velocity field coming from a water jet.
     The flow velocity is specified at the centre of the jet outlet.
     The closer to the outlet boundary the slower the flow (zero at boudary).
     */
    class Jet : public VelocityField
    {
    public:
        //! A constructor.
        /*!
         \param point the center of the jet in the world frame [m]
         \param direction the direction of the jet axis in the world frame
         \param radius the radius of the jet outlet [m]
         \param outletVelocity the velocity at the outlet [m/s]
         */
        Jet(const Vector3& point, const Vector3& direction, Scalar radius, Scalar outletVelocity);
        
        //! A method returning velocity at a specified point.
        /*!
         \param p a point at which the velocity is requested
         \return velocity [m/s]
         */
        Vector3 GetVelocityAtPoint(const Vector3& p) const;
        
        //! A method implementing the rendering of the jet.
        std::vector<Renderable> Render(VelocityFieldUBO& ubo);

        //! A method to change the flow velocity.
        /*!
         \param x new outlet velocity [m/s]
         */
        void setOutletVelocity(Scalar x);

         //! A method returning the type of the velocity field.
        VelocityFieldType getType() const;
        
    private:
        Vector3 c, n;
        Scalar r;
        Scalar vout;
    };
}

#endif
