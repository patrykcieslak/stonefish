//
//  Box.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Box.h"

#include "utils/MathUtil.hpp"

using namespace sf;

Box::Box(std::string uniqueName, const Vector3& dimensions, const Transform& originTrans, Material m, int lookId, Scalar thickness, bool isBuoyant) : SolidEntity(uniqueName, m, lookId, thickness, isBuoyant)
{
    halfExtents = dimensions * Scalar(0.5);
    T_G2CG = originTrans;
    
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
	mesh = OpenGLContent::BuildBox(glHalfExtents);
    transformMesh(mesh, originTrans);
    
    //Compute hydrodynamic properties
    ComputeHydrodynamicProxy(HYDRO_PROXY_CYLINDER);
    CB = T_G2CG.getOrigin();
    //dragCoeff = Vector3(halfExtents.y()*halfExtents.z()*Scalar(4*1.05), halfExtents.x()*halfExtents.z()*Scalar(4*1.05), halfExtents.y()*halfExtents.x()*Scalar(4*1.05));
}

SolidType Box::getSolidType()
{
    return SOLID_BOX;
}

btCollisionShape* Box::BuildCollisionShape()
{
    btCompoundShape* shape = new btCompoundShape();
    btBoxShape* boxShape = new btBoxShape(halfExtents);
    shape->addChildShape(T_G2CG, boxShape);
    return shape;
}

void Box::SetArbitraryPhysicalProperties(Scalar mass, const Vector3& inertia, const Transform& G2CG)
{
    sf::Transform oldG2CG = getG2CGTransform();
    SolidEntity::SetArbitraryPhysicalProperties(mass, inertia, G2CG);
    if(rigidBody != NULL)
    {
        btCompoundShape* colShape = (btCompoundShape*)rigidBody->getCollisionShape();
        colShape->updateChildTransform(0, oldG2CG);
    }
}
