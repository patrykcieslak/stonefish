//
//  UnitSystem.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/24/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "core/UnitSystem.h"

UnitSystems UnitSystem::externalUS = MKS;
bool UnitSystem::externalDeg = true;
const btScalar UnitSystem::CGStoOther[2][9] = //MKS = 0 , MMKS = 1
{
    {1e-2, 1e-3, 1e-7, 1e3,  1e-6, 1e-4, 1e-1, 1e-5, 1e-7},
    {1e1,  1e-3, 1e-1, 1e-6, 1e3,  1e2,  1e-4, 1e-2, 1e-1}
};

void UnitSystem::SetUnitSystem(UnitSystems unitSystem, bool useDegrees)
{
    externalUS = unitSystem;
    externalDeg = useDegrees;
}

UnitSystems UnitSystem::GetUnitSystem()
{
    return externalUS;
}

UnitSystems UnitSystem::GetInternalUnitSystem()
{
    return internalUS;
}

std::string UnitSystem::GetDescription()
{
    std::string desc = "";
    
    switch (externalUS)
    {
        case CGS:
            desc += "CGS";
            break;
            
        case MKS:
            desc += "MKS";
            break;
        
        case MMKS:
            desc += "MMKS";
            break;
    }
    
    if(externalDeg)
        desc += ", degrees";
    else
        desc += ", radians";
    
    return desc;
}

btScalar UnitSystem::Convert(unsigned int quantity, UnitSystems from, UnitSystems to, btScalar value)
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

btVector3 UnitSystem::Convert(unsigned int quantity, UnitSystems from, UnitSystems to, const btVector3& value)
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

btScalar UnitSystem::Length(UnitSystems from, UnitSystems to, btScalar value)
{
    return Convert(0, from, to, value);
}

btScalar UnitSystem::Mass(UnitSystems from, UnitSystems to, btScalar value)
{
    return Convert(1, from, to, value);
}

btVector3 UnitSystem::Inertia(UnitSystems from, UnitSystems to, const btVector3& value)
{
    return Convert(2, from, to, value);
}

btScalar UnitSystem::Angle(bool degToRad, btScalar value)
{
    if(degToRad)
        return (value/180.0) * M_PI;
    else
        return (value/M_PI) * 180.0;
}

btScalar UnitSystem::Density(UnitSystems from, UnitSystems to, btScalar value)
{
    return Convert(3, from, to, value);
}

btScalar UnitSystem::Volume(UnitSystems from, UnitSystems to, btScalar value)
{
    return Convert(4, from, to, value);
}

btScalar UnitSystem::Area(UnitSystems from, UnitSystems to, btScalar value)
{
    return Convert(5, from, to, value);
}

btScalar UnitSystem::Pressure(UnitSystems from, UnitSystems to, btScalar value)
{
    return Convert(6, from, to, value);
}

btVector3 UnitSystem::Position(UnitSystems from, UnitSystems to, const btVector3 &value)
{
    return Convert(0, from, to, value);
}

btVector3 UnitSystem::Orientation(bool degToRad, const btVector3 &value)
{
    if(degToRad)
        return value*PIover180;
    else
        return value*_180overPI;
}

btTransform UnitSystem::Transform(UnitSystems from, UnitSystems to, const btTransform &value)
{
    return btTransform(value.getBasis(), Convert(0, from, to, value.getOrigin()));
}

btVector3 UnitSystem::Velocity(UnitSystems from, UnitSystems to, const btVector3 &value)
{
    return Convert(0, from, to, value);
}

btScalar UnitSystem::Velocity(UnitSystems from, UnitSystems to, btScalar value)
{
    return Convert(0, from, to, value);
}

btVector3 UnitSystem::Acceleration(UnitSystems from, UnitSystems to, const btVector3 &value)
{
    return Convert(0, from, to, value);
}

btScalar UnitSystem::Acceleration(UnitSystems from, UnitSystems to, btScalar value)
{
    return Convert(0, from, to, value);
}

btVector3 UnitSystem::AngularVelocity(bool degToRad, const btVector3 &value)
{
    return Orientation(degToRad, value);
}

btScalar UnitSystem::AngularVelocity(bool degToRad, btScalar value)
{
    return Angle(degToRad, value);
}

btVector3 UnitSystem::AngularAcceleration(bool degToRad, const btVector3 &value)
{
    return Orientation(degToRad, value);
}

btScalar UnitSystem::AngularAcceleration(bool degToRad, btScalar value)
{
    return Angle(degToRad, value);
}

btVector3 UnitSystem::Force(UnitSystems from, UnitSystems to, const btVector3 &value)
{
    return Convert(7, from, to, value);
}

btScalar UnitSystem::Force(UnitSystems from, UnitSystems to, btScalar value)
{
    return Convert(7, from, to, value);
}

btVector3 UnitSystem::Torque(UnitSystems from, UnitSystems to, const btVector3 &value)
{
    return Convert(8, from, to, value);
}

btScalar UnitSystem::Torque(UnitSystems from, UnitSystems to, btScalar value)
{
    return Convert(8, from, to, value);
}

btScalar UnitSystem::SetLength(btScalar value)
{
    return Convert(0, externalUS, internalUS, value);
}

btScalar UnitSystem::SetMass(btScalar value)
{
    return Convert(1, externalUS, internalUS, value);
}

btVector3 UnitSystem::SetInertia(const btVector3 &value)
{
    return Convert(2, externalUS, internalUS, value);
}

