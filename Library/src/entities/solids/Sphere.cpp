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

Sphere::Sphere(std::string uniqueName, Scalar sphereRadius, const Transform& originTrans, Material m, int lookId, Scalar thickness, bool isBuoyant) : SolidEntity(uniqueName, m, lookId, thickness, isBuoyant)
{
    radius = sphereRadius;
    T_G2CG = originTrans;
    
    //Calculate physical properties
    if(thick > Scalar(0) && thick/Scalar(2) < radius)
    {
        Scalar r1 = radius - thick/Scalar(2);
        Scalar r2 = radius + thick/Scalar(2);
        volume = Scalar(4)/Scalar(3)*M_PI*(r2*r2*r2 - r1*r1*r1);
        mass = volume * mat.density;
        Scalar I = Scalar(2)/Scalar(5)*mass*((r2*r2*r2*r2*r2 - r1*r1*r1*r1*r1)/(r2*r2*r2 - r1*r1*r1));
        Ipri = Vector3(I,I,I);
    }    
    else
    {
        volume = Scalar(4)/Scalar(3)*M_PI*radius*radius*radius;
        mass = volume * mat.density;
        Scalar I = Scalar(2)/Scalar(5)*mass*radius*radius;
        Ipri = Vector3(I,I,I);
    }
    
    //Build geometry
    mesh = OpenGLContent::BuildSphere(radius);
    transformMesh(mesh, T_G2CG);
    
    //Compute hydrodynamic properties
    ComputeHydrodynamicProxy(HYDRO_PROXY_SPHERE);
    CB = T_G2CG.getOrigin();
    //dragCoeff = Vector3(Scalar(0.47)*M_PI*radius*radius, Scalar(0.47)*M_PI*radius*radius, Scalar(0.47)*M_PI*radius*radius);
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
    colShape->addChildShape(T_G2CG, sphereShape);
    //colShape->setMargin(0.0);
    return colShape;
}
