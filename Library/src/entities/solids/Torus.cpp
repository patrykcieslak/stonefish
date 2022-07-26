/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  Torus.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/01/13.
//  Copyright (c) 2013-2021 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Torus.h"

#include "core/TorusShape.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Torus::Torus(std::string uniqueName, BodyPhysicsSettings phy, Scalar majorRadius, Scalar minorRadius, const Transform& origin, std::string material, std::string look, Scalar thickness)
    : SolidEntity(uniqueName, phy, material, look, thickness)
{
    MR = majorRadius;
    mR = minorRadius;
    T_O2G = T_O2C = T_O2H = origin;
    T_CG2O = origin.inverse();
    T_CG2C = T_CG2G = I4();
    P_CB = Vector3(0,0,0);
    
    //Calculate physical properties
    if(thick > Scalar(0) && thick/Scalar(2) < mR)
    {
        Scalar mr1 = mR - thick/Scalar(2);
        Scalar mr2 = mR + thick/Scalar(2);
        volume = M_PI*MR*M_PI*(mr2*mr2 - mr1*mr1)*Scalar(2);
        mass = volume * mat.density;
        Scalar m1 = M_PI*MR*M_PI*mr1*mr1;
        Scalar m2 = M_PI*MR*M_PI*mr2*mr2;
        Scalar Id = (Scalar(4)*MR*MR + Scalar(5)*mr2*mr2)*m2/Scalar(8) - (Scalar(4)*MR*MR + Scalar(5)*mr1*mr1)*m1/Scalar(8);
        Scalar Ia = (MR*MR + Scalar(3)/Scalar(4)*mr2*mr2)*m2 - (MR*MR + Scalar(3)/Scalar(4)*mr1*mr1)*m1;
        Ipri = Vector3(Id,Ia,Id);
    }
    else
    {
        volume = M_PI*mR*mR*M_PI*MR*Scalar(2);
        mass = volume * mat.density;
        Scalar Id = (Scalar(4)*MR*MR + Scalar(5)*mR*mR)*mass/Scalar(8);
        Scalar Ia = (MR*MR + Scalar(3)/Scalar(4)*mR*mR)*mass;
        Ipri = Vector3(Id,Ia,Id);
    }
    
    //Build geometry
    phyMesh = OpenGLContent::BuildTorus(MR, mR);
    
    //Compute hydrodynamic properties
    ComputeFluidDynamicsApprox( GeometryApproxType::CYLINDER);
    //dragCoeff = Vector3(0.5, 0.5, 0.5);//Vector3(radius*halfHeight*4.0*0.5, M_PI*radius*radius*0.9, radius*halfHeight*4.0*0.5);
}

SolidType Torus::getSolidType()
{
    return SolidType::TORUS;
}

btCollisionShape* Torus::BuildCollisionShape()
{   
    btCollisionShape* torus = new TorusShape(MR, mR);
    torus->setMargin(COLLISION_MARGIN);
    return torus;
}

}
