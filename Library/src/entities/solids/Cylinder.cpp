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
//  Cylinder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013-2026 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Cylinder.h"

#include "graphics/OpenGLContent.h"

namespace sf
{

Cylinder::Cylinder(const std::string& uniqueName, PhysicsSettings phy, Scalar radius, Scalar height, const Transform& origin, const std::string& material, const std::string& look, Scalar thickness)
    : SolidEntity(uniqueName, phy, material, look, thickness)
{
    r_ = radius;
    halfHeight_ = height/Scalar(2);
    T_O2G_ = T_O2C_ = T_O2H_ = origin;
    T_CG2O_ = origin.inverse();
    T_CG2C_ = T_CG2G_ = I4();
    P_CB_ = Vector3(0,0,0);
    
    //Calculate physical properties
    if(thick_ > Scalar(0) && thick_/Scalar(2) < r_ && thick_/Scalar(2) < halfHeight_)
    {
        Scalar r1 = r_ - thick_/Scalar(2);
        Scalar r2 = r_ + thick_/Scalar(2);
        Scalar h1 = halfHeight_ - thick_/Scalar(2);
        Scalar h2 = halfHeight_ + thick_/Scalar(2);
        volume_ = M_PI*(r2*r2*h2 - r1*r1*h1)*Scalar(2);
        surface_ = (Scalar(2)*M_PI*r2*h2 + M_PI*r2*r2)*Scalar(2);
        mass_ = volume_ * mat_.density;
        Scalar m1 = M_PI*(r1*r1*h1)*Scalar(2)*mat_.density;
        Scalar m2 = M_PI*(r2*r2*h2)*Scalar(2)*mat_.density;
        Scalar Ia = m2*r2*r2/Scalar(2) - m1*r1*r1/Scalar(2); 
        Scalar Ip = m2*(Scalar(3)*r2*r2 + (h2*Scalar(2))*(h2*Scalar(2)))/Scalar(12) - m1*(Scalar(3)*r1*r1 + (h1*Scalar(2))*(h1*Scalar(2)))/Scalar(12);
        Ipri_ = Vector3(Ip,Ip,Ia);
    }
    else
    {
        volume_ = M_PI*r_*r_*halfHeight_*Scalar(2);
        surface_ = (Scalar(2)*M_PI*r_*halfHeight_ + M_PI*r_*r_)*Scalar(2);
        mass_ = volume_ * mat_.density;
        Scalar Ia = mass_*r_*r_/Scalar(2);
        Scalar Ip = mass_*(Scalar(3)*r_*r_ + (halfHeight_*Scalar(2))*(halfHeight_*Scalar(2)))/Scalar(12);
        Ipri_ = Vector3(Ip,Ip,Ia);
    }
    
    //Build geometry
    phyMesh_ = OpenGLContent::BuildCylinder((GLfloat)r_, (GLfloat)(halfHeight_*2), (unsigned int)btMax(ceil(2.0*M_PI*r_/0.1), 32.0)); //Max 0.1 m cylinder wall slice width
    
    //Compute hydrodynamic properties
    ComputeFluidDynamicsApprox( GeometryApproxType::CYLINDER);
    //dragCoeff = Vector3(radius*halfHeight*Scalar(4*0.5), M_PI*radius*radius*Scalar(0.9), radius*halfHeight*Scalar(4*0.5));
}

SolidType Cylinder::getSolidType() const
{
    return SolidType::CYLINDER;
}

std::unique_ptr<btCollisionShape> Cylinder::BuildCollisionShape()
{
    std::unique_ptr<btCollisionShape> cyl = std::make_unique<btCylinderShapeZ>(Vector3(r_, r_, halfHeight_));
    cyl->setMargin(COLLISION_MARGIN);
    return cyl;
}

}
