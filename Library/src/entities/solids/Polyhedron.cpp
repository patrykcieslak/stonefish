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
//  Polyhedron.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright (c) 2012-2019 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Polyhedron.h"

#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "utils/SystemUtil.hpp"

namespace sf
{

Polyhedron::Polyhedron(std::string uniqueName,
                       std::string graphicsFilename, Scalar graphicsScale, const Transform& graphicsOrigin,
                       std::string physicsFilename, Scalar physicsScale, const Transform& physicsOrigin,
                       Material m, BodyPhysicsType bpt, int lookId, bool smoothGraphicsNormals, Scalar thickness,
                       bool isBuoyant, GeometryApproxType approx)
                        : SolidEntity(uniqueName, m, bpt, lookId, thickness, isBuoyant)
{
    //1.Load geometry from file
    graMesh = OpenGLContent::LoadMesh(graphicsFilename, graphicsScale, smoothGraphicsNormals);
    T_O2G = graphicsOrigin;
    
    if(physicsFilename != "")
    {
        phyMesh = OpenGLContent::LoadMesh(physicsFilename, physicsScale, false);
        T_O2C = physicsOrigin;
    }
    else
    {
        phyMesh = graMesh;
        T_O2C = T_O2G;
    }
    
    //2. Compute physical properties
    Vector3 CG;
    Matrix3 Irot;
    ComputePhysicalProperties(phyMesh, thickness, m, CG, volume, Ipri, Irot);
    mass = volume*mat.density;
    T_CG2C.setOrigin(-CG); //Set CG position
    T_CG2C = Transform(Irot, Vector3(0,0,0)).inverse() * T_CG2C; //Align CG frame to principal axes of inertia
    
    //3.Calculate equivalent ellipsoid for hydrodynamic force computation
    ComputeFluidDynamicsApprox(approx);
    
    //4. Compute missing transformations
    T_CG2O = T_CG2C * T_O2C.inverse();
    T_CG2G = T_CG2O * T_O2G;
    T_O2H = T_CG2O.inverse() * T_CG2H;
    P_CB = Vector3(0,0,0);
}
    
Polyhedron::Polyhedron(std::string uniqueName,
                       std::string modelFilename, Scalar scale, const Transform& origin,
                       Material m, BodyPhysicsType bpt, int lookId, bool smoothNormals, Scalar thickness, bool isBuoyant, GeometryApproxType approx)
    : Polyhedron(uniqueName, modelFilename, scale, origin, "", scale, origin, m, bpt, lookId, smoothNormals, thickness, isBuoyant, approx)
{
}

Polyhedron::~Polyhedron()
{
    if(graMesh != NULL)
    {
        if(graMesh == phyMesh) phyMesh = NULL;
        delete graMesh;
    }
}
    
SolidType Polyhedron::getSolidType()
{
    return SolidType::SOLID_POLYHEDRON;
}

btCollisionShape* Polyhedron::BuildCollisionShape()
{
    btConvexHullShape* convex = new btConvexHullShape();
    for(size_t i=0; i<phyMesh->vertices.size(); ++i)
	{
		Vector3 v(phyMesh->vertices[i].pos.x, phyMesh->vertices[i].pos.y, phyMesh->vertices[i].pos.z);
        convex->addPoint(v);
	}
	convex->optimizeConvexHull();
    return convex;
}

void Polyhedron::BuildGraphicalObject()
{
    if(graMesh == NULL || !SimulationApp::getApp()->hasGraphics())
        return;
    
    graObjectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(graMesh);
}

}
