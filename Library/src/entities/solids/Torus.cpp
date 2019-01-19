//
//  Torus.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Torus.h"

#include "core/TorusShape.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Torus::Torus(std::string uniqueName, Scalar majorRadius, Scalar minorRadius, const Transform& origin, Material m, int lookId, Scalar thickness, bool enableHydrodynamicForces, bool isBuoyant)
    : SolidEntity(uniqueName, m, lookId, thickness, enableHydrodynamicForces, isBuoyant)
{
    MR = majorRadius;
    mR = minorRadius;
    T_O2G = T_O2C = origin;
    T_CG2O = origin.inverse();
    T_CG2C = T_CG2G = I4();
    P_CB = Vector3(0,0,0);
    
    //Calculate physical properties
    if(thick > Scalar(0) && thick/Scalar(2) < mR)
    {
        Scalar mr1 = mR - thick/Scalar(2);
        Scalar mr2 = mR + thick/Scalar(2);
        volume = M_PI*MR*M_PI*(mr2*mr2 - mr1*mr1);
        mass = volume * mat.density;
        Scalar m1 = M_PI*MR*M_PI*mr1*mr1;
        Scalar m2 = M_PI*MR*M_PI*mr2*mr2;
        Scalar Id = (Scalar(4)*MR*MR + Scalar(5)*mr2*mr2)*m2/Scalar(8) - (Scalar(4)*MR*MR + Scalar(5)*mr1*mr1)*m1/Scalar(8);
        Scalar Ia = (MR*MR + Scalar(3)/Scalar(4)*mr2*mr2)*m2 - (MR*MR + Scalar(3)/Scalar(4)*mr1*mr1)*m1;
        Ipri = Vector3(Id,Ia,Id);
    }
    else
    {
        volume = M_PI*mR*mR*M_PI*MR;
        mass = volume * mat.density;
        Scalar Id = (Scalar(4)*MR*MR + Scalar(5)*mR*mR)*mass/Scalar(8);
        Scalar Ia = (MR*MR + Scalar(3)/Scalar(4)*mR*mR)*mass;
        Ipri = Vector3(Id,Ia,Id);
    }
    
    //Build geometry
    phyMesh = OpenGLContent::BuildTorus(MR, mR);
    
    //Compute hydrodynamic properties
    ComputeHydrodynamicProxy(HYDRO_PROXY_CYLINDER);
    //dragCoeff = Vector3(0.5, 0.5, 0.5);//Vector3(radius*halfHeight*4.0*0.5, M_PI*radius*radius*0.9, radius*halfHeight*4.0*0.5);
}

SolidType Torus::getSolidType()
{
    return SolidType::SOLID_TORUS;
}

btCollisionShape* Torus::BuildCollisionShape()
{
    return new TorusShape(MR, mR);
}

}
