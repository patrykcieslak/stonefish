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

CableEntity::CableEntity(std::string uniqueName, PhysicsSettings phy, Vector3 firstEnd, Vector3 secondEnd, 
    size_t numSegments, Scalar diameter, std::string material, std::string look, Scalar stretching, float uvScale) : Entity(uniqueName), phy_(phy)
{
    SimulationManager* sm = SimulationApp::getApp()->getSimulationManager();

    // Get material
    mat_ = sm->getMaterialManager()->getMaterial(material);
    
    // Create cable soft body
    numSegments = numSegments < 2 ? 2 : numSegments;
    cableBody_ = btSoftBodyHelpers::CreateRope(sm->getSoftBodyWorldInfo(), firstEnd, secondEnd, numSegments, 0);
    nodalForces_.resize(cableBody_->m_nodes.size());
    restLength_ = (secondEnd - firstEnd).safeNorm();

    // Material properties
    cableBody_->m_materials[0]->m_kLST = lerp(Scalar(1.0), Scalar(0.02), btClamped(stretching, Scalar(0), Scalar(1)));
    cableBody_->m_materials[0]->m_kAST = Scalar(1);
    cableBody_->m_materials[0]->m_kVST = Scalar(1);
    radius_ = diameter / Scalar(2);
    Scalar cableVolume = M_PI * (radius_ * radius_) * (firstEnd - secondEnd).length();
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
    cableBody_->getCollisionShape()->setMargin(radius_);
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

Scalar CableEntity::getRestLength() const
{
    return restLength_;
}

Scalar CableEntity::getLength() const
{
    if (cableBody_ != nullptr)
    {
        Scalar length {0};
        for (size_t i=0; i<cableBody_->m_nodes.size()-1; ++i)
            length += (cableBody_->m_nodes[i+1].m_x - cableBody_->m_nodes[i].m_x).safeNorm();
        return length;
    }
    else
        return Scalar(0);
}

btSoftBody* CableEntity::getSoftBody() const
{
    return cableBody_;
}

void CableEntity::getAABB(Vector3& min, Vector3& max)
{
    if (cableBody_ != nullptr)
        cableBody_->getAabb(min, max);
}

void CableEntity::AttachToWorld(CableEnds ends)
{
    if (cableBody_ != nullptr)
    {
        if (ends == CableEnds::NONE)
        {
            cableBody_->setMass(0, cableBody_->getMass(1));
            cableBody_->setMass(cableBody_->m_nodes.size() - 1, cableBody_->getMass(cableBody_->m_nodes.size() - 2));
            cableBody_->removeAnchor(0);
            cableBody_->removeAnchor(cableBody_->m_nodes.size() - 1);
            return;
        }

        if (ends == CableEnds::FIRST || ends == CableEnds::BOTH)
        {
            cableBody_->setMass(0, Scalar(0));
            cableBody_->removeAnchor(0);
        }
        
        if (ends == CableEnds::SECOND || ends == CableEnds::BOTH)
        {
            cableBody_->setMass(cableBody_->m_nodes.size() - 1, Scalar(0));
            cableBody_->removeAnchor(cableBody_->m_nodes.size() - 1);
        }
    }
}

void CableEntity::AttachToSolid(CableEnds ends, SolidEntity* solid)
{
    if (ends == CableEnds::NONE)
        return;

    if (cableBody_ != nullptr && solid != nullptr)
    {
        btRigidBody* rb = solid->getRigidBody();
        if (rb == nullptr) // Not working for multibody colliders (deformable anchor doesn't work!)
            return;

        if (ends == CableEnds::FIRST || ends == CableEnds::BOTH)
        {
            cableBody_->setMass(0, cableBody_->getMass(1));
            cableBody_->appendAnchor(0, rb);
        }

        if (ends == CableEnds::SECOND || ends == CableEnds::BOTH)
        {
            cableBody_->setMass(cableBody_->m_nodes.size() - 1, cableBody_->getMass(cableBody_->m_nodes.size() - 2));
            cableBody_->appendAnchor(cableBody_->m_nodes.size() - 1, rb);
        }
    }
}

void CableEntity::AddToSimulation(SimulationManager* sm)
{
    if (cableBody_ != nullptr)
    {
        if (phy_.collisions)
        {
            sm->getDynamicsWorld()->addSoftBody(cableBody_, MASK_DYNAMIC, MASK_GHOST | MASK_STATIC | MASK_DYNAMIC | MASK_ANIMATED_COLLIDING);
        }
        else
        {
            cableBody_->setCollisionFlags(cableBody_->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
            sm->getDynamicsWorld()->addSoftBody(cableBody_, MASK_DYNAMIC, MASK_GHOST);
        }
    }
}

void CableEntity::ApplyGravity(const Vector3& g)
{
    if (cableBody_ != nullptr)
        cableBody_->addForce(g * cableBody_->getTotalMass() / static_cast<Scalar>(cableBody_->m_nodes.size()));
}

Scalar CableEntity::circularSegmentArea(Scalar h) const
{
    if (h <= Scalar(0))
        return Scalar(0);
    else if (h >= Scalar(2) * radius_)
        return Scalar(M_PI) * radius_ * radius_;
    else
    {
        if (h > radius_)
        {
            h = Scalar(2) * radius_ - h;
            Scalar theta = Scalar(2) * btAcos((radius_ - h) / radius_);
            return radius_ * radius_ * (Scalar(M_PI) - (theta - btSin(theta)) / Scalar(2));
        }
        else
        {
            Scalar theta = Scalar(2) * btAcos((radius_ - h) / radius_);
            return (radius_ * radius_ / Scalar(2)) * (theta - btSin(theta));
        }
    }
}

void CableEntity::ComputeHydrodynamicForces(HydrodynamicsSettings settings, Ocean* ocn)
{
    if(phy_.mode != PhysicsMode::FLOATING && phy_.mode != PhysicsMode::SUBMERGED) return;

    // Clear forces
    for (size_t i = 0; i < nodalForces_.size(); ++i)
        nodalForces_[i].clearForces();

    for (size_t i = 0; i < cableBody_->m_nodes.size()-1; ++i)
    {
        // Segment
        Vector3 p1 = cableBody_->m_nodes[i].m_x;
        Vector3 p2 = cableBody_->m_nodes[i+1].m_x;
        Scalar d1 = ocn->GetDepth(p1);
        Scalar d2 = ocn->GetDepth(p2);

        // Sort so that p1 is the shallower point (more negative depth)
        if (d1 > d2)
        {
            std::swap(p1, p2);
            std::swap(d1, d2);
        }

        // Segment parameters
        Scalar dz = d2 - d1; // positive (d1 < d2)
        Scalar dxy = Vector3(p2.getX() - p1.getX(), p2.getY() - p1.getY(), 0.0).safeNorm();
        Vector3 segmentVec = p2 - p1;
        Scalar segmentLength = segmentVec.length();
        Scalar alpha = btAtan(dz/dxy); // angle between segment and horizontal plane
        Scalar margin = btFuzzyZero(dxy) ? Scalar(0) : radius_ * btCos(alpha);
        
        // Compute submerged volume
        Scalar submergedV {0};

        if (d2 < -margin) // Segment is out of water
        {
            continue;
        }
        else if (d1 > margin) // Segment is fully submerged
        {
            submergedV = Scalar(M_PI) * radius_ * radius_ * segmentLength; 
        }
        else // Segment is partially submerged
        {
            if (btFuzzyZero(dxy)) // Vertical segment
            {
                Scalar submergedL = btMin(d2, dz);
                submergedV = Scalar(M_PI) * radius_ * radius_ * submergedL;
            }
            else if (btFuzzyZero(dz)) // Horizontal segment
            {
                Scalar submergedA = circularSegmentArea(d1 + radius_);
                submergedV = submergedA * segmentLength;
            }
            else if (btFabs(d1) <= margin && btFabs(d2) <= margin) // Inclined segment, both caps partially submerged
            {
                // Compute submerged area at both ends
                Scalar A1 = circularSegmentArea(d1/btCos(alpha) + radius_);
                Scalar A2 = circularSegmentArea(d2/btCos(alpha) + radius_);
                submergedV = (A1 + A2) / Scalar(2) * segmentLength;
            }
            else if (d1 < -margin && d2 > margin) // Inclined segment, first cap out of water, second cap submerged
            {
                // Compute length of completely submerged segment
                Scalar l1 = (d2 - radius_ * btCos(alpha)) / btSin(alpha);
                
                // Compute length of half-submerged segment
                Scalar l2 = Scalar(2) * radius_ * btCos(alpha);
                
                submergedV = Scalar(M_PI) * radius_ * radius_ * (l1 + l2 / Scalar(2));
            }
            else if (d2 <= margin) // d1 > margin // Inclined segment, first cap out of water, second cap partially submerged
            {
                // Compute length of half-submerged segment
                Scalar a = radius_ + d2 / btCos(alpha);
                Scalar l = a/btSin(alpha);
                submergedV = circularSegmentArea(a) * l / Scalar(2);
            }
            else if (d1 >= -margin) // Inclined segment, first cap partially submerged, second cap submerged
            {
                // Compute length of half-submerged segment
                Scalar a = radius_ - d1 / btCos(alpha);
                Scalar l = a/btSin(alpha);
                submergedV = Scalar(M_PI) * radius_ * radius_ * segmentLength - circularSegmentArea(a) * l / Scalar(2);
            }
        }

        // Apply buoyancy force
        if (settings.reallisticBuoyancy)
        {
            // !!! Here it is an approximation because the buoyancy force should be applied at the buoyancy center and then it will generate torque 
            // which will result in asymmetrical forces on both ends of the segment !!!
            Vector3 buoyancy = -submergedV * ocn->getLiquid().density * SimulationApp::getApp()->getSimulationManager()->getGravity();
            nodalForces_[i].Fb += buoyancy / Scalar(2);
            nodalForces_[i+1].Fb += buoyancy / Scalar(2);
        }

        // Drag
        if (settings.dampingForces && submergedV > Scalar(0))
        {
            Vector3 p = (p1 + p2) / Scalar(2);
            Vector3 v1 = cableBody_->m_nodes[i].m_v;
            Vector3 v2 = cableBody_->m_nodes[i+1].m_v;
            Vector3 v = (v1 + v2) / Scalar(2);
            Vector3 waterV = ocn->GetFluidVelocity(p);
            Vector3 relV = v - waterV;
        
            const Scalar Cd {0.5}; // Form drag coefficient
            const Scalar Cf {0.2}; // Skin friction coefficient
            Scalar f1 = btFabs(segmentVec.dot(relV));
            Scalar f2 = submergedV / (Scalar(M_PI) * radius_ * radius_ * segmentLength);

            // Form drag
            {
                Scalar S = Scalar(2) * radius_ * segmentLength;
                Vector3 Fdq = -(Scalar(1) - f1) * f2 * Scalar(0.5) * ocn->getLiquid().density * Scalar(Cd) * S * relV.length() * relV;
                nodalForces_[i].Fdq += Fdq / Scalar(2);
                nodalForces_[i+1].Fdq += Fdq / Scalar(2);
            }
            // Skin friction
            {
                Scalar S = f2 * (Scalar(2 * M_PI) * radius_ * segmentLength);
                Vector3 Fdf = -S * relV;
                nodalForces_[i].Fdf += Fdf / Scalar(2);
                nodalForces_[i+1].Fdf += Fdf / Scalar(2);
            }
        }
    }

    // Correction for end nodes
    nodalForces_[0].Fb *= Scalar(2);
    nodalForces_[cableBody_->m_nodes.size() - 1].Fb *= Scalar(2);
    nodalForces_[0].Fdq *= Scalar(2);
    nodalForces_[cableBody_->m_nodes.size() - 1].Fdq *= Scalar(2);
    nodalForces_[0].Fdf *= Scalar(2);
    nodalForces_[cableBody_->m_nodes.size() - 1].Fdf *= Scalar(2);
}

void CableEntity::ApplyHydrodynamicForces()
{
    for (size_t i = 0; i < cableBody_->m_nodes.size(); ++i)
    {
        Vector3 totalForce = nodalForces_[i].Fb + nodalForces_[i].Fdf + nodalForces_[i].Fdq;
        cableBody_->addForce(totalForce, i);
    }
}

std::vector<Renderable> CableEntity::Render()
{
    std::vector<Renderable> items(0);

    if (cableBody_ != nullptr)
    {
        // Cable mesh
        Renderable item;
        item.type = RenderableType::CABLE;
        item.lookId = displayMode_ == DisplayMode::GRAPHICAL ? lookId_ : -1;
        item.objectId = objectId_;
        item.materialName = mat_.name;
        item.model = glm::mat4(static_cast<GLfloat>(radius_));
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

        // Hydrodynamic forces
        if(phy_.mode == PhysicsMode::FLOATING || phy_.mode == PhysicsMode::SUBMERGED)
        {

            Renderable itemFb;
            itemFb.type = RenderableType::FORCE_BUOYANCY;
            itemFb.model = glm::mat4(1.f);
            itemFb.data = std::make_shared<std::vector<glm::vec3>>();
            std::vector<glm::vec3>* pointsFb = itemFb.getDataAsPoints();

            Renderable itemFdq;
            itemFdq.type = RenderableType::FORCE_QUADRATIC_DRAG;
            itemFdq.model = glm::mat4(1.f);
            itemFdq.data = std::make_shared<std::vector<glm::vec3>>();
            std::vector<glm::vec3>* pointsFdq = itemFdq.getDataAsPoints();

            Renderable itemFdf;
            itemFdf.type = RenderableType::FORCE_LINEAR_DRAG;
            itemFdf.model = glm::mat4(1.f);
            itemFdf.data = std::make_shared<std::vector<glm::vec3>>();
            std::vector<glm::vec3>* pointsFdf = itemFdf.getDataAsPoints();

            for (size_t i = 0; i < cableBody_->m_nodes.size(); ++i)
            {
                glm::vec3 p = glVectorFromVector(cableBody_->m_nodes[i].m_x);
                pointsFb->push_back(p);
                pointsFb->push_back(p + glVectorFromVector(nodalForces_[i].Fb));
                pointsFdq->push_back(p);
                pointsFdq->push_back(p + glVectorFromVector(nodalForces_[i].Fdq));
                pointsFdf->push_back(p);
                pointsFdf->push_back(p + glVectorFromVector(nodalForces_[i].Fdf));
            }

            items.push_back(itemFb);
            items.push_back(itemFdq);
            items.push_back(itemFdf);
        }
    }

    return items;
}



} // namespace sf