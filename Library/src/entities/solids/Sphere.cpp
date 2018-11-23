//
//  Sphere.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Sphere.h"

#include "utils/MathUtil.hpp"

using namespace sf;

Sphere::Sphere(std::string uniqueName, btScalar sphereRadius, const btTransform& originTrans, Material m, int lookId, btScalar thickness, bool isBuoyant) : SolidEntity(uniqueName, m, lookId, thickness, isBuoyant)
{
    radius = UnitSystem::SetLength(sphereRadius);
    localTransform = UnitSystem::SetTransform(originTrans);
    
    //Calculate physical properties
    if(thick > btScalar(0) && thick/btScalar(2) < radius)
    {
        btScalar r1 = radius - thick/btScalar(2);
        btScalar r2 = radius + thick/btScalar(2);
        volume = btScalar(4)/btScalar(3)*M_PI*(r2*r2*r2 - r1*r1*r1);
        mass = volume * mat.density;
        btScalar I = btScalar(2)/btScalar(5)*mass*((r2*r2*r2*r2*r2 - r1*r1*r1*r1*r1)/(r2*r2*r2 - r1*r1*r1));
        Ipri = btVector3(I,I,I);
    }    
    else
    {
        volume = btScalar(4)/btScalar(3)*M_PI*radius*radius*radius;
        mass = volume * mat.density;
        btScalar I = btScalar(2)/btScalar(5)*mass*radius*radius;
        Ipri = btVector3(I,I,I);
    }
    
    //Build geometry
    mesh = OpenGLContent::BuildSphere(radius);
    transformMesh(mesh, localTransform);
    
    //Compute hydrodynamic properties
    ComputeHydrodynamicProxy(HYDRO_PROXY_SPHERE);
    CoB = localTransform.getOrigin();
    //dragCoeff = btVector3(btScalar(0.47)*M_PI*radius*radius, btScalar(0.47)*M_PI*radius*radius, btScalar(0.47)*M_PI*radius*radius);
}

Sphere::~Sphere()
{
}

SolidType Sphere::getSolidType()
{
    return SOLID_SPHERE;
}

btCollisionShape* Sphere::BuildCollisionShape()
{
    btSphereShape* sphereShape = new btSphereShape(radius);
    btCompoundShape* colShape = new btCompoundShape();
    colShape->addChildShape(localTransform, sphereShape);
    //colShape->setMargin(0.0);
    return colShape;
}
