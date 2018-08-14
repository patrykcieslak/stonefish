//
//  Torus.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "Torus.h"
#include "TorusShape.h"

Torus::Torus(std::string uniqueName, btScalar torusMajorRadius, btScalar torusMinorRadius, Material m, int lookId, btScalar thickness, bool isBuoyant) : SolidEntity(uniqueName, m, lookId, thickness, isBuoyant)
{
    majorRadius = UnitSystem::SetLength(torusMajorRadius);
    minorRadius = UnitSystem::SetLength(torusMinorRadius);
    
    //Calculate physical properties
    if(thick > btScalar(0) && thick/btScalar(2) < minorRadius)
    {
        btScalar mr1 = minorRadius - thick/btScalar(2);
        btScalar mr2 = minorRadius + thick/btScalar(2);
        volume = M_PI*majorRadius*M_PI*(mr2*mr2 - mr1*mr1);
        mass = volume * mat.density;
        btScalar m1 = M_PI*majorRadius*M_PI*mr1*mr1;
        btScalar m2 = M_PI*majorRadius*M_PI*mr2*mr2;
        btScalar Id = (btScalar(4)*majorRadius*majorRadius + btScalar(5)*mr2*mr2)*m2/btScalar(8) - (btScalar(4)*majorRadius*majorRadius + btScalar(5)*mr1*mr1)*m1/btScalar(8);
        btScalar Ia = (majorRadius*majorRadius + btScalar(3)/btScalar(4)*mr2*mr2)*m2 - (majorRadius*majorRadius + btScalar(3)/btScalar(4)*mr1*mr1)*m1;
        Ipri = btVector3(Id,Ia,Id);
    }
    else
    {
        volume = M_PI*minorRadius*minorRadius*M_PI*majorRadius;
        mass = volume * mat.density;
        btScalar Id = (btScalar(4)*majorRadius*majorRadius + btScalar(5)*minorRadius*minorRadius)*mass/btScalar(8);
        btScalar Ia = (majorRadius*majorRadius + btScalar(3)/btScalar(4)*minorRadius*minorRadius)*mass;
        Ipri = btVector3(Id,Ia,Id);
    }
    
    //dragCoeff = btVector3(0.5, 0.5, 0.5);//btVector3(radius*halfHeight*4.0*0.5, M_PI*radius*radius*0.9, radius*halfHeight*4.0*0.5);
    
	mesh = OpenGLContent::BuildTorus(majorRadius, minorRadius);
	ComputeHydrodynamicProxy(HYDRO_PROXY_CYLINDER);
}

Torus::~Torus()
{
}

SolidType Torus::getSolidType()
{
    return SOLID_TORUS;
}

btCollisionShape* Torus::BuildCollisionShape()
{
    TorusShape* colShape = new TorusShape(majorRadius, minorRadius);
   // colShape->setMargin(0.0);
    return colShape;
}
