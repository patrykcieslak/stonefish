//
//  Cylinder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Cylinder.h"

#include "utils/MathUtil.hpp"

using namespace sf;

Cylinder::Cylinder(std::string uniqueName, Scalar cylinderRadius, Scalar cylinderHeight, const Transform& originTrans, Material m, int lookId, Scalar thickness, bool isBuoyant) : SolidEntity(uniqueName, m, lookId, thickness, isBuoyant)
{
    radius = cylinderRadius;
    halfHeight = cylinderHeight/Scalar(2);
    T_G2CG = originTrans;
    
    //Calculate physical properties
    if(thick > Scalar(0) && thick/Scalar(2) < radius && thick/Scalar(2) < halfHeight)
    {
        Scalar r1 = radius - thick/Scalar(2);
        Scalar r2 = radius + thick/Scalar(2);
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
        volume = M_PI*radius*radius*halfHeight*Scalar(2);
        mass = volume * mat.density;
        Scalar Ia = mass*radius*radius/Scalar(2);
        Scalar Ip = mass*(Scalar(3)*radius*radius + (halfHeight*Scalar(2))*(halfHeight*Scalar(2)))/Scalar(12);
        Ipri = Vector3(Ip,Ip,Ia);
    }
    
    //Build geometry
    mesh = OpenGLContent::BuildCylinder(radius, halfHeight*(GLfloat)2);
    transformMesh(mesh, T_G2CG);
    
    //Compute hydrodynamic properties
    ComputeHydrodynamicProxy(HYDRO_PROXY_CYLINDER);
    CB = T_G2CG.getOrigin();
    //dragCoeff = Vector3(radius*halfHeight*Scalar(4*0.5), M_PI*radius*radius*Scalar(0.9), radius*halfHeight*Scalar(4*0.5));
}

Cylinder::~Cylinder()
{
}

SolidType Cylinder::getSolidType()
{
    return SOLID_CYLINDER;
}

btCollisionShape* Cylinder::BuildCollisionShape()
{
    btCylinderShape* cylShape = new btCylinderShapeZ(Vector3(radius, radius, halfHeight));
    btCompoundShape* colShape = new btCompoundShape();
    colShape->addChildShape(T_G2CG, cylShape);
    //colShape->setMargin(0.0);
    return colShape;
}
