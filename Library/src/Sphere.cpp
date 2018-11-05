//
//  Sphere.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include <entities/solids/Sphere.h>

/*! Spherical solid entity constructor.
 *
 *  \param uniqueName name of the entity
 *  \param sphereRadius radius of the sphere
 *  \param mat pointer to a physical material
 *  \param l rendering style
 */
Sphere::Sphere(std::string uniqueName, btScalar sphereRadius, Material m, int lookId, btScalar thickness, bool isBuoyant) : SolidEntity(uniqueName, m, lookId, thickness, isBuoyant)
{
    radius = UnitSystem::SetLength(sphereRadius);
    
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
    
    //dragCoeff = btVector3(btScalar(0.47)*M_PI*radius*radius, btScalar(0.47)*M_PI*radius*radius, btScalar(0.47)*M_PI*radius*radius);
    
    mesh = OpenGLContent::BuildSphere(radius);
	ComputeHydrodynamicProxy(HYDRO_PROXY_SPHERE);
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
	//btVector3 pos(0,0,0);
	//btMultiSphereShape* colShape = new btMultiSphereShape(&pos, &radius, 1);
    btSphereShape* colShape = new btSphereShape(radius);
	//colShape->setMargin(0.0);
    return colShape;
}