btScalar UnitSystem::SetAngle(btScalar value)
{
    if(externalDeg)
        return Angle(true, value);
    else
        return value;
}

btScalar UnitSystem::SetDensity(btScalar value)
{
    return Convert(3, externalUS, internalUS, value);
}

btScalar UnitSystem::SetVolume(btScalar value)
{
    return Convert(4, externalUS, internalUS, value);
}

btScalar UnitSystem::SetArea(btScalar value)
{
    return Convert(5, externalUS, internalUS, value);
}

btScalar UnitSystem::SetPressure(btScalar value)
{
    return Convert(6, externalUS, internalUS, value);
}

btVector3 UnitSystem::SetPosition(const btVector3 &value)
{
    return Convert(0, externalUS, internalUS, value);
}

btVector3 UnitSystem::SetOrientation(const btVector3 &value)
{
    if(externalDeg)
        return Orientation(true, value);
    else
        return value;
}

btTransform UnitSystem::SetTransform(const btTransform &value)
{
    return btTransform(value.getBasis(), Convert(0, externalUS, internalUS, value.getOrigin()));
}

btVector3 UnitSystem::SetVelocity(const btVector3 &value)
{
    return Convert(0, externalUS, internalUS, value);
}

btScalar UnitSystem::SetVelocity(btScalar value)
{
    return Convert(0, externalUS, internalUS, value);
}

btVector3 UnitSystem::SetAcceleration(const btVector3 &value)
{
    return Convert(0, externalUS, internalUS, value);
}

btScalar UnitSystem::SetAcceleration(btScalar value)
{
    return Convert(0, externalUS, internalUS, value);
}

btVector3 UnitSystem::SetAngularVelocity(const btVector3 &value)
{
    return SetOrientation(value);
}

btScalar UnitSystem::SetAngularVelocity(btScalar value)
{
    return SetAngle(value);
}

btVector3 UnitSystem::SetAngularAcceleration(const btVector3 &value)
{
    return SetOrientation(value);
}

btScalar UnitSystem::SetAngularAcceleration(btScalar value)
{
    return SetAngle(value);
}

btVector3 UnitSystem::SetForce(const btVector3 &value)
{
    return Convert(7, externalUS, internalUS, value);
}

btScalar UnitSystem::SetForce(btScalar value)
{
    return Convert(7, externalUS, internalUS, value);
}

btVector3 UnitSystem::SetTorque(const btVector3 &value)
{
    return Convert(8, externalUS, internalUS, value);
}

btScalar UnitSystem::SetTorque(btScalar value)
{
    return Convert(8, externalUS, internalUS, value);
}

btScalar UnitSystem::GetLength(btScalar value)
{
    return Convert(0, internalUS, externalUS, value);
}

btScalar UnitSystem::GetMass(btScalar value)
{
    return Convert(1, internalUS, externalUS, value);
}

btVector3 UnitSystem::GetInertia(const btVector3 &value)
{
    return Convert(2, internalUS, externalUS, value);
}

btScalar UnitSystem::GetAngle(btScalar value)
{
    if(externalDeg)
        return Angle(false, value);
    else
        return value;
}

btScalar UnitSystem::GetDensity(btScalar value)
{
    return Convert(3, internalUS, externalUS, value);
}

btScalar UnitSystem::GetVolume(btScalar value)
{
    return Convert(4, internalUS, externalUS, value);
}

btScalar UnitSystem::GetArea(btScalar value)
{
    return Convert(5, internalUS, externalUS, value);
}

btScalar UnitSystem::GetPressure(btScalar value)
{
    return Convert(6, internalUS, externalUS, value);
}

btVector3 UnitSystem::GetPosition(const btVector3 &value)
{
    return Convert(0, internalUS, externalUS, value);
}

btVector3 UnitSystem::GetOrientation(const btVector3 &value)
{
    if(externalDeg)
        return Orientation(false, value);
    else
        return value;
}

btTransform UnitSystem::GetTransform(const btTransform &value)
{
    return btTransform(value.getBasis(), Convert(0, internalUS, externalUS, value.getOrigin()));
}

btVector3 UnitSystem::GetVelocity(const btVector3 &value)
{
    return Convert(0, internalUS, externalUS, value);
}

btScalar UnitSystem::GetVelocity(btScalar value)
{
    return Convert(0, internalUS, externalUS, value);
}

btVector3 UnitSystem::GetAcceleration(const btVector3 &value)
{
    return Convert(0, internalUS, externalUS, value);
}

btScalar UnitSystem::GetAcceleration(btScalar value)
{
    return Convert(0, internalUS, externalUS, value);
}

btVector3 UnitSystem::GetAngularVelocity(const btVector3 &value)
{
    return GetOrientation(value);
}

btScalar UnitSystem::GetAngularVelocity(btScalar value)
{
    return GetAngle(value);
}

btVector3 UnitSystem::GetAngularAcceleration(const btVector3 &value)
{
    return GetOrientation(value);
}

btScalar UnitSystem::GetAngularAcceleration(btScalar value)
{
    return GetAngle(value);
}

btVector3 UnitSystem::GetForce(const btVector3 &value)
{
    return Convert(7, internalUS, externalUS, value);
}

btScalar UnitSystem::GetForce(btScalar value)
{
    return Convert(7, internalUS, externalUS, value);
}

btVector3 UnitSystem::GetTorque(const btVector3 &value)
{
    return Convert(8, internalUS, externalUS, value);
}

btScalar UnitSystem::GetTorque(btScalar value)
{
    return Convert(8, internalUS, externalUS, value);
}
