//
//  UnitSystem.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/24/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_UnitSystem__
#define __Stonefish_UnitSystem__

#include <BulletDynamics/btBulletDynamicsCommon.h>

#define PIover180   0.01745329
#define _180overPI 57.29577951

typedef enum {CGS = 0, MKS, MMKS} UnitSystems;

class UnitSystem
{
public:
    //external
    static void SetUnitSystem(UnitSystems unitSystem, bool useDegrees);
    static UnitSystems GetUnitSystem();
    //internal
    static UnitSystems GetInternalUnitSystem();
    
    //universal
    static btScalar Length(UnitSystems from, UnitSystems to, btScalar value);               //0
    static btScalar Mass(UnitSystems from, UnitSystems to, btScalar value);                 //1
    static btVector3 Inertia(UnitSystems from, UnitSystems to, const btVector3& value);     //2
    static btScalar Angle(bool degToRad, btScalar value);
    static btScalar Density(UnitSystems from, UnitSystems to, btScalar value);              //3
    static btScalar Volume(UnitSystems from, UnitSystems to, btScalar value);               //4
    static btScalar Area(UnitSystems from, UnitSystems to, btScalar value);                 //5
    static btScalar Pressure(UnitSystems from, UnitSystems to, btScalar value);             //6
    static btVector3 Position(UnitSystems from, UnitSystems to, const btVector3& value);
    static btVector3 Orientation(bool degToRad, const btVector3& value);
    static btTransform Transform(UnitSystems from, UnitSystems to, const btTransform& value);
    static btVector3 Velocity(UnitSystems from, UnitSystems to, const btVector3& value);
    static btVector3 Acceleration(UnitSystems from, UnitSystems to, const btVector3& value);
    static btVector3 AngularVelocity(bool degToRad, const btVector3& value);
    static btVector3 AngularAcceleration(bool degToRad, const btVector3& value);
    static btVector3 Force(UnitSystems from, UnitSystems to, const btVector3& value);       //7
    static btVector3 Torque(UnitSystems from, UnitSystems to, const btVector3& value);      //8
    
    //from external to internal
    static btScalar SetLength(btScalar value);
    static btScalar SetMass(btScalar value);
    static btVector3 SetInertia(const btVector3& value);
    static btScalar SetAngle(btScalar value);
    static btScalar SetDensity(btScalar value);
    static btScalar SetVolume(btScalar value);
    static btScalar SetArea(btScalar value);
    static btScalar SetPressure(btScalar value);
    static btVector3 SetPosition(const btVector3& value);
    static btVector3 SetOrientation(const btVector3& value);
    static btTransform SetTransform(const btTransform& value);
    static btVector3 SetVelocity(const btVector3& value);
    static btVector3 SetAcceleration(const btVector3& value);
    static btVector3 SetAngularVelocity(const btVector3& value);
    static btVector3 SetAngularAcceleration(const btVector3& value);
    static btVector3 SetForce(const btVector3& value);
    static btVector3 SetTorque(const btVector3& value);
    
    //from internal to external
    static btScalar GetLength(btScalar value);
    static btScalar GetMass(btScalar value);
    static btVector3 GetInertia(const btVector3& value);
    static btScalar GetAngle(btScalar value);
    static btScalar GetDensity(btScalar value);
    static btScalar GetVolume(btScalar value);
    static btScalar GetArea(btScalar value);
    static btScalar GetPressure(btScalar value);
    static btVector3 GetPosition(const btVector3& value);
    static btVector3 GetOrientation(const btVector3& value);
    static btTransform GetTransform(const btTransform& value);
    static btVector3 GetVelocity(const btVector3& value);
    static btVector3 GetAcceleration(const btVector3& value);
    static btVector3 GetAngularVelocity(const btVector3& value);
    static btVector3 GetAngularAcceleration(const btVector3& value);
    static btVector3 GetForce(const btVector3& value);
    static btVector3 GetTorque(const btVector3& value);
    
private:
    static btScalar Convert(unsigned int quantity, UnitSystems from, UnitSystems to, btScalar value);
    static btVector3 Convert(unsigned int quantity, UnitSystems from, UnitSystems to, const btVector3& value);
    
    static const UnitSystems internalUS = MKS; //+Radians
    static const btScalar CGStoOther[2][9];
    static UnitSystems externalUS;
    static bool externalDeg;
};

#endif