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
//  Sphere.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013-2026 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Sphere.h"

#include "graphics/OpenGLContent.h"

namespace sf
{

Sphere::Sphere(const std::string& uniqueName, PhysicsSettings phy, Scalar radius, const Transform& origin, const std::string& material, const std::string& look, Scalar thickness)
    : SolidEntity(uniqueName, phy, material, look, thickness)
{
    r_ = radius;
    T_O2G_ = T_O2C_ = T_O2H_ = origin;
    T_CG2O_ = origin.inverse();
    T_CG2C_ = T_CG2G_ = I4();
    P_CB_ = Vector3(0,0,0);
    
    //Calculate physical properties
    if(thick_ > Scalar(0) && thick_/Scalar(2) < r_)
    {
        Scalar r1 = r_ - thick_/Scalar(2);
        Scalar r2 = r_ + thick_/Scalar(2);
        volume_ = Scalar(4)/Scalar(3)*M_PI*(r2*r2*r2 - r1*r1*r1);
        surface_ = Scalar(4)*M_PI*r2*r2;
        mass_ = volume_ * mat_.density;
        Scalar I = Scalar(2)/Scalar(5)*mass_*((r2*r2*r2*r2*r2 - r1*r1*r1*r1*r1)/(r2*r2*r2 - r1*r1*r1));
        Ipri_ = Vector3(I,I,I);
    }    
    else
    {
        volume_ = Scalar(4)/Scalar(3)*M_PI*r_*r_*r_;
        surface_ = Scalar(4)*M_PI*r_*r_;
        mass_ = volume_ * mat_.density;
        Scalar I = Scalar(2)/Scalar(5)*mass_*r_*r_;
        Ipri_ = Vector3(I,I,I);
    }
    
    //Build geometry
    phyMesh_ = OpenGLContent::BuildSphere((GLfloat)r_);
    
    //Compute hydrodynamic properties
    ComputeFluidDynamicsApprox( GeometryApproxType::SPHERE);
    //dragCoeff = Vector3(Scalar(0.47)*M_PI*radius*radius, Scalar(0.47)*M_PI*radius*radius, Scalar(0.47)*M_PI*radius*radius);
}

SolidType Sphere::getSolidType() const
{
    return SolidType::SPHERE;
}

std::unique_ptr<btCollisionShape> Sphere::BuildCollisionShape()
{
    return std::make_unique<btSphereShape>(r_); //Note: Entire radius is a collision margin.
}

}
