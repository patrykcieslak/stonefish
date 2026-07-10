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
//  Copyright (c) 2013-2026 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Box.h"

#include "graphics/OpenGLContent.h"

namespace sf
{

Box::Box(const std::string& uniqueName, PhysicsSettings phy, const Vector3& dimensions, const Transform& origin, const std::string& material, const std::string& look, Scalar thickness, unsigned int uvMode)
         : SolidEntity(uniqueName, phy, material, look, thickness)
{
    halfExtents_ = dimensions * Scalar(0.5);
    T_O2G_ = T_O2C_ = T_O2H_ = origin;
    T_CG2O_ = origin.inverse();
    T_CG2C_ = T_CG2G_ = I4();
    P_CB_ = Vector3(0,0,0);
    
    //Calculate physical properties
    if(thick_ > Scalar(0) && thick_/Scalar(2) < halfExtents_.x() && thick_/Scalar(2) < halfExtents_.y() && thick_/Scalar(2) < halfExtents_.z())
    {
        Vector3 halfExtents1 = halfExtents_ - Vector3(thick_, thick_, thick_)/Scalar(2);
        Vector3 halfExtents2 = halfExtents_ + Vector3(thick_, thick_, thick_)/Scalar(2);
        volume_ = (halfExtents2.x()*halfExtents2.y()*halfExtents2.z() - halfExtents1.x()*halfExtents1.y()*halfExtents1.z())*Scalar(8);
        surface_ = (halfExtents2.x()*halfExtents2.y()+halfExtents2.x()*halfExtents2.z()+halfExtents2.y()*halfExtents2.z())*Scalar(8);
        mass_ = volume_ * mat_.density;
        Scalar m1 = halfExtents1.x()*halfExtents1.y()*halfExtents1.z()*Scalar(8)*mat_.density;
        Scalar m2 = halfExtents2.x()*halfExtents2.y()*halfExtents2.z()*Scalar(8)*mat_.density; 
        Scalar Ix = Scalar(1)/Scalar(12)*m2*((halfExtents2.y()*Scalar(2))*(halfExtents2.y()*Scalar(2))+(halfExtents2.z()*Scalar(2))*(halfExtents2.z()*Scalar(2))) 
                      - Scalar(1)/Scalar(12)*m1*((halfExtents1.y()*Scalar(2))*(halfExtents1.y()*Scalar(2))+(halfExtents1.z()*Scalar(2))*(halfExtents1.z()*Scalar(2))); 
        Scalar Iy = Scalar(1)/Scalar(12)*m2*((halfExtents2.x()*Scalar(2))*(halfExtents2.x()*Scalar(2))+(halfExtents2.z()*Scalar(2))*(halfExtents2.z()*Scalar(2))) 
                      - Scalar(1)/Scalar(12)*m1*((halfExtents1.x()*Scalar(2))*(halfExtents1.x()*Scalar(2))+(halfExtents1.z()*Scalar(2))*(halfExtents1.z()*Scalar(2))); 
        Scalar Iz = Scalar(1)/Scalar(12)*m2*((halfExtents2.x()*Scalar(2))*(halfExtents2.x()*Scalar(2))+(halfExtents2.y()*Scalar(2))*(halfExtents2.y()*Scalar(2))) 
                      - Scalar(1)/Scalar(12)*m1*((halfExtents1.x()*Scalar(2))*(halfExtents1.x()*Scalar(2))+(halfExtents1.y()*Scalar(2))*(halfExtents1.y()*Scalar(2))); 
        Ipri_ = Vector3(Ix,Iy,Iz);
    }
    else
    {
        volume_ = halfExtents_.x()*halfExtents_.y()*halfExtents_.z()*Scalar(8);
        surface_ = (halfExtents_.x()*halfExtents_.y()+halfExtents_.x()*halfExtents_.z()+halfExtents_.y()*halfExtents_.z())*Scalar(8);
        mass_ = volume_ * mat_.density;
        Ipri_ = Vector3(Scalar(1)/Scalar(12)*mass_*((halfExtents_.y()*Scalar(2))*(halfExtents_.y()*Scalar(2))+(halfExtents_.z()*Scalar(2))*(halfExtents_.z()*Scalar(2))),
                        Scalar(1)/Scalar(12)*mass_*((halfExtents_.x()*Scalar(2))*(halfExtents_.x()*Scalar(2))+(halfExtents_.z()*Scalar(2))*(halfExtents_.z()*Scalar(2))),
                        Scalar(1)/Scalar(12)*mass_*((halfExtents_.x()*Scalar(2))*(halfExtents_.x()*Scalar(2))+(halfExtents_.y()*Scalar(2))*(halfExtents_.y()*Scalar(2))));
    }
    
    //Build geometry
	glm::vec3 glHalfExtents(halfExtents_.x(), halfExtents_.y(), halfExtents_.z());
	phyMesh_ = OpenGLContent::BuildBox(glHalfExtents, 3, uvMode);
    
    //Compute hydrodynamic properties
    ComputeFluidDynamicsApprox( GeometryApproxType::ELLIPSOID);
    //dragCoeff = Vector3(halfExtents.y()*halfExtents.z()*Scalar(4*1.05), halfExtents.x()*halfExtents.z()*Scalar(4*1.05), halfExtents.y()*halfExtents.x()*Scalar(4*1.05));
}

SolidType Box::getSolidType()
{
    return SolidType::BOX;
}

std::unique_ptr<btCollisionShape> Box::BuildCollisionShape()
{
    std::unique_ptr<btCollisionShape> box = std::make_unique<btBoxShape>(halfExtents_);
    box->setMargin(COLLISION_MARGIN);
    return box;
}

}
