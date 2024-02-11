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
//  Copyright (c) 2013-2019 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Sphere.h"

#include "graphics/OpenGLContent.h"

namespace sf
{

Sphere::Sphere(std::string uniqueName, BodyPhysicsSettings phy, Scalar radius, const Transform& origin, std::string material, std::string look, Scalar thickness)
    : SolidEntity(uniqueName, phy, material, look, thickness)
{
    r = radius;
    T_O2G = T_O2C = T_O2H = origin;
    T_CG2O = origin.inverse();
    T_CG2C = T_CG2G = I4();
    P_CB = Vector3(0,0,0);
    
    //Calculate physical properties
    if(thick > Scalar(0) && thick/Scalar(2) < radius)
    {
        Scalar r1 = r - thick/Scalar(2);
        Scalar r2 = r + thick/Scalar(2);
        volume = Scalar(4)/Scalar(3)*M_PI*(r2*r2*r2 - r1*r1*r1);
        mass = volume * mat.density;
        Scalar I = Scalar(2)/Scalar(5)*mass*((r2*r2*r2*r2*r2 - r1*r1*r1*r1*r1)/(r2*r2*r2 - r1*r1*r1));
        Ipri = Vector3(I,I,I);
    }    
    else
    {
        volume = Scalar(4)/Scalar(3)*M_PI*radius*r*r;
        mass = volume * mat.density;
        Scalar I = Scalar(2)/Scalar(5)*mass*r*r;
        Ipri = Vector3(I,I,I);
    }
    
    //Build geometry
    phyMesh = OpenGLContent::BuildSphere((GLfloat)r);
    
    //Compute hydrodynamic properties
    ComputeFluidDynamicsApprox( GeometryApproxType::SPHERE);
    //dragCoeff = Vector3(Scalar(0.47)*M_PI*radius*radius, Scalar(0.47)*M_PI*radius*radius, Scalar(0.47)*M_PI*radius*radius);
}

SolidType Sphere::getSolidType()
{
    return SolidType::SPHERE;
}

btCollisionShape* Sphere::BuildCollisionShape()
{
    return new btSphereShape(r); //Note: Entire radius is a collision margin.
}

}
