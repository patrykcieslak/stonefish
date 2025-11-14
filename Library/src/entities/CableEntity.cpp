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
//  CableEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/11/25.
//  Copyright(c) 2025 Patryk Cieslak. All rights reserved.
//

#include "entities/CableEntity.h"
#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "entities/SolidEntity.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"
#include "tinysplinecxx.h"
#include "entities/FeatherstoneEntity.h"
#include "entities/forcefields/Ocean.h"

namespace sf
{

CableEntity::CableEntity(std::string uniqueName, CableEnds fixedEnds, Vector3 firstEnd, Vector3 secondEnd, 
    size_t numSegments, Scalar diameter, std::string material, std::string look, float uvScale) : Entity(uniqueName)
{
    SimulationManager* sm = SimulationApp::getApp()->getSimulationManager();

    // Get material
    mat_ = sm->getMaterialManager()->getMaterial(material);
    
    // Create cable soft body
    cableBody_ = btSoftBodyHelpers::CreateRope(sm->getSoftBodyWorldInfo(), firstEnd, secondEnd, numSegments, static_cast<int>(fixedEnds));
    
    // Material properties
    cableBody_->m_materials[0]->m_kLST = 1.0;
    cableBody_->m_materials[0]->m_kAST = 1.0;
    cableBody_->m_materials[0]->m_kVST = 1.0;
    diameter_ = diameter;
    Scalar cableVolume = M_PI * (diameter_ * diameter_) / 4.0 * (firstEnd - secondEnd).length();
    cableBody_->setTotalMass(cableVolume * mat_.density);

    // Solver setup
    cableBody_->m_cfg.piterations = 10; // Position solver iterations
    //cableBody_->m_cfg.viterations = 10; // Velocity solver iterations
    //cableBody_->m_cfg.diterations = 10; // Drift solver iterations
    cableBody_->m_cfg.kDF = 1.0; // Dynamic friction
    cableBody_->m_cfg.kDP = 0.002; // Damping
    cableBody_->m_cfg.kDG = 0.0; // Drag
    cableBody_->m_cfg.kLF = 0.0; // Lift
    cableBody_->m_cfg.kCHR = 1.0; // Rigid contacts hardness
    cableBody_->m_cfg.kKHR = 1.0; // Kinetic contacts hardness
    cableBody_->m_cfg.kSHR = 1.0; // Soft contacts hardness
    cableBody_->m_cfg.kAHR = 1.0; // Anchors hardness
    cableBody_->m_cfg.collisions = btSoftBody::fCollision::SDF_RS;
    
    // Collisions
    cableBody_->setCollisionFlags(cableBody_->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
    cableBody_->getCollisionShape()->setMargin(diameter_ / 2.0);
    cableBody_->getCollisionShape()->setUserPointer(cableBody_);
    cableBody_->setUserPointer(this);
    cableBody_->setActivationState(DISABLE_DEACTIVATION);
    
    // Get Look
    displayMode_ = DisplayMode::GRAPHICAL;
    if(SimulationApp::getApp()->hasGraphics())
    {
        OpenGLContent* content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
        lookId_ = content->getLookId(look);
        uvScale_ = uvScale > 0.f ? uvScale : 1.f;

        size_t numNodes = cableBody_->m_nodes.size();
        numGraphicalNodes_ = btMax(
            static_cast<size_t>(ceil((cableBody_->m_nodes[0].m_x - cableBody_->m_nodes[numNodes - 1].m_x).safeNorm() / Scalar(0.01))),
            numNodes
        ); 
        objectId_ = content->BuildCable(numGraphicalNodes_);
    }
    else
    {
        numGraphicalNodes_ = 0;
        lookId_ = -1;
    }
}

void CableEntity::setDisplayMode(DisplayMode m)
{
    displayMode_ = m;
}

EntityType CableEntity::getType() const
{
    return EntityType::CABLE;
}
void CableEntity::getAABB(Vector3& min, Vector3& max)
{
    if (cableBody_ != nullptr)
        cableBody_->getAabb(min, max);
}

void CableEntity::AttachToSolid(CableEnds ends, SolidEntity* solid)
{
    if (ends == CableEnds::NONE)
        return;

    if (cableBody_ != nullptr && solid != nullptr)
    {
        btRigidBody* rb = solid->getRigidBody();
        if (rb != nullptr)
        {
            if ((ends == CableEnds::FIRST || ends == CableEnds::BOTH) 
                && cableBody_->getMass(0) > Scalar(0)
            )
            {
                cableBody_->appendAnchor(0, rb);
            }
            
            if ((ends == CableEnds::SECOND || ends == CableEnds::BOTH) 
                && cableBody_->getMass(cableBody_->m_nodes.size() - 1) > Scalar(0)
            )
            {
                cableBody_->appendAnchor(cableBody_->m_nodes.size() - 1, rb);
            }
        }
        else 
        {
            btMultiBodyLinkCollider* mbc = solid->getMultiBodyLinkCollider();
            if (mbc != nullptr)
            {
                if ((ends == CableEnds::FIRST || ends == CableEnds::BOTH) 
                    && cableBody_->getMass(0) > Scalar(0)
                )
                {
                    cableBody_->appendDeformableAnchor(0, mbc); // Not working?!
                }
                
                if ((ends == CableEnds::SECOND || ends == CableEnds::BOTH) 
                    && cableBody_->getMass(cableBody_->m_nodes.size() - 1) > Scalar(0)
                )
                {
                    cableBody_->appendDeformableAnchor(cableBody_->m_nodes.size() - 1, mbc); // Not working?!
                }
            }
        }
    }
}

void CableEntity::AddToSimulation(SimulationManager* sm)
{
    if (cableBody_ != nullptr)
        sm->getDynamicsWorld()->addSoftBody(cableBody_);
}

void CableEntity::ComputeHydrodynamicForces(HydrodynamicsSettings settings, Ocean* ocn)
{
    // To be implemented
    printf("ComputeHydrodynamicForces not implemented yet!\n");
}

void CableEntity::ApplyHydrodynamicForces()
{
    // To be implemented
    printf("ApplyHydrodynamicForces not implemented yet!\n");
}

std::vector<Renderable> CableEntity::Render()
{
    std::vector<Renderable> items(0);

    if (cableBody_ != nullptr)
    {
        Renderable item;
        item.type = RenderableType::CABLE;
        item.lookId = displayMode_ == DisplayMode::GRAPHICAL ? lookId_ : -1;
        item.objectId = objectId_;
        item.materialName = mat_.name;
        item.model = glm::mat4(static_cast<GLfloat>(diameter_)/2.f); // Radius is used for drawing
        item.cor = glm::vec3(0.f);
        item.vel = glm::vec3(0.f);
        item.avel = glm::vec3(0.f);
        
        // Build B-spline representation of the cable
        size_t numNodes = cableBody_->m_nodes.size();
        std::vector<Scalar> nodeData(numNodes * 3);

        for (size_t i = 0; i < numNodes; ++i)
        {
            const btSoftBody::Node& node = cableBody_->m_nodes[i];
            nodeData[i * 3 + 0] = node.m_x.getX();
            nodeData[i * 3 + 1] = node.m_x.getY();
            nodeData[i * 3 + 2] = node.m_x.getZ();
        }

        tinyspline::BSpline spline = tinyspline::BSpline::interpolateCubicNatural(nodeData, 3);
        auto knots = spline.uniformKnotSeq(numGraphicalNodes_);
        auto splinePoints = spline.evalAll(knots);

        // Populate cable node data
        item.data = std::make_shared<std::vector<CableNode>>(numGraphicalNodes_);
        std::vector<CableNode>* cableNodes = std::static_pointer_cast<std::vector<CableNode>>(item.data).get();

        for (size_t i = 0; i < numGraphicalNodes_; ++i)
        {
            glm::vec3 p0, p1, p2;
            p1.x = static_cast<GLfloat>(splinePoints[i * 3 + 0]);
            p1.y = static_cast<GLfloat>(splinePoints[i * 3 + 1]);
            p1.z = static_cast<GLfloat>(splinePoints[i * 3 + 2]);

            if (i == 0)
            {
                p2.x = static_cast<GLfloat>(splinePoints[1 * 3 + 0]);
                p2.y = static_cast<GLfloat>(splinePoints[1 * 3 + 1]);
                p2.z = static_cast<GLfloat>(splinePoints[1 * 3 + 2]);
                p0 = p1 - (p2 - p1);
            }
            else if (i == numGraphicalNodes_ - 1)
            {
                p0.x = static_cast<GLfloat>(splinePoints[(i - 1) * 3 + 0]);
                p0.y = static_cast<GLfloat>(splinePoints[(i - 1) * 3 + 1]);
                p0.z = static_cast<GLfloat>(splinePoints[(i - 1) * 3 + 2]);
                p2 = p1 + (p1 - p0);
            }
            else
            {
                p0.x = static_cast<GLfloat>(splinePoints[(i - 1) * 3 + 0]);
                p0.y = static_cast<GLfloat>(splinePoints[(i - 1) * 3 + 1]);
                p0.z = static_cast<GLfloat>(splinePoints[(i - 1) * 3 + 2]);
                p2.x = static_cast<GLfloat>(splinePoints[(i + 1) * 3 + 0]);
                p2.y = static_cast<GLfloat>(splinePoints[(i + 1) * 3 + 1]);
                p2.z = static_cast<GLfloat>(splinePoints[(i + 1) * 3 + 2]);      
            }

            // Tangents
            glm::vec3 t01 = glm::normalize(p1 - p0);
            glm::vec3 t12 = glm::normalize(p2 - p1);

            // Store data
            cableNodes->at(i).posCoord = glm::vec4(p1, static_cast<GLfloat>(knots[i]) * uvScale_);
            cableNodes->at(i).localTangent = glm::normalize(t01 + t12);
        }

        items.push_back(item);
    }

    return items;
}



} // namespace sf