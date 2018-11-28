//
//  Torus.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Torus.h"

#include "core/TorusShape.h"
#include "utils/MathUtil.hpp"

using namespace sf;

Torus::Torus(std::string uniqueName, Scalar torusMajorRadius, Scalar torusMinorRadius, const Transform& originTrans, Material m, int lookId, Scalar thickness, bool isBuoyant) : SolidEntity(uniqueName, m, lookId, thickness, isBuoyant)
{
    majorRadius = torusMajorRadius;
    minorRadius = torusMinorRadius;
    T_G2CG = originTrans;
    
    //Calculate physical properties
    if(thick > Scalar(0) && thick/Scalar(2) < minorRadius)
    {
        Scalar mr1 = minorRadius - thick/Scalar(2);
        Scalar mr2 = minorRadius + thick/Scalar(2);
        volume = M_PI*majorRadius*M_PI*(mr2*mr2 - mr1*mr1);
        mass = volume * mat.density;
        Scalar m1 = M_PI*majorRadius*M_PI*mr1*mr1;
        Scalar m2 = M_PI*majorRadius*M_PI*mr2*mr2;
        Scalar Id = (Scalar(4)*majorRadius*majorRadius + Scalar(5)*mr2*mr2)*m2/Scalar(8) - (Scalar(4)*majorRadius*majorRadius + Scalar(5)*mr1*mr1)*m1/Scalar(8);
        Scalar Ia = (majorRadius*majorRadius + Scalar(3)/Scalar(4)*mr2*mr2)*m2 - (majorRadius*majorRadius + Scalar(3)/Scalar(4)*mr1*mr1)*m1;
        Ipri = Vector3(Id,Ia,Id);
    }
    else
    {
        volume = M_PI*minorRadius*minorRadius*M_PI*majorRadius;
        mass = volume * mat.density;
        Scalar Id = (Scalar(4)*majorRadius*majorRadius + Scalar(5)*minorRadius*minorRadius)*mass/Scalar(8);
        Scalar Ia = (majorRadius*majorRadius + Scalar(3)/Scalar(4)*minorRadius*minorRadius)*mass;
        Ipri = Vector3(Id,Ia,Id);
    }
    
    //Build geometry
    mesh = OpenGLContent::BuildTorus(majorRadius, minorRadius);
    transformMesh(mesh, T_G2CG);
    
    //Compute hydrodynamic properties
    ComputeHydrodynamicProxy(HYDRO_PROXY_CYLINDER);
    CB = T_G2CG.getOrigin();
    //dragCoeff = Vector3(0.5, 0.5, 0.5);//Vector3(radius*halfHeight*4.0*0.5, M_PI*radius*radius*0.9, radius*halfHeight*4.0*0.5);
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
    TorusShape* torusShape = new TorusShape(majorRadius, minorRadius);
    btCompoundShape* colShape = new btCompoundShape();
    colShape->addChildShape(T_G2CG, torusShape);
   // colShape->setMargin(0.0);
    return colShape;
}
