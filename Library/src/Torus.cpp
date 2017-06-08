//
//  Torus.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "Torus.h"
#include "TorusShape.h"

Torus::Torus(std::string uniqueName, btScalar torusMajorRadius, btScalar torusMinorRadius, Material* mat, int lookId) : SolidEntity(uniqueName, mat, lookId)
{
    majorRadius = UnitSystem::SetLength(torusMajorRadius);
    minorRadius = UnitSystem::SetLength(torusMinorRadius);
    material = mat;
    
    //Calculate physical properties
    dragCoeff = btVector3(0.5, 0.5, 0.5);//btVector3(radius*halfHeight*4.0*0.5, M_PI*radius*radius*0.9, radius*halfHeight*4.0*0.5);
    volume = M_PI*minorRadius*minorRadius*btScalar(0.5*2)*M_PI*majorRadius;
    mass = volume * material->density;
    btScalar idiam, ivert;
	idiam = btScalar(1)/btScalar(8)*(btScalar(4)*majorRadius*majorRadius + btScalar(5)*minorRadius*minorRadius)*mass;
	ivert = (majorRadius*majorRadius + btScalar(3)/btScalar(4)*minorRadius*minorRadius)*mass;
	Ipri = btVector3(ivert, idiam, ivert);
	
	mesh = OpenGLContent::BuildTorus(majorRadius, minorRadius);
}

Torus::~Torus()
{
}

SolidEntityType Torus::getSolidType()
{
    return SOLID_TORUS;
}

btCollisionShape* Torus::BuildCollisionShape()
{
    TorusShape* colShape = new TorusShape(majorRadius, minorRadius);
    colShape->setMargin(UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), btScalar(0.001)));
    return colShape;
}
