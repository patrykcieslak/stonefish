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
//  Copyright (c) 2013-2026 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Torus.h"

#include "core/TorusShape.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Torus::Torus(const std::string& uniqueName, PhysicsSettings phy, Scalar majorRadius, Scalar minorRadius, const Transform& origin, const std::string& material, const std::string& look, Scalar thickness)
    : SolidEntity(uniqueName, phy, material, look, thickness)
{
    majorRadius_ = majorRadius;
    minorRadius_ = minorRadius;
    T_O2G_ = T_O2C_ = T_O2H_ = origin;
    T_CG2O_ = origin.inverse();
    T_CG2C_ = T_CG2G_ = I4();
    P_CB_ = Vector3(0,0,0);
    
    //Calculate physical properties
    if(thick_ > Scalar(0) && thick_/Scalar(2) < minorRadius_)
    {
        Scalar mr1 = minorRadius_ - thick_/Scalar(2);
        Scalar mr2 = minorRadius_ + thick_/Scalar(2);
        volume_ = M_PI*majorRadius_*M_PI*(mr2*mr2 - mr1*mr1)*Scalar(2);
        surface_ = Scalar(2)*M_PI*mr2 * Scalar(2)*M_PI*majorRadius_;
        mass_ = volume_ * mat_.density;
        Scalar m1 = M_PI*majorRadius_*M_PI*mr1*mr1;
        Scalar m2 = M_PI*majorRadius_*M_PI*mr2*mr2;
        Scalar Id = (Scalar(4)*majorRadius_*majorRadius_ + Scalar(5)*mr2*mr2)*m2/Scalar(8) - (Scalar(4)*majorRadius_*majorRadius_ + Scalar(5)*mr1*mr1)*m1/Scalar(8);
        Scalar Ia = (majorRadius_*majorRadius_ + Scalar(3)/Scalar(4)*mr2*mr2)*m2 - (majorRadius_*majorRadius_ + Scalar(3)/Scalar(4)*mr1*mr1)*m1;
        Ipri_ = Vector3(Id,Ia,Id);
    }
    else
    {
        volume_ = M_PI*minorRadius_*minorRadius_*M_PI*majorRadius_*Scalar(2);
        surface_ = Scalar(2)*M_PI*minorRadius_ * Scalar(2)*M_PI*majorRadius_;
        mass_ = volume_ * mat_.density;
        Scalar Id = (Scalar(4)*majorRadius_*majorRadius_ + Scalar(5)*minorRadius_*minorRadius_)*mass_/Scalar(8);
        Scalar Ia = (majorRadius_*majorRadius_ + Scalar(3)/Scalar(4)*minorRadius_*minorRadius_)*mass_;
        Ipri_ = Vector3(Id,Ia,Id);
    }
    
    //Build geometry
    phyMesh_ = OpenGLContent::BuildTorus(majorRadius_, minorRadius_);
    
    //Compute hydrodynamic properties
    ComputeFluidDynamicsApprox( GeometryApproxType::CYLINDER);
    //dragCoeff = Vector3(0.5, 0.5, 0.5);//Vector3(radius*halfHeight*4.0*0.5, M_PI*radius*radius*0.9, radius*halfHeight*4.0*0.5);
}

SolidType Torus::getSolidType()
{
    return SolidType::TORUS;
}

std::unique_ptr<btCollisionShape> Torus::BuildCollisionShape()
{   
    std::unique_ptr<btCollisionShape> torus = std::make_unique<TorusShape>(majorRadius_, minorRadius_);
    torus->setMargin(COLLISION_MARGIN);
    return torus;
}

}
