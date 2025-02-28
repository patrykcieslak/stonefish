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
//  Stream.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/07/18.
//  Copyright(c) 2018-2021 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Stream__
#define __Stonefish_Stream__

#include "entities/forcefields/VelocityField.h"

namespace sf
{
    //! Stream (current) velocity field class.
    /*!
     Class implements a velocity field in a shape of a tube along a Hermite spline, with variable diameter.
     The flow velocity is specified at the centre of the beginning of the tube (tanget to spline).
     The closer to the boundary the slower the flow (zero at boudary).
     */
    class Stream : public VelocityField
    {
    public:
        //! A constructor.
        /*!
         \param streamline the list of points of the stream line [m]
         \param radius the radius of the stream at each point of stream line [m]
         \param inputVelocity the velocity at the beginning of the stream [m/s]
         \param exponent a factor
         */
        Stream(const std::vector<Vector3>& streamline, const std::vector<Scalar>& radius, Scalar inputVelocity, Scalar exponent);
        
        //! A method returning velocity at a specified point.
        /*!
         \param p a point at which the velocity is requested
         \return velocity [m/s]
         */
        Vector3 GetVelocityAtPoint(const Vector3& p) const;
        
        //! A method implementing the rendering of the stream.
        std::vector<Renderable> Render(VelocityFieldUBO& ubo);

         //! A method returning the type of the velocity field.
        VelocityFieldType getType() const;
        
    private:
        std::vector<Vector3> c;
        std::vector<Scalar> r;
        Scalar vin;
        Scalar gamma;
    };
}

#endif
