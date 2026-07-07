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
//  Created by Patryk Cieslak on 29/12/12.
//  Copyright (c) 2012-2024 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Polyhedron.h"

#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "utils/SystemUtil.hpp"
#include "utils/GeometryFileUtil.h"

namespace sf
{

Polyhedron::Polyhedron(std::string uniqueName, PhysicsSettings phy, 
                       std::string graphicsFilename, Scalar graphicsScale, const Transform& graphicsOrigin,
                       std::string physicsFilename, Scalar physicsScale, const Transform& physicsOrigin,
                       std::string material, std::string look, Scalar thickness, GeometryApproxType approx)
                        : SolidEntity(uniqueName, phy, material, look, thickness)
{
    //1.Load geometry from file
    graMesh_ = OpenGLContent::LoadMesh(graphicsFilename, graphicsScale, false);
    T_O2G_ = graphicsOrigin;
    
    if(physicsFilename != "")
    {
        phyMesh_ = OpenGLContent::LoadMesh(physicsFilename, physicsScale, false);
        T_O2C_ = physicsOrigin;
    }
    else
    {
        phyMesh_ = graMesh_;
        T_O2C_ = T_O2G_;
    }
    
    OpenGLContent::Refine(phyMesh_, 3.f);
    
    //2. Compute physical properties
    Vector3 CG;
    Matrix3 Irot;
    ComputePhysicalProperties(phyMesh_, thickness, mat_.density, mass_, CG, volume_, surface_, Ipri_, Irot);
    T_CG2C_.setOrigin(-CG); //Set CG position
    T_CG2C_ = Transform(Irot, Vector3(0,0,0)).inverse() * T_CG2C_; //Align CG frame to principal axes of inertia
    T_CG2O_ = T_CG2C_ * T_O2C_.inverse();
    T_CG2G_ = T_CG2O_ * T_O2G_;

    //3.Calculate equivalent ellipsoid for hydrodynamic force computation
    ComputeFluidDynamicsApprox(approx);
    T_O2H_ = T_CG2O_.inverse() * T_CG2H_;
    P_CB_ = Vector3(0,0,0);
}
    
Polyhedron::Polyhedron(std::string uniqueName, PhysicsSettings phy, 
                       std::string modelFilename, Scalar scale, const Transform& origin,
                       std::string material, std::string look, Scalar thickness, GeometryApproxType approx)
                        : Polyhedron(uniqueName, phy, modelFilename, scale, origin, "", scale, origin, material, look, thickness, approx)
{
}

Polyhedron::~Polyhedron()
{
    if(graMesh_ != nullptr && graMesh_ != phyMesh_)
        delete graMesh_;
}
    
SolidType Polyhedron::getSolidType()
{
    return SolidType::POLYHEDRON;
}

btCollisionShape* Polyhedron::BuildCollisionShape()
{
    btConvexHullShape* convex = new btConvexHullShape();
    for(size_t i=0; i<phyMesh_->getNumOfVertices(); ++i)
    {
        glm::vec3 pos = phyMesh_->getVertexPos(i);
        Vector3 v(pos.x, pos.y, pos.z);
        convex->addPoint(v);
    }
    //convex->optimizeConvexHull();
    convex->setMargin(0);
    return convex;
}

void Polyhedron::BuildGraphicalObject()
{
    if(graMesh_ == NULL || !SimulationApp::getApp()->hasGraphics())
        return;
    
    graObjectId_ = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(graMesh_);
    phyObjectId_ = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(phyMesh_);
}

}
