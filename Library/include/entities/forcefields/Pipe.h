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
//  Pipe.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/07/18.
//  Copyright(c) 2018-2021 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Pipe__
#define __Stonefish_Pipe__

#include "entities/forcefields/VelocityField.h"

namespace sf
{
    //! Pipe (current) velocity field class.
    /*!
     Class implements a velocity field in a shape of a simple tube with variable diameter.
     The flow velocity is specified at the centre of the beginning of the tube.
     The closer to the boundary the slower the flow (zero at boudary).
     */
    class Pipe : public VelocityField
    {
    public:
        //! A constructor.
        /*!
         \param point1 the beginning of the pipe in world frame [m]
         \param point2 the end of the pipe in world frame [m]
         \param radius1 the radius at the beginning of the pipe [m]
         \param radius2 the radius at the end of the pipe [m]
         \param inletVelocity the velocity of fluid at the beginning of the pipe [m/s]
         \param exponent a factor determining the velicty profile along the perimeter of the pipe
         */
        Pipe(const Vector3& point1, const Vector3& point2, Scalar radius1, Scalar radius2, Scalar inletVelocity, Scalar exponent);
        
        //! A method returning velocity at a specified point.
        /*!
         \param p a point at which the velocity is requested
         \return velocity [m/s]
         */
        Vector3 GetVelocityAtPoint(const Vector3& p) const;
        
        //! A method implementing the rendering of the pipe.
        std::vector<Renderable> Render(VelocityFieldUBO& ubo);

         //! A method returning the type of the velocity field.
        VelocityFieldType getType() const;
        
    private:
        Vector3 p1, n;
        Scalar r1, r2, l;
        Scalar vin;
        Scalar gamma;
    };
}

#endif
