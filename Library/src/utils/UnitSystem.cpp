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
//  UnitSystem.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/24/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#include "utils/UnitSystem.h"

namespace sf
{

const Scalar UnitSystem::CGStoOther[2][9] = //MKS = 0 , MMKS = 1
{
    {1e-2, 1e-3, 1e-7, 1e3,  1e-6, 1e-4, 1e-1, 1e-5, 1e-7},
    {1e1,  1e-3, 1e-1, 1e-6, 1e3,  1e2,  1e-4, 1e-2, 1e-1}
};

Scalar UnitSystem::Convert(unsigned int quantity, UnitSystems from, UnitSystems to, Scalar value)
{
    if(from == to)
        return value;
    else
    {
        if(from == CGS)
            return value*CGStoOther[to-1][quantity];
        else if(to == CGS)
            return value/CGStoOther[from-1][quantity];
        else
            return value/CGStoOther[from-1][quantity] * CGStoOther[to-1][quantity];
    }
}

Vector3 UnitSystem::Convert(unsigned int quantity, UnitSystems from, UnitSystems to, const Vector3& value)
{
    if(from == to)
        return value;
    else
    {
        if(from == CGS)
            return value*CGStoOther[to-1][quantity];
        else if(to == CGS)
            return value/CGStoOther[from-1][quantity];
        else
            return value/CGStoOther[from-1][quantity] * CGStoOther[to-1][quantity];
    }
}

Scalar UnitSystem::Length(UnitSystems from, UnitSystems to, Scalar value)
{
    return Convert(0, from, to, value);
}

Scalar UnitSystem::Mass(UnitSystems from, UnitSystems to, Scalar value)
{
    return Convert(1, from, to, value);
}

Vector3 UnitSystem::Inertia(UnitSystems from, UnitSystems to, const Vector3& value)
{
    return Convert(2, from, to, value);
}

Scalar UnitSystem::Angle(bool degToRad, Scalar value)
{
    if(degToRad)
        return (value/180.0) * M_PI;
    else
        return (value/M_PI) * 180.0;
}

Scalar UnitSystem::Density(UnitSystems from, UnitSystems to, Scalar value)
{
    return Convert(3, from, to, value);
}

Scalar UnitSystem::Volume(UnitSystems from, UnitSystems to, Scalar value)
{
    return Convert(4, from, to, value);
}

Scalar UnitSystem::Area(UnitSystems from, UnitSystems to, Scalar value)
{
    return Convert(5, from, to, value);
}

Scalar UnitSystem::Pressure(UnitSystems from, UnitSystems to, Scalar value)
{
    return Convert(6, from, to, value);
}

Vector3 UnitSystem::Position(UnitSystems from, UnitSystems to, const Vector3 &value)
{
    return Convert(0, from, to, value);
}

Vector3 UnitSystem::Orientation(bool degToRad, const Vector3 &value)
{
    if(degToRad)
        return value*PIover180;
    else
        return value*_180overPI;
}

Transform UnitSystem::Transformation(UnitSystems from, UnitSystems to, const Transform &value)
{
    return Transform(value.getBasis(), Convert(0, from, to, value.getOrigin()));
}

Vector3 UnitSystem::Velocity(UnitSystems from, UnitSystems to, const Vector3 &value)
{
    return Convert(0, from, to, value);
}

Scalar UnitSystem::Velocity(UnitSystems from, UnitSystems to, Scalar value)
{
    return Convert(0, from, to, value);
}

Vector3 UnitSystem::Acceleration(UnitSystems from, UnitSystems to, const Vector3 &value)
{
    return Convert(0, from, to, value);
}

Scalar UnitSystem::Acceleration(UnitSystems from, UnitSystems to, Scalar value)
{
    return Convert(0, from, to, value);
}

Vector3 UnitSystem::AngularVelocity(bool degToRad, const Vector3 &value)
{
    return Orientation(degToRad, value);
}

Scalar UnitSystem::AngularVelocity(bool degToRad, Scalar value)
{
    return Angle(degToRad, value);
}

Vector3 UnitSystem::AngularAcceleration(bool degToRad, const Vector3 &value)
{
    return Orientation(degToRad, value);
}

Scalar UnitSystem::AngularAcceleration(bool degToRad, Scalar value)
{
    return Angle(degToRad, value);
}

Vector3 UnitSystem::Force(UnitSystems from, UnitSystems to, const Vector3 &value)
{
    return Convert(7, from, to, value);
}

Scalar UnitSystem::Force(UnitSystems from, UnitSystems to, Scalar value)
{
    return Convert(7, from, to, value);
}

Vector3 UnitSystem::Torque(UnitSystems from, UnitSystems to, const Vector3 &value)
{
    return Convert(8, from, to, value);
}

Scalar UnitSystem::Torque(UnitSystems from, UnitSystems to, Scalar value)
{
    return Convert(8, from, to, value);
}

}
