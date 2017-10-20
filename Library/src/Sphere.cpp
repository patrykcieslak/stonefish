//
//  Sphere.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "Sphere.h"

/*! Spherical solid entity constructor.
 *
 *  \param uniqueName name of the entity
 *  \param sphereRadius radius of the sphere
 *  \param mat pointer to a physical material
 *  \param l rendering style
 */
Sphere::Sphere(std::string uniqueName, btScalar sphereRadius, Material m, int lookId) : SolidEntity(uniqueName, m, lookId)
{
    radius = UnitSystem::SetLength(sphereRadius);
    
    //Calculate physical properties
    volume = btScalar(4)/btScalar(3)*M_PI*radius*radius*radius;
    //dragCoeff = btVector3(btScalar(0.47)*M_PI*radius*radius, btScalar(0.47)*M_PI*radius*radius, btScalar(0.47)*M_PI*radius*radius);
    mass = volume * mat.density;
    Ipri = btVector3(btScalar(2)/btScalar(5)*mass*radius*radius,
                     btScalar(2)/btScalar(5)*mass*radius*radius,
                     btScalar(2)/btScalar(5)*mass*radius*radius);
					 
	mesh = OpenGLContent::BuildSphere(radius);
	ComputeEquivEllipsoid();
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
    btSphereShape* colShape = new btSphereShape(radius);
    colShape->setMargin(UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), btScalar(0.001)));
    return colShape;
}