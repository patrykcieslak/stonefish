//
//  Sphere.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Sphere.h"

#include "graphics/OpenGLContent.h"
#include "utils/MathUtil.hpp"

namespace sf
{

Sphere::Sphere(std::string uniqueName, Scalar radius, const Transform& origin, Material m, int lookId, Scalar thickness, bool isBuoyant) : SolidEntity(uniqueName, m, lookId, thickness, isBuoyant)
{
    r = radius;
    T_O2G = T_O2C = origin;
    T_CG2O = origin.inverse();
    T_CG2C = T_CG2G = I4();
    P_CB = Vector3(0,0,0);
    
    //Calculate physical properties
    if(thick > Scalar(0) && thick/Scalar(2) < radius)
    {
        Scalar r1 = r - thick/Scalar(2);
        Scalar r2 = r + thick/Scalar(2);
        volume = Scalar(4)/Scalar(3)*M_PI*(r2*r2*r2 - r1*r1*r1);
        mass = volume * mat.density;
        Scalar I = Scalar(2)/Scalar(5)*mass*((r2*r2*r2*r2*r2 - r1*r1*r1*r1*r1)/(r2*r2*r2 - r1*r1*r1));
        Ipri = Vector3(I,I,I);
    }    
    else
    {
        volume = Scalar(4)/Scalar(3)*M_PI*radius*r*r;
        mass = volume * mat.density;
        Scalar I = Scalar(2)/Scalar(5)*mass*r*r;
        Ipri = Vector3(I,I,I);
    }
    
    //Build geometry
    phyMesh = OpenGLContent::BuildSphere((GLfloat)r);
    
    //Compute hydrodynamic properties
    ComputeHydrodynamicProxy(HYDRO_PROXY_SPHERE);
    //dragCoeff = Vector3(Scalar(0.47)*M_PI*radius*radius, Scalar(0.47)*M_PI*radius*radius, Scalar(0.47)*M_PI*radius*radius);
}

SolidType Sphere::getSolidType()
{
    return SOLID_SPHERE;
}

btCollisionShape* Sphere::BuildCollisionShape()
{
    return new btSphereShape(r);
}

}
