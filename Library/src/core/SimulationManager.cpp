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
//  SimulationManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012-2026 Patryk Cieslak. All rights reserved.
//

#include "core/SimulationManager.h"

#include "BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.h"
#include "BulletDynamics/MLCPSolvers/btDantzigSolver.h"
#include "BulletDynamics/MLCPSolvers/btSolveProjectedGaussSeidel.h"
#include "BulletDynamics/MLCPSolvers/btLemkeSolver.h"
#include "BulletDynamics/MLCPSolvers/btMLCPSolver.h"
#include "BulletDynamics/Featherstone/btMultiBodyMLCPConstraintSolver.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "BulletSoftBody/btDefaultSoftBodySolver.h"
#include "tinyxml2.h"
#include <chrono>
#include <thread>
#include <typeinfo>
#include <omp.h>
#include <algorithm>
#include "core/FilteredCollisionDispatcher.h"
#include "core/GraphicalSimulationApp.h"
#include "core/NameManager.h"
#include "core/MaterialManager.h"
#include "core/Robot.h"
#include "core/NED.h"
#include "graphics/OpenGLState.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLTrackball.h"
#include "graphics/OpenGLDebugDrawer.h"
#include "utils/SystemUtil.hpp"
#include "utils/UnitSystem.h"
#include "utils/RayTest.hpp"
#include "entities/Entity.h"
#include "entities/CableEntity.h"
#include "entities/FeatherstoneEntity.h"
#include "entities/solids/Compound.h"
#include "entities/StaticEntity.h"
#include "entities/AnimatedEntity.h"
#include "entities/ForcefieldEntity.h"
#include "entities/forcefields/Trigger.h"
#include "entities/statics/Plane.h"
#include "joints/Joint.h"
#include "actuators/Actuator.h"
#include "actuators/Light.h"
#include "actuators/SuctionCup.h"
#include "sensors/Sensor.h"
#include "comms/Comm.h"
#include "sensors/Contact.h"
#include "sensors/VisionSensor.h"

extern ContactAddedCallback gContactAddedCallback;
extern ContactProcessedCallback gContactProcessedCallback;
extern ContactDestroyedCallback gContactDestroyedCallback;

