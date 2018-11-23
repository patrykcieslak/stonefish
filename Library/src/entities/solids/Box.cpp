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

Box::Box(std::string uniqueName, const btVector3& dimensions, const btTransform& originTrans, Material m, int lookId, btScalar thickness, bool isBuoyant) : SolidEntity(uniqueName, m, lookId, thickness, isBuoyant)
{
    halfExtents = UnitSystem::SetPosition(dimensions * btScalar(0.5));
    localTransform = UnitSystem::SetTransform(originTrans);
    
    //Calculate physical properties
    if(thick > btScalar(0) && thick/btScalar(2) < halfExtents.x() && thick/btScalar(2) < halfExtents.y() && thick/btScalar(2) < halfExtents.z())
    {
        btVector3 halfExtents1 = halfExtents - btVector3(thick, thick, thick)/btScalar(2);
        btVector3 halfExtents2 = halfExtents + btVector3(thick, thick, thick)/btScalar(2);
        volume = (halfExtents2.x()*halfExtents2.y()*halfExtents2.z() - halfExtents1.x()*halfExtents1.y()*halfExtents1.z())*btScalar(8);
        mass = volume * mat.density;
        btScalar m1 = halfExtents1.x()*halfExtents1.y()*halfExtents1.z()*btScalar(8)*mat.density;
        btScalar m2 = halfExtents2.x()*halfExtents2.y()*halfExtents2.z()*btScalar(8)*mat.density; 
        btScalar Ix = btScalar(1)/btScalar(12)*m2*((halfExtents2.y()*btScalar(2))*(halfExtents2.y()*btScalar(2))+(halfExtents2.z()*btScalar(2))*(halfExtents2.z()*btScalar(2))) 
                      - btScalar(1)/btScalar(12)*m1*((halfExtents1.y()*btScalar(2))*(halfExtents1.y()*btScalar(2))+(halfExtents1.z()*btScalar(2))*(halfExtents1.z()*btScalar(2))); 
        btScalar Iy = btScalar(1)/btScalar(12)*m2*((halfExtents2.x()*btScalar(2))*(halfExtents2.x()*btScalar(2))+(halfExtents2.z()*btScalar(2))*(halfExtents2.z()*btScalar(2))) 
                      - btScalar(1)/btScalar(12)*m1*((halfExtents1.x()*btScalar(2))*(halfExtents1.x()*btScalar(2))+(halfExtents1.z()*btScalar(2))*(halfExtents1.z()*btScalar(2))); 
        btScalar Iz = btScalar(1)/btScalar(12)*m2*((halfExtents2.x()*btScalar(2))*(halfExtents2.x()*btScalar(2))+(halfExtents2.y()*btScalar(2))*(halfExtents2.y()*btScalar(2))) 
                      - btScalar(1)/btScalar(12)*m1*((halfExtents1.x()*btScalar(2))*(halfExtents1.x()*btScalar(2))+(halfExtents1.y()*btScalar(2))*(halfExtents1.y()*btScalar(2))); 
        Ipri = btVector3(Ix,Iy,Iz);
    }
    else
    {
        volume = halfExtents.x()*halfExtents.y()*halfExtents.z()*btScalar(8);
        mass = volume * mat.density;
        Ipri = btVector3(btScalar(1)/btScalar(12)*mass*((halfExtents.y()*btScalar(2))*(halfExtents.y()*btScalar(2))+(halfExtents.z()*btScalar(2))*(halfExtents.z()*btScalar(2))),
                        btScalar(1)/btScalar(12)*mass*((halfExtents.x()*btScalar(2))*(halfExtents.x()*btScalar(2))+(halfExtents.z()*btScalar(2))*(halfExtents.z()*btScalar(2))),
                        btScalar(1)/btScalar(12)*mass*((halfExtents.x()*btScalar(2))*(halfExtents.x()*btScalar(2))+(halfExtents.y()*btScalar(2))*(halfExtents.y()*btScalar(2))));
    }
    
    //Build geometry
	glm::vec3 glHalfExtents(halfExtents.x(), halfExtents.y(), halfExtents.z());
	mesh = OpenGLContent::BuildBox(glHalfExtents);
    transformMesh(mesh, localTransform);
    
    //Compute hydrodynamic properties
    ComputeHydrodynamicProxy(HYDRO_PROXY_ELLIPSOID);
    CoB = localTransform.getOrigin();
    //dragCoeff = btVector3(halfExtents.y()*halfExtents.z()*btScalar(4*1.05), halfExtents.x()*halfExtents.z()*btScalar(4*1.05), halfExtents.y()*halfExtents.x()*btScalar(4*1.05));
}

Box::~Box()
{
}

SolidType Box::getSolidType()
{
    return SOLID_BOX;
}

btCollisionShape* Box::BuildCollisionShape()
{
    btBoxShape* boxShape = new btBoxShape(halfExtents);
    btCompoundShape* colShape = new btCompoundShape();
    colShape->addChildShape(localTransform, boxShape);
    //colShape->setMargin(0.0);
    return colShape;
}

