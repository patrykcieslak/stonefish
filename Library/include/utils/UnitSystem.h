//
//  UnitSystem.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/24/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_UnitSystem__
#define __Stonefish_UnitSystem__

#include "StonefishCommon.h"

#define FULL_ANGLE  Scalar(2. * M_PI)
#define PIover180   Scalar(M_PI / 180.0)
#define _180overPI  Scalar(180.0 / M_PI)

namespace sf
{
    typedef enum {CGS = 0, MKS, MMKS} UnitSystems;
    
    //! A utility class used to convert between unit systems.
    class UnitSystem
    {
    public:
        UnitSystem();
        
        static Scalar Length(UnitSystems from, UnitSystems to, Scalar value);               //0
        static Scalar Mass(UnitSystems from, UnitSystems to, Scalar value);                 //1
        static Vector3 Inertia(UnitSystems from, UnitSystems to, const Vector3& value);     //2
        static Scalar Angle(bool degToRad, Scalar value);
        static Scalar Density(UnitSystems from, UnitSystems to, Scalar value);              //3
        static Scalar Volume(UnitSystems from, UnitSystems to, Scalar value);               //4
        static Scalar Area(UnitSystems from, UnitSystems to, Scalar value);                 //5
        static Scalar Pressure(UnitSystems from, UnitSystems to, Scalar value);             //6
        static Vector3 Position(UnitSystems from, UnitSystems to, const Vector3& value);    //0
        static Vector3 Orientation(bool degToRad, const Vector3& value);
        static Transform Transformation(UnitSystems from, UnitSystems to, const Transform& value);
        static Vector3 Velocity(UnitSystems from, UnitSystems to, const Vector3& value);
        static Scalar Velocity(UnitSystems from, UnitSystems to, Scalar value);
        static Vector3 Acceleration(UnitSystems from, UnitSystems to, const Vector3& value);
        static Scalar Acceleration(UnitSystems from, UnitSystems to, Scalar value);
        static Vector3 AngularVelocity(bool degToRad, const Vector3& value);
        static Scalar AngularVelocity(bool degToRad, Scalar value);
        static Vector3 AngularAcceleration(bool degToRad, const Vector3& value);
        static Scalar AngularAcceleration(bool degToRad, Scalar value);
        static Vector3 Force(UnitSystems from, UnitSystems to, const Vector3& value);       //7
        static Scalar Force(UnitSystems from, UnitSystems to, Scalar value);
        static Vector3 Torque(UnitSystems from, UnitSystems to, const Vector3& value);      //8
        static Scalar Torque(UnitSystems from, UnitSystems to, Scalar value);
        
    private:
        static Scalar Convert(unsigned int quantity, UnitSystems from, UnitSystems to, Scalar value);
        static Vector3 Convert(unsigned int quantity, UnitSystems from, UnitSystems to, const Vector3& value);
        static const Scalar CGStoOther[2][9];
    };
}

#endif
