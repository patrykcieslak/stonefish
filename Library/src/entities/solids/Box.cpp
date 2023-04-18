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
//  Box.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013-2023 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Box.h"

#include "graphics/OpenGLContent.h"

namespace sf
{

Box::Box(std::string uniqueName, BodyPhysicsSettings phy, const Vector3& dimensions, const Transform& origin, std::string material, std::string look, Scalar thickness, unsigned int uvMode)
         : SolidEntity(uniqueName, phy, material, look, thickness)
{
    halfExtents = dimensions * Scalar(0.5);
    T_O2G = T_O2C = T_O2H = origin;
    T_CG2O = origin.inverse();
    T_CG2C = T_CG2G = I4();
    P_CB = Vector3(0,0,0);
    
    //Calculate physical properties
    if(thick > Scalar(0) && thick/Scalar(2) < halfExtents.x() && thick/Scalar(2) < halfExtents.y() && thick/Scalar(2) < halfExtents.z())
    {
        Vector3 halfExtents1 = halfExtents - Vector3(thick, thick, thick)/Scalar(2);
        Vector3 halfExtents2 = halfExtents + Vector3(thick, thick, thick)/Scalar(2);
        volume = (halfExtents2.x()*halfExtents2.y()*halfExtents2.z() - halfExtents1.x()*halfExtents1.y()*halfExtents1.z())*Scalar(8);
        mass = volume * mat.density;
        Scalar m1 = halfExtents1.x()*halfExtents1.y()*halfExtents1.z()*Scalar(8)*mat.density;
        Scalar m2 = halfExtents2.x()*halfExtents2.y()*halfExtents2.z()*Scalar(8)*mat.density; 
        Scalar Ix = Scalar(1)/Scalar(12)*m2*((halfExtents2.y()*Scalar(2))*(halfExtents2.y()*Scalar(2))+(halfExtents2.z()*Scalar(2))*(halfExtents2.z()*Scalar(2))) 
                      - Scalar(1)/Scalar(12)*m1*((halfExtents1.y()*Scalar(2))*(halfExtents1.y()*Scalar(2))+(halfExtents1.z()*Scalar(2))*(halfExtents1.z()*Scalar(2))); 
        Scalar Iy = Scalar(1)/Scalar(12)*m2*((halfExtents2.x()*Scalar(2))*(halfExtents2.x()*Scalar(2))+(halfExtents2.z()*Scalar(2))*(halfExtents2.z()*Scalar(2))) 
                      - Scalar(1)/Scalar(12)*m1*((halfExtents1.x()*Scalar(2))*(halfExtents1.x()*Scalar(2))+(halfExtents1.z()*Scalar(2))*(halfExtents1.z()*Scalar(2))); 
        Scalar Iz = Scalar(1)/Scalar(12)*m2*((halfExtents2.x()*Scalar(2))*(halfExtents2.x()*Scalar(2))+(halfExtents2.y()*Scalar(2))*(halfExtents2.y()*Scalar(2))) 
                      - Scalar(1)/Scalar(12)*m1*((halfExtents1.x()*Scalar(2))*(halfExtents1.x()*Scalar(2))+(halfExtents1.y()*Scalar(2))*(halfExtents1.y()*Scalar(2))); 
        Ipri = Vector3(Ix,Iy,Iz);
    }
    else
    {
        volume = halfExtents.x()*halfExtents.y()*halfExtents.z()*Scalar(8);
        mass = volume * mat.density;
        Ipri = Vector3(Scalar(1)/Scalar(12)*mass*((halfExtents.y()*Scalar(2))*(halfExtents.y()*Scalar(2))+(halfExtents.z()*Scalar(2))*(halfExtents.z()*Scalar(2))),
                        Scalar(1)/Scalar(12)*mass*((halfExtents.x()*Scalar(2))*(halfExtents.x()*Scalar(2))+(halfExtents.z()*Scalar(2))*(halfExtents.z()*Scalar(2))),
                        Scalar(1)/Scalar(12)*mass*((halfExtents.x()*Scalar(2))*(halfExtents.x()*Scalar(2))+(halfExtents.y()*Scalar(2))*(halfExtents.y()*Scalar(2))));
    }
    
    //Build geometry
	glm::vec3 glHalfExtents(halfExtents.x(), halfExtents.y(), halfExtents.z());
	phyMesh = OpenGLContent::BuildBox(glHalfExtents, 3, uvMode);
    
    //Compute hydrodynamic properties
    ComputeFluidDynamicsApprox( GeometryApproxType::ELLIPSOID);
    //dragCoeff = Vector3(halfExtents.y()*halfExtents.z()*Scalar(4*1.05), halfExtents.x()*halfExtents.z()*Scalar(4*1.05), halfExtents.y()*halfExtents.x()*Scalar(4*1.05));
}

SolidType Box::getSolidType()
{
    return SolidType::BOX;
}

btCollisionShape* Box::BuildCollisionShape()
{
    btCollisionShape* box = new btBoxShape(halfExtents);
    box->setMargin(COLLISION_MARGIN);
    return box;
}

}
