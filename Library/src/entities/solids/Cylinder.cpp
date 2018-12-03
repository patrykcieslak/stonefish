//
//  Cylinder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Cylinder.h"

#include "graphics/OpenGLContent.h"
#include "utils/MathUtil.hpp"

namespace sf
{

Cylinder::Cylinder(std::string uniqueName, Scalar radius, Scalar height, const Transform& origin, Material m, int lookId, Scalar thickness, bool isBuoyant) : SolidEntity(uniqueName, m, lookId, thickness, isBuoyant)
{
    r = radius;
    halfHeight = height/Scalar(2);
    T_O2G = T_O2C = origin;
    T_CG2O = origin.inverse();
    T_CG2C = T_CG2G = I4();
    P_CB = Vector3(0,0,0);
    
    //Calculate physical properties
    if(thick > Scalar(0) && thick/Scalar(2) < r && thick/Scalar(2) < halfHeight)
    {
        Scalar r1 = r - thick/Scalar(2);
        Scalar r2 = r + thick/Scalar(2);
        Scalar h1 = halfHeight - thick/Scalar(2);
        Scalar h2 = halfHeight + thick/Scalar(2);
        volume = M_PI*(r2*r2*h2 - r1*r1*h1)*Scalar(2);
        mass = volume * mat.density;
        Scalar m1 = M_PI*(r1*r1*h1)*Scalar(2)*mat.density;
        Scalar m2 = M_PI*(r2*r2*h2)*Scalar(2)*mat.density;
        Scalar Ia = m2*r2*r2/Scalar(2) - m1*r1*r1/Scalar(2); 
        Scalar Ip = m2*(Scalar(3)*r2*r2 + (h2*Scalar(2))*(h2*Scalar(2)))/Scalar(12) - m1*(Scalar(3)*r1*r1 + (h1*Scalar(2))*(h1*Scalar(2)))/Scalar(12);
        Ipri = Vector3(Ip,Ip,Ia);
    }
    else
    {
        volume = M_PI*r*r*halfHeight*Scalar(2);
        mass = volume * mat.density;
        Scalar Ia = mass*r*r/Scalar(2);
        Scalar Ip = mass*(Scalar(3)*r*r + (halfHeight*Scalar(2))*(halfHeight*Scalar(2)))/Scalar(12);
        Ipri = Vector3(Ip,Ip,Ia);
    }
    
    //Build geometry
    phyMesh = OpenGLContent::BuildCylinder((GLfloat)r, (GLfloat)(halfHeight*2));
    
    //Compute hydrodynamic properties
    ComputeHydrodynamicProxy(HYDRO_PROXY_CYLINDER);
    //dragCoeff = Vector3(radius*halfHeight*Scalar(4*0.5), M_PI*radius*radius*Scalar(0.9), radius*halfHeight*Scalar(4*0.5));
}

SolidType Cylinder::getSolidType()
{
    return SOLID_CYLINDER;
}

btCollisionShape* Cylinder::BuildCollisionShape()
{
    return new btCylinderShapeZ(Vector3(r, r, halfHeight));
}

}