namespace sf
{

SimulationManager::SimulationManager(Scalar stepsPerSecond, Solver st, CollisionFilter cft) 
    : perfMon_(PerformanceMonitor(100))
{
    //Initialize simulation world
    realtimeFactor_ = Scalar(1);
    cpuUsage_ = Scalar(0);
    solver_ = st;
    collisionFilter_ = cft;
    jointErp_ = Scalar(0.1);
    jointLimitErp_ = Scalar(0.2);
    linSleepThreshold_ = Scalar(0);
    angSleepThreshold_ = Scalar(0);
    fdCounter_ = 0;
    currentTime_ = 0;
    timeOffset_ = 0;
    simulationTime_ = 0;
    mlcpFallbacks_ = 0;
    callSimulationStepCompleted_ = true;
    sdm_ = DisplayMode::GRAPHICAL;
    simHydroMutex_ = SDL_CreateMutex();
    simSettingsMutex_ = SDL_CreateMutex();
    simInfoMutex_ = SDL_CreateMutex();
    setStepsPerSecond(stepsPerSecond);
    
    //Set IC solver params
    icProblemSolved_ = false;
    setICSolverParams(false);
    simulationFresh_ = false;
    
    //Create managers
    nameManager_ = std::make_unique<NameManager>();
    materialManager_ = std::make_unique<MaterialManager>();
    ned_ = std::make_unique<NED>();
}

SimulationManager::~SimulationManager()
{
    DestroyScenario();
    SDL_DestroyMutex(simSettingsMutex_);
    SDL_DestroyMutex(simInfoMutex_);
    SDL_DestroyMutex(simHydroMutex_);
}

Robot* SimulationManager::AddRobot(std::unique_ptr<Robot> robot, const Transform& worldTransform)
{
    if(robot != nullptr)
    {
        robots_.push_back(std::move(robot));
        robots_.back()->AddToSimulation(this, worldTransform);
        return robots_.back().get();
    }
    else
        return nullptr;
}

Entity* SimulationManager::AddEntity(std::unique_ptr<Entity> ent)
{
    if(ent != nullptr)
    {
        entities_.push_back(std::move(ent));
        entities_.back()->AddToSimulation(this);
        return entities_.back().get();
    }
    else
        return nullptr;
}

StaticEntity* SimulationManager::AddStaticEntity(std::unique_ptr<StaticEntity> ent, const Transform& origin)
{
    if(ent != nullptr)
    {
        entities_.push_back(std::move(ent));

        StaticEntity* staticEnt = static_cast<StaticEntity*>(entities_.back().get());
        staticEnt->AddToSimulation(this, origin);
        return staticEnt;
    }
    else
        return nullptr;
}

AnimatedEntity* SimulationManager::AddAnimatedEntity(std::unique_ptr<AnimatedEntity> ent)
{
    if(ent != nullptr)
    {
        entities_.push_back(std::move(ent));
        
        AnimatedEntity* animated = static_cast<AnimatedEntity*>(entities_.back().get());
        animated->AddToSimulation(this);
        return animated;
    }
    else
        return nullptr;
}

SolidEntity* SimulationManager::AddSolidEntity(std::unique_ptr<SolidEntity> ent, const Transform& origin)
{
    if(ent != nullptr)
    {
        entities_.push_back(std::move(ent));

        SolidEntity* soild = static_cast<SolidEntity*>(entities_.back().get());
        soild->AddToSimulation(this, origin);
        return soild;
    }
    else
        return nullptr;
}

void SimulationManager::RemoveSolidEntity(SolidEntity* ent)
{
    if(ent != nullptr)
    {
        auto it = std::find_if(entities_.begin(), entities_.end(), [ent](const std::unique_ptr<Entity>& e) { return e.get() == ent; });
        if(it != entities_.end() && (*it)->getType() == EntityType::SOLID)
        {
            static_cast<SolidEntity*>(it->get())->RemoveFromSimulation(this);
            entities_.erase(it);
        }
    }
}

void SimulationManager::AddFeatherstoneEntity(std::unique_ptr<FeatherstoneEntity> ent, const Transform& origin)
{
    if(ent != nullptr)
    {
        entities_.push_back(std::move(ent));
        static_cast<FeatherstoneEntity*>(entities_.back().get())->AddToSimulation(this, origin);
    }
}

void SimulationManager::RemoveFeatherstoneEntity(FeatherstoneEntity* ent)
{
    if(ent != nullptr)
    {
        auto it = std::find_if(entities_.begin(), entities_.end(), [ent](const std::unique_ptr<Entity>& e) { return e.get() == ent; });
        if(it != entities_.end() && (*it)->getType() == EntityType::FEATHERSTONE)
        {
            static_cast<FeatherstoneEntity*>(it->get())->RemoveFromSimulation(this);
            entities_.erase(it);
        }
    }
}
    
void SimulationManager::EnableOcean(Scalar waves, Fluid f)
{
    if(ocean_ != nullptr)
        return;
    
    if(f.name == "")
    {
        std::string water = getMaterialManager()->CreateFluid("Water", 1000.0, 1.308e-3, 1.55); 
        f = getMaterialManager()->getFluid(water);
    }
    
    bool hasGraphics = SimulationApp::getApp()->hasGraphics();

    ocean_ = std::make_unique<Ocean>("Ocean", hasGraphics ? waves : 0.0, f);
    ocean_->AddToSimulation(this);
    
    if(hasGraphics)
    {
        ocean_->InitGraphics(simHydroMutex_);
        ocean_->setRenderable(true);
    }
}
    
void SimulationManager::EnableAtmosphere()
{
    if(atmosphere_ != nullptr)
        return;
    
    std::string air = getMaterialManager()->CreateFluid("Air", 1.0, 1e-6, 1.0);
    Fluid f = getMaterialManager()->getFluid(air);
    
    atmosphere_ = std::make_unique<Atmosphere>("Atmosphere", f);
    atmosphere_->AddToSimulation(this);
    
    if(SimulationApp::getApp()->hasGraphics())
    {
        atmosphere_->InitGraphics(((GraphicalSimulationApp*)SimulationApp::getApp())->getRenderSettings());
        atmosphere_->setRenderable(true);
    }
}

void SimulationManager::AddSensor(std::unique_ptr<Sensor> sens)
{
    if(sens != nullptr)
        sensors_.push_back(std::move(sens));
}

void SimulationManager::AddComm(std::unique_ptr<Comm> comm)
{
    if(comm != nullptr)
        comms_.push_back(std::move(comm));
}

void SimulationManager::AddJoint(std::unique_ptr<Joint> jnt)
{
    if(jnt != nullptr)
    {
        joints_.push_back(std::move(jnt));
        joints_.back()->AddToSimulation(this);
    }
}

void SimulationManager::RemoveJoint(Joint* jnt)
{
    if(jnt != nullptr)
    {
        auto it = std::find_if(joints_.begin(), joints_.end(), [jnt](const std::unique_ptr<Joint>& j) { return j.get() == jnt; });
        if(it != joints_.end())
        {
            (*it)->RemoveFromSimulation(this);
            joints_.erase(it);
        }
    }
}

void SimulationManager::AddActuator(std::unique_ptr<Actuator> act)
{
    if(act != nullptr)
        actuators_.push_back(std::move(act));
}

void SimulationManager::AddContact(std::unique_ptr<Contact> cnt)
{
    if(cnt != nullptr)
    {
        contacts_.push_back(std::move(cnt));
        EnableCollision(cnt->getEntityA(), cnt->getEntityB());
    }
}

int SimulationManager::CheckCollision(const Entity *entA, const Entity *entB)
{
    for(size_t i = 0; i < collisions_.size(); ++i)
    {
        if((collisions_[i].A == entA && collisions_[i].B == entB) 
            || (collisions_[i].B == entA && collisions_[i].A == entB))
                return (int)i;
    }
    
    return -1;
}

void SimulationManager::EnableCollision(const Entity* entA, const Entity* entB)
{
    int colId = CheckCollision(entA, entB);
    
    if(collisionFilter_ == CollisionFilter::INCLUSIVE && colId == -1)
    {
        Collision c;
        c.A = const_cast<Entity*>(entA);
        c.B = const_cast<Entity*>(entB);
        collisions_.push_back(c);
    }
    else if(collisionFilter_ == CollisionFilter::EXCLUSIVE && colId > -1)
    {
        collisions_.erase(collisions_.begin() + colId);
    }
}
    
void SimulationManager::DisableCollision(const Entity* entA, const Entity* entB)
{
    int colId = CheckCollision(entA, entB);
    if(collisionFilter_ == CollisionFilter::EXCLUSIVE && colId == -1)
    {
        Collision c;
        c.A = const_cast<Entity*>(entA);
        c.B = const_cast<Entity*>(entB);
        collisions_.push_back(c);
        cInfo("Disabling collisions between '%s' and '%s'.", entA->getName().c_str(), entB->getName().c_str());
    }
    else if(collisionFilter_ == CollisionFilter::INCLUSIVE && colId > -1)
    {
        collisions_.erase(collisions_.begin() + colId);
        cInfo("Disabling collisions between '%s' and '%s'.", entA->getName().c_str(), entB->getName().c_str());
    }
}

Contact* SimulationManager::getContact(Entity* entA, Entity* entB)
{
    for(size_t i = 0; i < contacts_.size(); ++i)
    {
        if(contacts_[i]->getEntityA() == entA)
        {
            if(contacts_[i]->getEntityB() == entB)
                return contacts_[i].get();
        }
        else if(contacts_[i]->getEntityB() == entA)
        {
            if(contacts_[i]->getEntityA() == entB)
                return contacts_[i].get();
        }
    }
    
    return nullptr;
}

Contact* SimulationManager::getContact(unsigned int index)
{
    if(index < contacts_.size())
        return contacts_[index].get();
    else
        return nullptr;
}

Contact* SimulationManager::getContact(const std::string& name)
{
    for(size_t i = 0; i < contacts_.size(); ++i)
        if(contacts_[i]->getName() == name)
            return contacts_[i].get();
    
    return nullptr;
}

CollisionFilter SimulationManager::getCollisionFilter() const
{
    return collisionFilter_;
}

Solver SimulationManager::getSolver() const
{
    return solver_;
}

Robot* SimulationManager::getRobot(unsigned int index)
{
    if(index < robots_.size())
        return robots_[index].get();
    else
        return nullptr;
}

Robot* SimulationManager::getRobot(const std::string& name)
{
    for(size_t i = 0; i < robots_.size(); ++i)
        if(robots_[i]->getName() == name)
            return robots_[i].get();
    
    return nullptr;
}

Entity* SimulationManager::getEntity(unsigned int index)
{
    if(index < entities_.size())
        return entities_[index].get();
    else
        return nullptr;
}

Entity* SimulationManager::getEntity(const std::string& name)
{
    for(size_t i = 0; i < entities_.size(); ++i)
        if(entities_[i]->getName() == name)
            return entities_[i].get();
    
    return nullptr;
}

Joint* SimulationManager::getJoint(unsigned int index)
{
    if(index < joints_.size())
        return joints_[index].get();
    else
        return nullptr;
}

Joint* SimulationManager::getJoint(const std::string& name)
{
    for(size_t i = 0; i < joints_.size(); ++i)
        if(joints_[i]->getName() == name)
            return joints_[i].get();
    
    return nullptr;
}

Actuator* SimulationManager::getActuator(unsigned int index)
{
    if(index < actuators_.size())
        return actuators_[index].get();
    else
        return nullptr;
}

Actuator* SimulationManager::getActuator(const std::string& name)
{
    for(size_t i = 0; i < actuators_.size(); ++i)
        if(actuators_[i]->getName() == name)
            return actuators_[i].get();
    
    return nullptr;
}

Sensor* SimulationManager::getSensor(unsigned int index)
{
    if(index < sensors_.size())
        return sensors_[index].get();
    else
        return nullptr;
}

Sensor* SimulationManager::getSensor(const std::string& name)
{
    for(size_t i = 0; i < sensors_.size(); ++i)
        if(sensors_[i]->getName() == name)
            return sensors_[i].get();
    
    return nullptr;
}

Comm* SimulationManager::getComm(unsigned int index)
{
    if(index < comms_.size())
        return comms_[index].get();
    else
        return nullptr;
}

Comm* SimulationManager::getComm(const std::string& name)
{
    for(size_t i = 0; i < comms_.size(); ++i)
        if(comms_[i]->getName() == name)
            return comms_[i].get();
    
    return nullptr;
}

NED* SimulationManager::getNED()
{
    return ned_.get();
}

Ocean* SimulationManager::getOcean()
{
    return ocean_.get();
}

Atmosphere* SimulationManager::getAtmosphere()
{
    return atmosphere_.get();
}

btSoftMultiBodyDynamicsWorld* SimulationManager::getDynamicsWorld()
{
    return dynamicsWorld_.get();
}

bool SimulationManager::isSimulationFresh() const
{
    return simulationFresh_;
}

Scalar SimulationManager::getSimulationTime(bool applyOffset) const
{
    // Thread safe access to simulation time
    SDL_LockMutex(simInfoMutex_);
    Scalar st = simulationTime_;
    SDL_UnlockMutex(simInfoMutex_);
    
    // Apply time offset in seconds
    if(applyOffset)
        st += timeOffset_/(Scalar)1e6;

    return st;
}

uint64_t SimulationManager::getSimulationClock() const
{
    return (uint64_t)ceil(realtimeFactor_ * (Scalar)GetTimeInMicroseconds());
}

void SimulationManager::SimulationClockSleep(uint64_t us)
{
    uint64_t t = (uint64_t)ceil((Scalar)us/realtimeFactor_);
    std::this_thread::sleep_for(std::chrono::microseconds(t));
}

MaterialManager* SimulationManager::getMaterialManager()
{
    return materialManager_.get();
}

NameManager* SimulationManager::getNameManager()
{
    return nameManager_.get();
}

PerformanceMonitor& SimulationManager::getPerformanceMonitor()
{
    return perfMon_;
}

void SimulationManager::setStepsPerSecond(Scalar steps)
{
    if(sps_ == steps)
        return;
    
    SDL_LockMutex(simSettingsMutex_);
    sps_ = steps;
    ssus_ = (uint64_t)(1000000.0/steps);
    setFluidDynamicsPrescaler((unsigned int)round(sps_/Scalar(50)));
    SDL_UnlockMutex(simSettingsMutex_);
}

void SimulationManager::setFluidDynamicsPrescaler(unsigned int presc)
{
    if(presc == 0)
        fdPrescaler_ = 1;
    else
        fdPrescaler_ = presc;
}

void SimulationManager::setRealtimeFactor(Scalar f)
{
    SDL_LockMutex(simInfoMutex_);
    realtimeFactor_ = f;
    SDL_UnlockMutex(simInfoMutex_);
}

void SimulationManager::setCallSimulationStepCompleted(bool call)
{
    SDL_LockMutex(simSettingsMutex_);
    callSimulationStepCompleted_ = call;
    SDL_UnlockMutex(simSettingsMutex_);
}

bool SimulationManager::getCallSimulationStepCompleted() const
{
    return callSimulationStepCompleted_;
}

Scalar SimulationManager::getStepsPerSecond() const
{
    return sps_;
}

Scalar SimulationManager::getCpuUsage() const
{
    SDL_LockMutex(simInfoMutex_);
    Scalar cpu = cpuUsage_;
    SDL_UnlockMutex(simInfoMutex_);
    return cpu;
}

Scalar SimulationManager::getRealtimeFactor() const
{
    SDL_LockMutex(simInfoMutex_);
    Scalar rf = realtimeFactor_;
    SDL_UnlockMutex(simInfoMutex_);
    return rf;
}

void SimulationManager::getWorldAABB(Vector3& min, Vector3& max)
{
    min.setValue(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
    max.setValue(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
    
    for(unsigned int i = 0; i < entities_.size(); i++)
    {
        Vector3 entAabbMin, entAabbMax;
        entities_[i]->getAABB(entAabbMin, entAabbMax);
        if(entAabbMin.x() < min.x()) min.setX(entAabbMin.x());
        if(entAabbMin.y() < min.y()) min.setY(entAabbMin.y());
        if(entAabbMin.z() < min.z()) min.setZ(entAabbMin.z());
        if(entAabbMax.x() > max.x()) max.setX(entAabbMax.x());
        if(entAabbMax.y() > max.y()) max.setY(entAabbMax.y());
        if(entAabbMax.z() > max.z()) max.setZ(entAabbMax.z());
    }
}

btSoftBodyWorldInfo& SimulationManager::getSoftBodyWorldInfo()
{
    return sbInfo;
}

void SimulationManager::setGravity(Scalar gravityConstant)
{
    g_ = gravityConstant;
}

Vector3 SimulationManager::getGravity() const
{
    return Vector3(0,0,g_);
}

void SimulationManager::setICSolverParams(bool useGravity, Scalar timeStep, unsigned int maxIterations, Scalar maxTime, Scalar linearTolerance, Scalar angularTolerance)
{
    icUseGravity_ = useGravity;
    icTimeStep_ = timeStep > SIMD_EPSILON ? timeStep : Scalar(0.001);
    icMaxIter_ = maxIterations > 0 ? maxIterations : INT_MAX;
    icMaxTime_ = maxTime > SIMD_EPSILON ? maxTime : BT_LARGE_FLOAT;
    icLinTolerance_ = linearTolerance > SIMD_EPSILON ? linearTolerance : Scalar(1e-6);
    icAngTolerance_ = angularTolerance > SIMD_EPSILON ? angularTolerance : Scalar(1e-6);
}

void SimulationManager::setSolverParams(Scalar erp, Scalar stopErp, Scalar erp2, Scalar globalDamping, Scalar globalFriction,
                                            Scalar linearSleepingThreshold, Scalar angularSleepingThreshold)
{
    if(dynamicsWorld_ == nullptr)
        return;

    dynamicsWorld_->getSolverInfo().m_erp = erp;
    dynamicsWorld_->getSolverInfo().m_erp2 = erp2;
    dynamicsWorld_->getSolverInfo().m_damping = globalDamping;
    dynamicsWorld_->getSolverInfo().m_friction = globalFriction;
    
    jointErp_ = erp;
    jointLimitErp_ = stopErp;
    linSleepThreshold_ = linearSleepingThreshold;
    angSleepThreshold_ = angularSleepingThreshold;
}

void SimulationManager::setSolidDisplayMode(DisplayMode m)
{
    if(sdm_ == m) 
        return;
    sdm_ = m;

    for(size_t i=0; i<entities_.size(); ++i)
    {
        if(entities_[i]->getType() == EntityType::STATIC)
            static_cast<StaticEntity*>(entities_[i].get())->setDisplayMode(sdm_);
        else if(entities_[i]->getType() == EntityType::SOLID || entities_[i]->getType() == EntityType::ANIMATED)
            static_cast<MovingEntity*>(entities_[i].get())->setDisplayMode(sdm_);
        else if(entities_[i]->getType() == EntityType::FEATHERSTONE)
            static_cast<FeatherstoneEntity*>(entities_[i].get())->setDisplayMode(sdm_);
        else if(entities_[i]->getType() == EntityType::CABLE)
            static_cast<CableEntity*>(entities_[i].get())->setDisplayMode(sdm_);
    }

    for(size_t i=0; i<actuators_.size(); ++i)
        actuators_[i]->setDisplayMode(sdm_);
}

DisplayMode SimulationManager::getSolidDisplayMode() const
{
    return sdm_;
}
    
bool SimulationManager::isOceanEnabled() const
{
    return ocean_ != nullptr;
}

void SimulationManager::getSleepingThresholds(Scalar& linear, Scalar& angular) const
{
    linear = linSleepThreshold_;
    angular = angSleepThreshold_;
}

void SimulationManager::getJointErp(Scalar& erp, Scalar& stopErp) const
{
    erp = jointErp_;
    stopErp = jointLimitErp_;
}

void SimulationManager::InitializeSolver()
{
    dwBroadphase_ = std::make_unique<btDbvtBroadphase>();
    dwCollisionConfig_ = std::make_unique<btSoftBodyRigidBodyCollisionConfiguration>();
    
    //Choose collision dispatcher
    switch(collisionFilter_)
    {
        case CollisionFilter::INCLUSIVE:
            dwDispatcher_ = std::make_unique<FilteredCollisionDispatcher>(dwCollisionConfig_.get(), true);
            break;

        case CollisionFilter::EXCLUSIVE:
            dwDispatcher_ = std::make_unique<FilteredCollisionDispatcher>(dwCollisionConfig_.get(), false);
            break;
    }
    
    //Choose constraint solver
    if(solver_ == Solver::SI)
    {
        mbSolver_ = std::make_unique<btMultiBodyConstraintSolver>();
    }
    else
    {
        btMLCPSolverInterface* mlcp;
    
        switch(solver_)
        {
            default:
            case Solver::DANTZIG:
                mlcp = new btDantzigSolver();
                break;
            
            case Solver::PGS:
                mlcp = new btSolveProjectedGaussSeidel();
                break;
            
            case Solver::LEMKE:
                mlcp = new btLemkeSolver();
                //((btLemkeSolver*)mlcp)->m_maxLoops = 10000;
                break;
        }
        
        mbSolver_ = std::make_unique<btMultiBodyMLCPConstraintSolver>(mlcp);
    }
    
    sbSolver_ = std::make_unique<btDefaultSoftBodySolver>();

    //Create dynamics world
    dynamicsWorld_ = std::make_unique<btSoftMultiBodyDynamicsWorld>(dwDispatcher_.get(), dwBroadphase_.get(), mbSolver_.get(), dwCollisionConfig_.get(), sbSolver_.get());
    
    //Basic configuration
    dynamicsWorld_->getSolverInfo().m_solverMode = SOLVER_USE_WARMSTARTING | SOLVER_SIMD | SOLVER_USE_2_FRICTION_DIRECTIONS; //SOLVER_RANDMIZE_ORDER | SOLVER_ENABLE_FRICTION_DIRECTION_CACHING;
    dynamicsWorld_->getSolverInfo().m_warmstartingFactor = Scalar(1.);
    dynamicsWorld_->getSolverInfo().m_minimumSolverBatchSize = 256;
    dynamicsWorld_->getSolverInfo().m_timeStep = Scalar(1)/getStepsPerSecond();
	
    //Quality/stability
    dynamicsWorld_->getSolverInfo().m_tau = Scalar(1.);  //mass factor
    dynamicsWorld_->getSolverInfo().m_erp = jointErp_; //non-contact constraint Baumgarte factor //0.25
    dynamicsWorld_->getSolverInfo().m_erp2 = Scalar(10)/getStepsPerSecond(); //contact constraint Baumgarte factor //0.75
    dynamicsWorld_->getSolverInfo().m_frictionERP = Scalar(0.1); //friction constraint Baumgarte factor //0.5
    dynamicsWorld_->getSolverInfo().m_numIterations = 100; //number of constraint iterations //100
    dynamicsWorld_->getSolverInfo().m_sor = Scalar(1.); //not used
    dynamicsWorld_->getSolverInfo().m_maxErrorReduction = Scalar(0.); //not used
    
    //Collision
    dynamicsWorld_->getSolverInfo().m_splitImpulse = true; //avoid adding energy to the system
    dynamicsWorld_->getSolverInfo().m_splitImpulsePenetrationThreshold = Scalar(-COLLISION_MARGIN); //value close to zero needed for accurate friction // -0.001
    dynamicsWorld_->getSolverInfo().m_splitImpulseTurnErp = Scalar(0.1); //rigid body angular velocity Baumgarte factor //1.0
    dynamicsWorld_->getDispatchInfo().m_useContinuous = false;
    dynamicsWorld_->getDispatchInfo().m_allowedCcdPenetration = Scalar(0.0);
    dynamicsWorld_->getDispatchInfo().m_enableSPU = true;
    dynamicsWorld_->setApplySpeculativeContactRestitution(false); //to make it work one needs restitution in the m_restitution field
    dynamicsWorld_->getSolverInfo().m_restitutionVelocityThreshold = Scalar(0.05); //Velocity at which restitution is overwritten with 0 (bodies stick, stop vibrating)
    
    //Special forces
    dynamicsWorld_->getSolverInfo().m_maxGyroscopicForce = Scalar(1e30); //gyroscopic effect
    
    //Unrealistic components
    dynamicsWorld_->getSolverInfo().m_globalCfm = Scalar(0.); //global constraint force mixing factor
    dynamicsWorld_->getSolverInfo().m_frictionCFM = Scalar(0.); //friction constraint force mixing factor
    dynamicsWorld_->getSolverInfo().m_damping = Scalar(0.); //global damping
    dynamicsWorld_->getSolverInfo().m_friction = Scalar(0.); //global friction
    dynamicsWorld_->getSolverInfo().m_restitution = Scalar(0.); // global restitution
    dynamicsWorld_->getSolverInfo().m_singleAxisRollingFrictionThreshold = Scalar(1e30); //single axis rolling velocity threshold
    dynamicsWorld_->getSolverInfo().m_linearSlop = Scalar(0.); //position bias
    
    dynamicsWorld_->getWorldInfo().m_sparsesdf.setDefaultVoxelsz(Scalar(0.25));
    dynamicsWorld_->getWorldInfo().m_sparsesdf.Reset();

    //Override default callbacks
    dynamicsWorld_->setWorldUserInfo(this);
    dynamicsWorld_->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    gContactAddedCallback = SimulationManager::CustomMaterialCombinerCallback; //Compute combined friction and restitution
    //gContactProcessedCallback = SimulationManager::ContactInfoUpdateCallback; //Update user data
    gContactDestroyedCallback = SimulationManager::ContactInfoDestroyCallback; //Clear user data allocated in contact points
    dynamicsWorld_->setSynchronizeAllMotionStates(false);
    
    //Set default params
    g_ = Scalar(9.81);

    sbInfo = dynamicsWorld_->getWorldInfo();
    sbInfo.m_sparsesdf.Initialize();
    sbInfo.m_sparsesdf.setDefaultVoxelsz(Scalar(0.1));
    sbInfo.m_sparsesdf.Reset();
    sbInfo.air_density = 0.0;
    sbInfo.water_density = 0.0;
    sbInfo.water_normal = Vector3(0.0, 0.0, -1.0);
    sbInfo.water_offset = 0.0;
    sbInfo.m_gravity.setValue(0, 0, 0);
        
    //Debugging
    debugDrawer_ = std::make_unique<OpenGLDebugDrawer>();
    dynamicsWorld_->setDebugDrawer(debugDrawer_.get());
}

void SimulationManager::InitializeScenario()
{
    if(SimulationApp::getApp()->hasGraphics())
    {
		OpenGLState::Init();
        static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->CreateTrackball();
        
    }
	EnableAtmosphere();
}

void SimulationManager::RestartScenario()
{
    DestroyScenario();
    InitializeSolver();
    InitializeScenario();
    BuildScenario(); //Defined by specific application
    
    if(SimulationApp::getApp()->hasGraphics())
    {    
        if(isOceanEnabled())
            ocean_->getOpenGLOcean()->AllocateParticles(((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->getView(0));

        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->Finalize();
    }

    simulationFresh_ = true;
}

void SimulationManager::DestroyScenario()
{
    //Destroy dynamics world
    if(dynamicsWorld_ != nullptr)
    {
        //remove objects from dynamic world
        for(int i = dynamicsWorld_->getNumConstraints()-1; i >= 0; i--)
        {
            btTypedConstraint* constraint = dynamicsWorld_->getConstraint(i);
            dynamicsWorld_->removeConstraint(constraint);
            delete constraint;
        }
    
        for(int i = dynamicsWorld_->getNumCollisionObjects()-1; i >= 0; i--)
        {
            btCollisionObject* obj = dynamicsWorld_->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if (body && body->getMotionState())
                delete body->getMotionState();
            dynamicsWorld_->removeCollisionObject(obj);
            delete obj;
        }
    
        dynamicsWorld_.reset();
        mbSolver_.reset();
        sbSolver_.reset();
        dwBroadphase_.reset();
        dwDispatcher_.reset();
        dwCollisionConfig_.reset();
        debugDrawer_.reset();
    }
    
    //Remove simulation manager objects
    robots_.clear();
    entities_.clear();
    joints_.clear();
    contacts_.clear();
    sensors_.clear();
    comms_.clear();
    actuators_.clear();

    ocean_.reset();
    atmosphere_.reset();
    
    nameManager_->ClearNames();
    materialManager_->ClearMaterialsAndFluids();

    if(SimulationApp::getApp() != nullptr && SimulationApp::getApp()->hasGraphics())
        static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DestroyContent();
}

bool SimulationManager::StartSimulation()
{
    simulationFresh_ = false;
    currentTime_ = 0;
    simulationTime_ = 0;
    mlcpFallbacks_ = 0;
    fdCounter_ = 0;
    
    //Solve initial conditions problem
    if(!SolveICProblem())
        return false;
    
    //Reset contacts
    for(unsigned int i = 0; i < contacts_.size(); i++)
        contacts_[i]->ClearHistory();
    
    //Reset sensors
    for(unsigned int i = 0; i < sensors_.size(); i++)
        sensors_[i]->Reset();

    perfMon_.SimulationStarted();
    
    return true;
}

void SimulationManager::ResumeSimulation()
{
    if(!icProblemSolved_)
        StartSimulation();
    else
        currentTime_ = 0;
}

void SimulationManager::StopSimulation()
{
    perfMon_.SimulationFinished();
}

bool SimulationManager::SolveICProblem()
{
    //Solve for joint positions
    icProblemSolved_ = false;
    
    //Should use gravity?
    if(icUseGravity_)
        dynamicsWorld_->setGravity(Vector3(0,0,g_));
    else
        dynamicsWorld_->setGravity(Vector3(0,0,0));
    
    //Set IC callback
    dynamicsWorld_->setInternalTickCallback(SolveICTickCallback, this, true); //Pre-tick
    dynamicsWorld_->setInternalTickCallback(nullptr, this, false); //Post-tick
    
    uint64_t icTime = GetTimeInMicroseconds();
    unsigned int iterations = 0;
    
    do
    {
        if(iterations > icMaxIter_) //Check iterations limit
        {
            cError("IC problem not solved! Reached maximum interation count.");
            return false;
        }
        else if((GetTimeInMicroseconds() - icTime)/(double)1e6 > icMaxTime_) //Check time limit
        {
            cError("IC problem not solved! Reached maximum time.");
            return false;
        }
        
        //Simulate world
        dynamicsWorld_->stepSimulation(icTimeStep_, 1, icTimeStep_);
        iterations++;
    }
    while(!icProblemSolved_);
    
    double solveTime = (GetTimeInMicroseconds() - icTime)/(double)1e6;
    
    //Synchronize body transforms
    dynamicsWorld_->synchronizeMotionStates();
    simulationTime_ = Scalar(0.);

    //Solving time
    cInfo("IC problem solved with %d iterations in %1.6lf s.", iterations, solveTime);
    
    //Set gravity
    dynamicsWorld_->setGravity(Vector3(0,0,g_));
    
    //Set simulation tick
    dynamicsWorld_->setInternalTickCallback(SimulationTickCallback, this, true); //Pre-tick
    dynamicsWorld_->setInternalTickCallback(SimulationPostTickCallback, this, false); //Post-tick
    return true;
}

void SimulationManager::AdvanceSimulation()
{
    //Check if initial conditions solved
    if(!icProblemSolved_)
        return;

    //Calculate eleapsed time
    uint64_t deltaTime;

    if(currentTime_ == 0) //Start of simulation
    {
        deltaTime = 0.0;
        simulationTime_ = 0.0;
        currentTime_ = getSimulationClock();
        timeOffset_ = currentTime_;
        return;
    }

    uint64_t timeInMicroseconds = getSimulationClock(); //Realtime factor included in clock
    deltaTime = timeInMicroseconds - currentTime_; 
    currentTime_ = timeInMicroseconds;

    if(deltaTime < ssus_) //Sleep if clock did not tick one simulation step
    {
        SimulationClockSleep(ssus_ - deltaTime);
        timeInMicroseconds = getSimulationClock();
        deltaTime += timeInMicroseconds - currentTime_;
        currentTime_ = timeInMicroseconds;
    }
    
    StepSimulation((Scalar)deltaTime/Scalar(1000000.0));
    
    SDL_LockMutex(simInfoMutex_);
    Scalar cpuUsageNow = (Scalar)perfMon_.getPhysicsTime()/(Scalar)deltaTime * Scalar(100);
    Scalar filter(0.001);
    cpuUsage_ = filter * cpuUsageNow + (Scalar(1)-filter) * cpuUsage_;   
    SDL_UnlockMutex(simInfoMutex_);
}

void SimulationManager::StepSimulation(Scalar timeStep)
{
    SDL_LockMutex(simSettingsMutex_);
    perfMon_.PhysicsStarted();
    dynamicsWorld_->stepSimulation((Scalar)timeStep, 1000000, (Scalar)ssus_/Scalar(1000000.0));
    perfMon_.PhysicsFinished();
    SDL_UnlockMutex(simSettingsMutex_);

    //Inform about MLCP failures
    if(solver_ != Solver::SI)
    {
        SDL_LockMutex(simInfoMutex_);
        btMultiBodyMLCPConstraintSolver* mlcp = static_cast<btMultiBodyMLCPConstraintSolver*>(mbSolver_.get());
        int numFallbacks = mlcp->getNumFallbacks();
        if(numFallbacks)
        {
            mlcpFallbacks_ += numFallbacks;
            mlcp->setNumFallbacks(0);
#ifdef DEBUG
            cWarning("MLCP solver failed %d times.\n", mlcpFallbacks_);
#endif
        }
        SDL_UnlockMutex(simInfoMutex_);
    }
}

void SimulationManager::SimulationStepCompleted(Scalar timeStep)
{
#ifdef DEBUG
    if(!SimulationApp::getApp()->hasGraphics())
        cInfo("Simulation time: %1.3lf s", getSimulationTime());
#endif	
}

void SimulationManager::UpdateDrawingQueue()
{
    //Build new drawing queue
    OpenGLPipeline* glPipeline = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline();
 
    //Solids, manipulators, systems....
    for(size_t i=0; i<entities_.size(); ++i)
        glPipeline->AddToDrawingQueue(entities_[i]->Render());

    std::pair<Entity*, int> selected = ((GraphicalSimulationApp*)SimulationApp::getApp())->getSelectedEntity();
    if(selected.first != nullptr)
    {
        if(selected.first->getType() == EntityType::SOLID && ((SolidEntity*)selected.first)->getSolidType() == SolidType::COMPOUND)
            glPipeline->AddToSelectedDrawingQueue(((Compound*)selected.first)->Render(selected.second));
        else
            glPipeline->AddToSelectedDrawingQueue(selected.first->Render());
    }

    //Joints
    for(size_t i=0; i<joints_.size(); ++i)
        glPipeline->AddToDrawingQueue(joints_[i]->Render());
        
    //Actuators
    for(size_t i=0; i<actuators_.size(); ++i)
    {
        glPipeline->AddToDrawingQueue(actuators_[i]->Render());
        if(actuators_[i]->getType() == ActuatorType::LIGHT)
            (static_cast<Light*>(actuators_[i].get()))->UpdateTransform();
    }
    
    //Sensors
    for(size_t i=0; i<sensors_.size(); ++i)
    {
        glPipeline->AddToDrawingQueue(sensors_[i]->Render());
        if(sensors_[i]->getType() == SensorType::VISION)
            (static_cast<VisionSensor*>(sensors_[i].get()))->UpdateTransform();
    }
    
    //Comms
    for(size_t i=0; i<comms_.size(); ++i)
        glPipeline->AddToDrawingQueue(comms_[i]->Render());
    
    //Trackball
    static_cast<OpenGLTrackball*>(glPipeline->getContent()->getView(0))->UpdateCenterPos();
    
    //Contacts
    for(size_t i=0; i<contacts_.size(); ++i)
        glPipeline->AddToDrawingQueue(contacts_[i]->Render());
    
    //Ocean currents
    if(ocean_ != nullptr)
        glPipeline->AddToDrawingQueue(ocean_->Render(actuators_));
}

std::pair<Entity*, int>  SimulationManager::PickEntity(Vector3 eye, Vector3 ray)
{
    ray *= Scalar(100000);
    DetailedRayResultCallback rayCallback(eye, eye+ray);
    rayCallback.m_collisionFilterGroup = MASK_DYNAMIC;
    rayCallback.m_collisionFilterMask = MASK_DYNAMIC | MASK_STATIC | MASK_ANIMATED_COLLIDING | MASK_ANIMATED_NONCOLLIDING;
    dynamicsWorld_->rayTest(eye, eye+ray, rayCallback);
                
    if(rayCallback.hasHit())
    {
        Entity* ent = static_cast<Entity*>(rayCallback.m_collisionObject->getUserPointer());
        if (ent != nullptr
            && !(ent->getType() == EntityType::STATIC && static_cast<StaticEntity*>(ent)->getStaticType() == StaticEntityType::PLANE) // Ignore plane entities
        ) 
        {
            return std::make_pair(ent, rayCallback.m_childShapeIndex);
        }
    }
    return std::make_pair(nullptr, -1);
}

void SimulationManager::RenderBulletDebug()
{
    dynamicsWorld_->debugDrawWorld();
    debugDrawer_->Render();
}
 
std::string SimulationManager::CreateMaterial(const std::string& uniqueName, Scalar density, Scalar restitution)
{
    return getMaterialManager()->CreateMaterial(uniqueName, density, restitution);
}

bool SimulationManager::SetMaterialsInteraction(const std::string& firstMaterialName, const std::string& secondMaterialName, Scalar staticFricCoeff, Scalar dynamicFricCoeff)
{
    return getMaterialManager()->SetMaterialsInteraction(firstMaterialName, secondMaterialName, staticFricCoeff, dynamicFricCoeff);
}

std::string SimulationManager::CreateLook(const std::string& name, Color color, float roughness, float metalness, float reflectivity, 
    const std::string& albedoTexturePath, const std::string& normalTexturePath, const std::string& temperatureTexturePath, const std::pair<float, float>& temperatureRange)
{
    if(SimulationApp::getApp()->hasGraphics())
        return ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->CreatePhysicalLook(name, color.rgb, roughness, metalness, reflectivity, 
            albedoTexturePath, normalTexturePath, temperatureTexturePath, glm::vec2(temperatureRange.first, temperatureRange.second));
    else
        return "";
}

bool SimulationManager::CustomMaterialCombinerCallback(btManifoldPoint& cp,	const btCollisionObjectWrapper* colObj0Wrap,int partId0,int index0,const btCollisionObjectWrapper* colObj1Wrap,int partId1,int index1)
{
    //Retrieve entities associated with colliding objects
    Entity* ent0 = (Entity*)colObj0Wrap->getCollisionObject()->getUserPointer();
    Entity* ent1 = (Entity*)colObj1Wrap->getCollisionObject()->getUserPointer();
    
    //Check if entities are real
    if(ent0 == nullptr || ent1 == nullptr)
    {
        cp.m_combinedFriction = Scalar(0.);
        cp.m_combinedRollingFriction = Scalar(0.);
        cp.m_combinedRestitution = Scalar(0.);
        return true;
    }
    
    //Get material and contact velocity information
    MaterialManager* mm = SimulationApp::getApp()->getSimulationManager()->getMaterialManager();
    
    Material mat0;
    Vector3 contactVelocity0;
    Scalar contactAngularVelocity0;
    
    if(ent0->getType() == EntityType::STATIC)
    {
        StaticEntity* sent0 = (StaticEntity*)ent0;
        mat0 = sent0->getMaterial();
        contactVelocity0.setZero();
        contactAngularVelocity0 = Scalar(0);
    }
    else if(ent0->getType() == EntityType::SOLID)
    {
        SolidEntity* sent0 = (SolidEntity*)ent0;
        if(sent0->getSolidType() == SolidType::COMPOUND)
            mat0 = ((Compound*)sent0)->getMaterial(((Compound*)sent0)->getPartId(index0));
        else
            mat0 = sent0->getMaterial();
        //Vector3 localPoint0 = sent0->getTransform().getBasis() * cp.m_localPointA;
        Vector3 localPoint0 = sent0->getCGTransform().inverse() * cp.getPositionWorldOnA();
        contactVelocity0 = sent0->getLinearVelocityInLocalPoint(localPoint0);
        contactAngularVelocity0 = sent0->getAngularVelocity().dot(-cp.m_normalWorldOnB);
    }
    else
    {
        cp.m_combinedFriction = Scalar(0);
        cp.m_combinedRollingFriction = Scalar(0);
        cp.m_combinedRestitution = Scalar(0);
        return true;
    }
    
    Material mat1;
    Vector3 contactVelocity1;
    Scalar contactAngularVelocity1;
    
    if(ent1->getType() == EntityType::STATIC)
    {
        StaticEntity* sent1 = (StaticEntity*)ent1;
        mat1 = sent1->getMaterial();
        contactVelocity1.setZero();
        contactAngularVelocity1 = Scalar(0);
    }
    else if(ent1->getType() == EntityType::SOLID)
    {
        SolidEntity* sent1 = (SolidEntity*)ent1;
        if(sent1->getSolidType() == SolidType::COMPOUND)
            mat1 = ((Compound*)sent1)->getMaterial(((Compound*)sent1)->getPartId(index1));
        else
            mat1 = sent1->getMaterial();
        //Vector3 localPoint1 = sent1->getTransform().getBasis() * cp.m_localPointB;
        Vector3 localPoint1 = sent1->getCGTransform().inverse() * cp.getPositionWorldOnB();
        contactVelocity1 = sent1->getLinearVelocityInLocalPoint(localPoint1);
        contactAngularVelocity1 = sent1->getAngularVelocity().dot(cp.m_normalWorldOnB);
    }
    else
    {
        cp.m_combinedFriction = Scalar(0);
        cp.m_combinedRollingFriction = Scalar(0);
        cp.m_combinedRestitution = Scalar(0);
        return true;
    }

    //Calculate contact forces
    //A. Stribeck friction model
    Vector3 relLocalVel = contactVelocity1 - contactVelocity0;
    Vector3 normalVel = cp.m_normalWorldOnB * cp.m_normalWorldOnB.dot(relLocalVel);
    Vector3 slipVel = relLocalVel - normalVel;
    Scalar sigma = 1000;
    // f = (static - dynamic)/(sigma * v^2 + 1) + dynamic
    Friction f = mm->GetMaterialsInteraction(mat0.name, mat1.name);
    cp.m_combinedFriction = (f.fStatic - f.fDynamic)/(sigma * slipVel.length2() + Scalar(1)) + f.fDynamic;
    
    //Rolling friction not possible to generalize - needs special treatment
    cp.m_combinedRollingFriction = Scalar(0);
    cp.m_combinedSpinningFriction = Scalar(0);
    
    //Save user data
    ContactInfo* cInfo = new ContactInfo();
    cInfo->totalAppliedImpulse = Scalar(0);
    cInfo->slip = slipVel;
    cp.m_userPersistentData = cInfo;
    
    //Damping angular velocity around contact normal (reduce spinning)
    //calculate relative angular velocity
    Scalar relAngularVelocity01 = contactAngularVelocity0 - contactAngularVelocity1;
    Scalar relAngularVelocity10 = contactAngularVelocity1 - contactAngularVelocity0;
    
    //calculate contact normal force and friction torque
    Scalar normalForce = cp.m_appliedImpulse * SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond();
    Scalar T = cp.m_combinedFriction * normalForce * 0.002;

    //apply damping torque
    if(ent0->getType() == EntityType::SOLID && !btFuzzyZero(relAngularVelocity01))
        ((SolidEntity*)ent0)->ApplyTorque(cp.m_normalWorldOnB * relAngularVelocity01/btFabs(relAngularVelocity01) * T);
    
    if(ent1->getType() == EntityType::SOLID && !btFuzzyZero(relAngularVelocity10))
        ((SolidEntity*)ent1)->ApplyTorque(cp.m_normalWorldOnB * relAngularVelocity10/btFabs(relAngularVelocity10) * T);
    
    //Restitution
    cp.m_combinedRestitution = mat0.restitution * mat1.restitution;
    
    //B. Magnetic attraction (only between magnet and ferromagnetic body, no magnet-magnet support)
    if((mat0.magnetic < Scalar(0) && mat1.magnetic > Scalar(0))
        || (mat0.magnetic > Scalar(0) && mat1.magnetic < Scalar(0)))
    {
        Scalar d = btClamped(cp.getDistance(), Scalar(0.0001), BT_LARGE_FLOAT);
        Scalar mag = (btFabs(mat0.magnetic) * btFabs(mat1.magnetic))/(d*d)/Scalar(1e4);
        btClamp(mag, Scalar(0), Scalar(10000)); //Arbitrary limit of 10kN
        Vector3 mForce = cp.m_normalWorldOnB * mag;

        if(ent0->getType() == EntityType::SOLID)
        {
            SolidEntity* sent0 = (SolidEntity*)ent0;
            sent0->ApplyCentralForce(-mForce);
            sent0->ApplyTorque((cp.m_positionWorldOnA - sent0->getCGTransform().getOrigin()).cross(-mForce));
        }
        if(ent1->getType() == EntityType::SOLID)
        {
            SolidEntity* sent1 = (SolidEntity*)ent1;
            sent1->ApplyCentralForce(mForce);
            sent1->ApplyTorque((cp.m_positionWorldOnB - sent1->getCGTransform().getOrigin()).cross(mForce));
        }

        cp.m_combinedRestitution = Scalar(0); //Allows sticking of bodies together
    }
    
    return true;
}

void SimulationManager::SolveICTickCallback(btDynamicsWorld* world, Scalar timeStep)
{
    SimulationManager* simManager = (SimulationManager*)world->getWorldUserInfo();
    btSoftMultiBodyDynamicsWorld* dynamicsWorld = static_cast<btSoftMultiBodyDynamicsWorld*>(world);
    
    //Clear all forces to ensure that no summing occurs
    dynamicsWorld->clearForces(); //Includes clearing of multibody forces!
    
    //Solve for objects settling
    bool objectsSettled = true;
    
    if(simManager->icUseGravity_)
    {
        //Apply gravity to bodies
        for(size_t i = 0; i < simManager->entities_.size(); ++i)
        {
            if(simManager->entities_[i]->getType() == EntityType::SOLID)
            {
                SolidEntity* solid = static_cast<SolidEntity*>(simManager->entities_[i].get());
                solid->ApplyGravity(world->getGravity());
            }
            else if(simManager->entities_[i]->getType() == EntityType::FEATHERSTONE)
            {
                FeatherstoneEntity* feather = static_cast<FeatherstoneEntity*>(simManager->entities_[i].get());
                feather->ApplyGravity(world->getGravity());
            }
            else if(simManager->entities_[i]->getType() == EntityType::CABLE)
            {
                CableEntity* cable = static_cast<CableEntity*>(simManager->entities_[i].get());
                cable->ApplyGravity(world->getGravity());
            }
        }
        
        if(simManager->simulationTime_ < Scalar(0.01)) //Wait for a few cycles to ensure bodies started moving
            objectsSettled = false;
        else
        {
            //Check if objects settled
            for(size_t i = 0; i < simManager->entities_.size(); ++i)
            {
                if(simManager->entities_[i]->getType() == EntityType::SOLID)
                {
                    SolidEntity* solid = static_cast<SolidEntity*>(simManager->entities_[i].get());
                    if(solid->getLinearVelocity().length() > simManager->icLinTolerance_ * Scalar(100.) || solid->getAngularVelocity().length() > simManager->icAngTolerance_ * Scalar(100.))
                    {
                        objectsSettled = false;
                        break;
                    }
                }
                else if(simManager->entities_[i]->getType() == EntityType::FEATHERSTONE)
                {
                    FeatherstoneEntity* multibody = static_cast<FeatherstoneEntity*>(simManager->entities_[i].get());
                    
                    //Check base velocity
                    Vector3 baseLinVel = multibody->getLinkLinearVelocity(0);
                    Vector3 baseAngVel = multibody->getLinkAngularVelocity(0);
                    
                    if(baseLinVel.length() > simManager->icLinTolerance_ * Scalar(100.) || baseAngVel.length() > simManager->icAngTolerance_ * Scalar(100.0))
                    {
                        objectsSettled = false;
                        break;
                    }
                    
                    //Loop through all joints
                    for(size_t h = 0; h < multibody->getNumOfJoints(); ++h)
                    {
                        Scalar jVelocity;
                        btMultibodyLink::eFeatherstoneJointType jType;
                        multibody->getJointVelocity((unsigned int)h, jVelocity, jType);
                        
                        switch(jType)
                        {
                            case btMultibodyLink::eRevolute:
                                if(Vector3(jVelocity,0,0).length() > simManager->icAngTolerance_ * Scalar(100.))
                                    objectsSettled = false;
                                break;
                                
                            case btMultibodyLink::ePrismatic:
                                if(Vector3(jVelocity,0,0).length() > simManager->icLinTolerance_ * Scalar(100.))
                                    objectsSettled = false;
                                break;
                                
                            default:
                                break;
                        }
                        
                        if(!objectsSettled)
                            break;
                    }
                }
                else if(simManager->entities_[i]->getType() == EntityType::CABLE)
                {
                    btSoftBody* cableBody = static_cast<CableEntity*>(simManager->entities_[i].get())->getSoftBody();
                    for (int h = 0; h < cableBody->m_nodes.size(); ++h)
                    {
                        if (cableBody->m_nodes[h].m_v.length() > simManager->icLinTolerance_ * Scalar(100.))
                        {
                            objectsSettled = false;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    //Solve for joint initial conditions
    bool jointsICSolved = true;
    
    for(size_t i = 0; i < simManager->joints_.size(); ++i)
        if(!simManager->joints_[i]->SolvePositionIC(simManager->icLinTolerance_, simManager->icAngTolerance_))
            jointsICSolved = false;

    //Check if everything solved
    if(objectsSettled && jointsICSolved)
        simManager->icProblemSolved_ = true;
    
    //Update time
    simManager->simulationTime_ += timeStep;
}

//Used to apply and accumulate forces
void SimulationManager::SimulationTickCallback(btDynamicsWorld* world, Scalar timeStep)
{
    SimulationManager* simManager = (SimulationManager*)world->getWorldUserInfo();
    btSoftMultiBodyDynamicsWorld* dynamicsWorld = static_cast<btSoftMultiBodyDynamicsWorld*>(world);
        
    //Clear all forces to ensure that no summing occurs
    dynamicsWorld->clearForces(); //Includes clearing of multibody forces!
        
    //loop through all actuators -> apply forces to bodies (free and connected by joints)
    for(size_t i = 0; i < simManager->actuators_.size(); ++i)
        simManager->actuators_[i]->Update(timeStep);
    
    //loop through all joints -> apply damping forces to bodies connected by joints
    for(size_t i = 0; i < simManager->joints_.size(); ++i)
        simManager->joints_[i]->ApplyDamping();
    
    //loop through all entities that may need special actions
    for(size_t i = 0; i < simManager->entities_.size(); ++i)
    {
        Entity* ent = simManager->entities_[i].get();
        
        if(ent->getType() == EntityType::SOLID)
        {
            SolidEntity* solid = static_cast<SolidEntity*>(ent);
            solid->ApplyGravity(dynamicsWorld->getGravity());
        }
        else if(ent->getType() == EntityType::FEATHERSTONE)
        {
            FeatherstoneEntity* multibody = static_cast<FeatherstoneEntity*>(ent);
            multibody->ApplyGravity(dynamicsWorld->getGravity());
            multibody->ApplyDamping();
        }
        else if(ent->getType() == EntityType::CABLE)
        {
            CableEntity* cable = static_cast<CableEntity*>(ent);
            cable->ApplyGravity(dynamicsWorld->getGravity());
        }
        else if(ent->getType() == EntityType::FORCEFIELD)
        {
            ForcefieldEntity* ff = static_cast<ForcefieldEntity*>(ent);
            if(ff->getForcefieldType() == ForcefieldType::TRIGGER)
            {				
                Trigger* trigger = static_cast<Trigger*>(ff);
                trigger->Clear();
                btBroadphasePairArray& pairArray = trigger->getGhost()->getOverlappingPairCache()->getOverlappingPairArray();
                int numPairs = pairArray.size();
                    
                for(int h = 0; h < numPairs; ++h)
                {
                    const btBroadphasePair& pair = pairArray[h];
                    btBroadphasePair* colPair = world->getPairCache()->findPair(pair.m_pProxy0, pair.m_pProxy1);
                    if(!colPair)
                        continue;
                    
                    btCollisionObject* co1 = (btCollisionObject*)colPair->m_pProxy0->m_clientObject;
                    btCollisionObject* co2 = (btCollisionObject*)colPair->m_pProxy1->m_clientObject;
                
                    if(co1 == trigger->getGhost())
                        trigger->Activate(co2);
                    else if(co2 == trigger->getGhost())
                        trigger->Activate(co1);
                }
            }
        }
    }

    //Geometry-based forces
    bool recompute = simManager->fdCounter_ % simManager->fdPrescaler_ == 0;
    ++simManager->fdCounter_;
    
    //Aerodynamic forces
    if(simManager->atmosphere_ != nullptr)
    {
        btBroadphasePairArray& pairArray = simManager->atmosphere_->getGhost()->getOverlappingPairCache()->getOverlappingPairArray();
        int numPairs = pairArray.size();
        
        if(numPairs > 0)
        {
            #pragma omp parallel for schedule(dynamic)
            for(int h=0; h<numPairs; ++h)
            {
                const btBroadphasePair& pair = pairArray[h];
                btBroadphasePair* colPair = world->getPairCache()->findPair(pair.m_pProxy0, pair.m_pProxy1);
                if (!colPair)
                    continue;
                    
                btCollisionObject* co1 = static_cast<btCollisionObject*>(colPair->m_pProxy0->m_clientObject);
                btCollisionObject* co2 = static_cast<btCollisionObject*>(colPair->m_pProxy1->m_clientObject);
                
                if(co1 == simManager->atmosphere_->getGhost())
                    simManager->atmosphere_->ApplyFluidForces(world, co2, recompute);
                else if(co2 == simManager->ocean_->getGhost())
                    simManager->atmosphere_->ApplyFluidForces(world, co1, recompute);
            }
        }
    }
    
    //Hydrodynamic forces
    if(simManager->ocean_ != nullptr)
    {
        if(recompute) SDL_LockMutex(simManager->simHydroMutex_);
        simManager->perfMon_.HydrodynamicsStarted();
        
        btBroadphasePairArray& pairArray = simManager->ocean_->getGhost()->getOverlappingPairCache()->getOverlappingPairArray();
        int numPairs = pairArray.size();
        
        if(numPairs > 0)
        {
            #pragma omp parallel for schedule(dynamic)
            for(int h=0; h<numPairs; ++h)
            {
                const btBroadphasePair& pair = pairArray[h];
                btBroadphasePair* colPair = world->getPairCache()->findPair(pair.m_pProxy0, pair.m_pProxy1);
                if (!colPair)
                    continue;
                    
                btCollisionObject* co1 = static_cast<btCollisionObject*>(colPair->m_pProxy0->m_clientObject);
                btCollisionObject* co2 = static_cast<btCollisionObject*>(colPair->m_pProxy1->m_clientObject);
                
                if(co1 == simManager->ocean_->getGhost())
                    simManager->ocean_->ApplyFluidForces(world, co2, recompute);
                else if(co2 == simManager->ocean_->getGhost())
                    simManager->ocean_->ApplyFluidForces(world, co1, recompute);
            }
        }
        
        simManager->perfMon_.HydrodynamicsFinished();
        if(recompute) SDL_UnlockMutex(simManager->simHydroMutex_);
    }
}

//Used to measure body motions and calculate controls
void SimulationManager::SimulationPostTickCallback(btDynamicsWorld *world, Scalar timeStep)
{
    SimulationManager* simManager = (SimulationManager*)world->getWorldUserInfo();
    
    //Update motion data
    for(size_t i = 0; i < simManager->entities_.size(); ++i)
    {
        Entity* ent = simManager->entities_[i].get();
            
        if(ent->getType() == EntityType::SOLID)
        {
            SolidEntity* solid = static_cast<SolidEntity*>(ent);
            solid->UpdateAcceleration(timeStep);
        }
        else if(ent->getType() == EntityType::FEATHERSTONE)
        {
            FeatherstoneEntity* fe = static_cast<FeatherstoneEntity*>(ent);
            fe->UpdateAcceleration(timeStep);
        }
        else if(ent->getType() == EntityType::ANIMATED)
        {
            AnimatedEntity* anim = static_cast<AnimatedEntity*>(ent);
            anim->Update(timeStep);
        }
    }

    //Special treatment of suction cup actuator
    for(size_t i = 0; i < simManager->actuators_.size(); ++i)
        if(simManager->actuators_[i]->getType() == ActuatorType::SUCTION_CUP)
            (static_cast<SuctionCup*>(simManager->actuators_[i].get()))->Engage(simManager);

    //Loop through all sensors -> update measurements
    for(size_t i = 0; i < simManager->sensors_.size(); ++i)
        simManager->sensors_[i]->Update(timeStep);
        
    //Loop through all comms -> update state and measurements
    for(size_t i = 0; i < simManager->comms_.size(); ++i)
        simManager->comms_[i]->Update(timeStep);
    
    // Loop through all comms again to process messages (there can be a cross-influence between updates)
    for(size_t i = 0; i < simManager->comms_.size(); ++i)
        simManager->comms_[i]->ProcessMessages();
    
    //Loop through contact manifolds -> update contacts
    if(simManager->getContact(0) != nullptr) // If at least one contact is defined
    {
        int numManifolds = world->getDispatcher()->getNumManifolds();
        for(int i=0; i<numManifolds; ++i)
        {
            btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
            btCollisionObject* coA = const_cast<btCollisionObject*>(contactManifold->getBody0());
            btCollisionObject* coB = const_cast<btCollisionObject*>(contactManifold->getBody1());
            Entity* entA = static_cast<Entity*>(coA->getUserPointer());
            Entity* entB = static_cast<Entity*>(coB->getUserPointer());
            Contact* contact = simManager->getContact(entA, entB);
            if(contact != nullptr && contactManifold->getNumContacts() > 0)
                contact->AddContactPoint(contactManifold, contact->getEntityA() != entA, timeStep);        
        }
    }

    //Update simulation time
    simManager->simulationTime_ += timeStep;
    
    //Optional method to update some post simulation data (like ROS messages...)
    if (simManager->getCallSimulationStepCompleted())
    {
        ////cInfo("PostTickCallback %ld", simManager->getSimulationClock());
        simManager->SimulationStepCompleted(timeStep);
    }
}

//Used to save contact information, including contact forces
bool SimulationManager::ContactInfoUpdateCallback(btManifoldPoint& cp, void* body0, void* body1)
{
    ContactInfo* cInfo = (ContactInfo*)cp.m_userPersistentData;
    cInfo->totalAppliedImpulse += cp.m_appliedImpulse;  
    return true;
}

//Used to deallocate memory reserved for contact information structure
bool SimulationManager::ContactInfoDestroyCallback(void* userPersistentData)
{
    delete ((ContactInfo*)userPersistentData);
    return true;
}

}
