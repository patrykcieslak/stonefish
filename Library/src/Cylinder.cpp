//
//  Cylinder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include <entities/solids/Cylinder.h>

Cylinder::Cylinder(std::string uniqueName, btScalar cylinderRadius, btScalar cylinderHeight, Material m, int lookId, btScalar thickness, bool isBuoyant) : SolidEntity(uniqueName, m, lookId, thickness, isBuoyant)
{
    radius = UnitSystem::SetLength(cylinderRadius);
    halfHeight = UnitSystem::SetLength(cylinderHeight/btScalar(2));
    
    //Calculate physical properties
    if(thick > btScalar(0) && thick/btScalar(2) < radius && thick/btScalar(2) < halfHeight)
    {
        btScalar r1 = radius - thick/btScalar(2);
        btScalar r2 = radius + thick/btScalar(2);
        btScalar h1 = halfHeight - thick/btScalar(2);
        btScalar h2 = halfHeight + thick/btScalar(2);
        volume = M_PI*(r2*r2*h2 - r1*r1*h1)*btScalar(2);
        mass = volume * mat.density;
        btScalar m1 = M_PI*(r1*r1*h1)*btScalar(2)*mat.density;
        btScalar m2 = M_PI*(r2*r2*h2)*btScalar(2)*mat.density;
        btScalar Ia = m2*r2*r2/btScalar(2) - m1*r1*r1/btScalar(2); 
        btScalar Ip = m2*(btScalar(3)*r2*r2 + (h2*btScalar(2))*(h2*btScalar(2)))/btScalar(12) - m1*(btScalar(3)*r1*r1 + (h1*btScalar(2))*(h1*btScalar(2)))/btScalar(12);
        Ipri = btVector3(Ip,Ia,Ip);
    }
    else
    {
        volume = M_PI*radius*radius*halfHeight*btScalar(2);
        mass = volume * mat.density;
        btScalar Ia = mass*radius*radius/btScalar(2);
        btScalar Ip = mass*(btScalar(3)*radius*radius + (halfHeight*btScalar(2))*(halfHeight*btScalar(2)))/btScalar(12);
        Ipri = btVector3(Ip,Ia,Ip);
    }
    
    //dragCoeff = btVector3(radius*halfHeight*btScalar(4*0.5), M_PI*radius*radius*btScalar(0.9), radius*halfHeight*btScalar(4*0.5));
    
	mesh = OpenGLContent::BuildCylinder(radius, halfHeight*(GLfloat)2);
	ComputeHydrodynamicProxy(HYDRO_PROXY_CYLINDER);
    
#ifdef DEBUG
    std::cout << getName() << " m:" << mass << " I:" << Ipri.x() << "," << Ipri.y() << "," << Ipri.z() << std::endl;
#endif
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
    btCylinderShape* colShape = new btCylinderShape(btVector3(radius, halfHeight, radius));
    //colShape->setMargin(0.0);
    return colShape;
}
