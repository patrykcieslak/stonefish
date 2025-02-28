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
//  Copyright (c) 2012-2023 Patryk Cieslak. All rights reserved.
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
//#include "entities/CableEntity.h"
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
#include "comms/VLC.h"
#include "sensors/Contact.h"
#include "sensors/VisionSensor.h"

extern ContactAddedCallback gContactAddedCallback;
extern ContactProcessedCallback gContactProcessedCallback;
extern ContactDestroyedCallback gContactDestroyedCallback;

namespace sf
{

SimulationManager::SimulationManager(Scalar stepsPerSecond, SolverType st, CollisionFilteringType cft) 
    : perfMon(PerformanceMonitor(100))
{
    //Initialize simulation world
    realtimeFactor = Scalar(1);
    cpuUsage = Scalar(0);
    solver = st;
    collisionFilter = cft;
    jointErp = Scalar(0.1);
    jointLimitErp = Scalar(0.2);
    linSleepThreshold = Scalar(0);
    angSleepThreshold = Scalar(0);
    fdCounter = 0;
    currentTime = 0;
    timeOffset = 0;
    simulationTime = 0;
    mlcpFallbacks = 0;
    dynamicsWorld = nullptr;
    mbSolver = nullptr;
    sbSolver = nullptr;
    dwBroadphase = nullptr;
    dwCollisionConfig = nullptr;
    dwDispatcher = nullptr;
    ocean = nullptr;
    atmosphere = nullptr;
    trackball = nullptr;
    sdm = DisplayMode::GRAPHICAL;
    simHydroMutex = SDL_CreateMutex();
    simSettingsMutex = SDL_CreateMutex();
    simInfoMutex = SDL_CreateMutex();
    setStepsPerSecond(stepsPerSecond);
    
    //Set IC solver params
    icProblemSolved = false;
    setICSolverParams(false);
    simulationFresh = false;
    
    //Create managers
    nameManager = new NameManager();
    materialManager = new MaterialManager();
    ned = new NED();
}

SimulationManager::~SimulationManager()
{
    DestroyScenario();
    if(atmosphere != nullptr) delete atmosphere;
    SDL_DestroyMutex(simSettingsMutex);
    SDL_DestroyMutex(simInfoMutex);
    SDL_DestroyMutex(simHydroMutex);
    delete materialManager;
    delete nameManager;
    delete ned;
}

void SimulationManager::AddRobot(Robot* robot, const Transform& worldTransform)
{
    if(robot != nullptr)
    {
        robots.push_back(robot);
        robot->AddToSimulation(this, worldTransform);
    }
}

void SimulationManager::AddEntity(Entity *ent)
{
    if(ent != nullptr)
    {
        entities.push_back(ent);
        ent->AddToSimulation(this);
    }
}

void SimulationManager::AddStaticEntity(StaticEntity* ent, const Transform& origin)
{
    if(ent != nullptr)
    {
        entities.push_back(ent);
        ent->AddToSimulation(this, origin);
    }
}

void SimulationManager::AddAnimatedEntity(AnimatedEntity* ent)
{
    if(ent != nullptr)
    {
        entities.push_back(ent);
        ent->AddToSimulation(this);
    }
}

void SimulationManager::AddSolidEntity(SolidEntity* ent, const Transform& origin)
{
    if(ent != nullptr)
    {
        entities.push_back(ent);
        ent->AddToSimulation(this, origin);
    }
}

void SimulationManager::RemoveSolidEntity(SolidEntity* ent)
{
    if(ent != nullptr)
    {
        auto it = std::find(entities.begin(), entities.end(), ent);
        if(it != entities.end() && (*it)->getType() == EntityType::SOLID)
        {
            SolidEntity* solid = static_cast<SolidEntity*>(*it);
            solid->RemoveFromSimulation(this);
            entities.erase(it);
        }
    }
}

void SimulationManager::AddFeatherstoneEntity(FeatherstoneEntity* ent, const Transform& origin)
{
    if(ent != nullptr)
    {
        entities.push_back(ent);
        ent->AddToSimulation(this, origin);
    }
}

void SimulationManager::RemoveFeatherstoneEntity(FeatherstoneEntity* ent)
{
    if(ent != nullptr)
    {
        auto it = std::find(entities.begin(), entities.end(), ent);
        if(it != entities.end() && (*it)->getType() == EntityType::FEATHERSTONE)
        {
            FeatherstoneEntity* fe = static_cast<FeatherstoneEntity*>(*it);
            fe->RemoveFromSimulation(this);
            entities.erase(it);
        }
    }
}
    
void SimulationManager::EnableOcean(Scalar waves, Fluid f)
{
    if(ocean != nullptr)
        return;
    
    if(f.name == "")
    {
        std::string water = getMaterialManager()->CreateFluid("Water", 1000.0, 1.308e-3, 1.55); 
        f = getMaterialManager()->getFluid(water);
    }
    
    bool hasGraphics = SimulationApp::getApp()->hasGraphics();

    ocean = new Ocean("Ocean", hasGraphics ? waves : 0.0, f);
    ocean->AddToSimulation(this);
    
    if(hasGraphics)
    {
        ocean->InitGraphics(simHydroMutex);
        ocean->setRenderable(true);
    }
}
    
void SimulationManager::EnableAtmosphere()
{
    if(atmosphere != nullptr)
        return;
    
    std::string air = getMaterialManager()->CreateFluid("Air", 1.0, 1e-6, 1.0);
    Fluid f = getMaterialManager()->getFluid(air);
    
    atmosphere = new Atmosphere("Atmosphere", f);
    atmosphere->AddToSimulation(this);
    
    if(SimulationApp::getApp()->hasGraphics())
    {
        atmosphere->InitGraphics(((GraphicalSimulationApp*)SimulationApp::getApp())->getRenderSettings());
        atmosphere->setRenderable(true);
    }
}

void SimulationManager::AddSensor(Sensor* sens)
{
    if(sens != nullptr)
        sensors.push_back(sens);
}

void SimulationManager::AddComm(Comm* comm)
{
    if(comm != nullptr)
        comms.push_back(comm);
}

void SimulationManager::AddJoint(Joint* jnt)
{
    if(jnt != nullptr)
    {
        joints.push_back(jnt);
        jnt->AddToSimulation(this);
    }
}

void SimulationManager::RemoveJoint(Joint* jnt)
{
    if(jnt != nullptr)
    {
        auto it = std::find(joints.begin(), joints.end(), jnt);
        if(it != joints.end())
        {
            (*it)->RemoveFromSimulation(this);
            delete *it;
            joints.erase(it);
        }
    }
}

void SimulationManager::AddActuator(Actuator *act)
{
    if(act != nullptr)
        actuators.push_back(act);
}

void SimulationManager::AddContact(Contact* cnt)
{
    if(cnt != nullptr)
    {
        contacts.push_back(cnt);
        EnableCollision(cnt->getEntityA(), cnt->getEntityB());
    }
}

int SimulationManager::CheckCollision(const Entity *entA, const Entity *entB)
{
    for(size_t i = 0; i < collisions.size(); ++i)
    {
        if((collisions[i].A == entA && collisions[i].B == entB) 
            || (collisions[i].B == entA && collisions[i].A == entB))
                return (int)i;
    }
    
    return -1;
}

void SimulationManager::EnableCollision(const Entity* entA, const Entity* entB)
{
    int colId = CheckCollision(entA, entB);
    
    if(collisionFilter == CollisionFilteringType::COLLISION_INCLUSIVE && colId == -1)
    {
        Collision c;
        c.A = const_cast<Entity*>(entA);
        c.B = const_cast<Entity*>(entB);
        collisions.push_back(c);
    }
    else if(collisionFilter == CollisionFilteringType::COLLISION_EXCLUSIVE && colId > -1)
    {
        collisions.erase(collisions.begin() + colId);
    }
}
    
void SimulationManager::DisableCollision(const Entity* entA, const Entity* entB)
{
    int colId = CheckCollision(entA, entB);
    if(collisionFilter == CollisionFilteringType::COLLISION_EXCLUSIVE && colId == -1)
    {
        Collision c;
        c.A = const_cast<Entity*>(entA);
        c.B = const_cast<Entity*>(entB);
        collisions.push_back(c);
        cInfo("Disabling collisions between '%s' and '%s'.", entA->getName().c_str(), entB->getName().c_str());
    }
    else if(collisionFilter == CollisionFilteringType::COLLISION_INCLUSIVE && colId > -1)
    {
        collisions.erase(collisions.begin() + colId);
        cInfo("Disabling collisions between '%s' and '%s'.", entA->getName().c_str(), entB->getName().c_str());
    }
}

Contact* SimulationManager::getContact(Entity* entA, Entity* entB)
{
    for(size_t i = 0; i < contacts.size(); ++i)
    {
        if(contacts[i]->getEntityA() == entA)
        {
            if(contacts[i]->getEntityB() == entB)
                return contacts[i];
        }
        else if(contacts[i]->getEntityB() == entA)
        {
            if(contacts[i]->getEntityA() == entB)
                return contacts[i];
        }
    }
    
    return nullptr;
}

Contact* SimulationManager::getContact(unsigned int index)
{
    if(index < contacts.size())
        return contacts[index];
    else
        return nullptr;
}

Contact* SimulationManager::getContact(const std::string& name)
{
    for(size_t i = 0; i < contacts.size(); ++i)
        if(contacts[i]->getName() == name)
            return contacts[i];
    
    return nullptr;
}

CollisionFilteringType SimulationManager::getCollisionFilter() const
{
    return collisionFilter;
}

SolverType SimulationManager::getSolverType() const
{
    return solver;
}

Robot* SimulationManager::getRobot(unsigned int index)
{
    if(index < robots.size())
        return robots[index];
    else
        return nullptr;
}

Robot* SimulationManager::getRobot(const std::string& name)
{
    for(size_t i = 0; i < robots.size(); ++i)
        if(robots[i]->getName() == name)
            return robots[i];
    
    return nullptr;
}

Entity* SimulationManager::getEntity(unsigned int index)
{
    if(index < entities.size())
        return entities[index];
    else
        return nullptr;
}

Entity* SimulationManager::getEntity(const std::string& name)
{
    for(size_t i = 0; i < entities.size(); ++i)
        if(entities[i]->getName() == name)
            return entities[i];
    
    return nullptr;
}

Joint* SimulationManager::getJoint(unsigned int index)
{
    if(index < joints.size())
        return joints[index];
    else
        return nullptr;
}

Joint* SimulationManager::getJoint(const std::string& name)
{
    for(size_t i = 0; i < joints.size(); ++i)
        if(joints[i]->getName() == name)
            return joints[i];
    
    return nullptr;
}

Actuator* SimulationManager::getActuator(unsigned int index)
{
    if(index < actuators.size())
        return actuators[index];
    else
        return nullptr;
}

Actuator* SimulationManager::getActuator(const std::string& name)
{
    for(size_t i = 0; i < actuators.size(); ++i)
        if(actuators[i]->getName() == name)
            return actuators[i];
    
    return nullptr;
}

Sensor* SimulationManager::getSensor(unsigned int index)
{
    if(index < sensors.size())
        return sensors[index];
    else
        return nullptr;
}

Sensor* SimulationManager::getSensor(const std::string& name)
{
    for(size_t i = 0; i < sensors.size(); ++i)
        if(sensors[i]->getName() == name)
            return sensors[i];
    
    return nullptr;
}

Comm* SimulationManager::getComm(unsigned int index)
{
    if(index < comms.size())
        return comms[index];
    else
        return nullptr;
}

Comm* SimulationManager::getComm(const std::string& name)
{
    for(size_t i = 0; i < comms.size(); ++i)
        if(comms[i]->getName() == name)
            return comms[i];
    
    return nullptr;
}

NED* SimulationManager::getNED()
{
    return ned;
}

Ocean* SimulationManager::getOcean()
{
    return ocean;
}

Atmosphere* SimulationManager::getAtmosphere()
{
    return atmosphere;
}

btSoftMultiBodyDynamicsWorld* SimulationManager::getDynamicsWorld()
{
    return dynamicsWorld;
}

bool SimulationManager::isSimulationFresh() const
{
    return simulationFresh;
}

Scalar SimulationManager::getSimulationTime(bool applyOffset) const
{
    // Thread safe access to simulation time
    SDL_LockMutex(simInfoMutex);
    Scalar st = simulationTime;
    SDL_UnlockMutex(simInfoMutex);
    
    // Apply time offset in seconds
    if(applyOffset)
        st += timeOffset/(Scalar)1e6;

    return st;
}

uint64_t SimulationManager::getSimulationClock() const
{
    return (uint64_t)ceil(realtimeFactor * (Scalar)GetTimeInMicroseconds());
}

void SimulationManager::SimulationClockSleep(uint64_t us)
{
    uint64_t t = (uint64_t)ceil((Scalar)us/realtimeFactor);
    std::this_thread::sleep_for(std::chrono::microseconds(t));
}

MaterialManager* SimulationManager::getMaterialManager()
{
    return materialManager;
}

NameManager* SimulationManager::getNameManager()
{
    return nameManager;
}

PerformanceMonitor& SimulationManager::getPerformanceMonitor()
{
    return perfMon;
}

OpenGLTrackball* SimulationManager::getTrackball()
{
    return trackball;
}

void SimulationManager::setStepsPerSecond(Scalar steps)
{
    if(sps == steps)
        return;
    
    SDL_LockMutex(simSettingsMutex);
    sps = steps;
    ssus = (uint64_t)(1000000.0/steps);
    setFluidDynamicsPrescaler((unsigned int)round(sps/Scalar(50)));
    SDL_UnlockMutex(simSettingsMutex);
}

void SimulationManager::setFluidDynamicsPrescaler(unsigned int presc)
{
    if(presc == 0)
        fdPrescaler = 1;
    else
        fdPrescaler = presc;
}

void SimulationManager::setRealtimeFactor(Scalar f)
{
    SDL_LockMutex(simInfoMutex);
    realtimeFactor = f;
    SDL_UnlockMutex(simInfoMutex);
}

Scalar SimulationManager::getStepsPerSecond() const
{
    return sps;
}

Scalar SimulationManager::getCpuUsage() const
{
    SDL_LockMutex(simInfoMutex);
    Scalar cpu = cpuUsage;
    SDL_UnlockMutex(simInfoMutex);
    return cpu;
}

Scalar SimulationManager::getRealtimeFactor() const
{
    SDL_LockMutex(simInfoMutex);
    Scalar rf = realtimeFactor;
    SDL_UnlockMutex(simInfoMutex);
    return rf;
}

void SimulationManager::getWorldAABB(Vector3& min, Vector3& max)
{
    min.setValue(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
    max.setValue(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
    
    for(unsigned int i = 0; i < entities.size(); i++)
    {
        Vector3 entAabbMin, entAabbMax;
        entities[i]->getAABB(entAabbMin, entAabbMax);
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
    g = gravityConstant;
}

Vector3 SimulationManager::getGravity() const
{
    return Vector3(0,0,g);
}

void SimulationManager::setICSolverParams(bool useGravity, Scalar timeStep, unsigned int maxIterations, Scalar maxTime, Scalar linearTolerance, Scalar angularTolerance)
{
    icUseGravity = useGravity;
    icTimeStep = timeStep > SIMD_EPSILON ? timeStep : Scalar(0.001);
    icMaxIter = maxIterations > 0 ? maxIterations : INT_MAX;
    icMaxTime = maxTime > SIMD_EPSILON ? maxTime : BT_LARGE_FLOAT;
    icLinTolerance = linearTolerance > SIMD_EPSILON ? linearTolerance : Scalar(1e-6);
    icAngTolerance = angularTolerance > SIMD_EPSILON ? angularTolerance : Scalar(1e-6);
}

void SimulationManager::setSolverParams(Scalar erp, Scalar stopErp, Scalar erp2, Scalar globalDamping, Scalar globalFriction,
                                            Scalar linearSleepingThreshold, Scalar angularSleepingThreshold)
{
    if(dynamicsWorld == nullptr)
        return;

    dynamicsWorld->getSolverInfo().m_erp = erp;
    dynamicsWorld->getSolverInfo().m_erp2 = erp2;
    dynamicsWorld->getSolverInfo().m_damping = globalDamping;
    dynamicsWorld->getSolverInfo().m_friction = globalFriction;
    
    jointErp = erp;
    jointLimitErp = stopErp;
    linSleepThreshold = linearSleepingThreshold;
    angSleepThreshold = angularSleepingThreshold;
}

void SimulationManager::setSolidDisplayMode(DisplayMode m)
{
    if(sdm == m) 
        return;
    sdm = m;

    for(size_t i=0; i<entities.size(); ++i)
    {
        if(entities[i]->getType() == EntityType::STATIC)
            ((StaticEntity*)entities[i])->setDisplayMode(sdm);
        else if(entities[i]->getType() == EntityType::SOLID || entities[i]->getType() == EntityType::ANIMATED)
            ((MovingEntity*)entities[i])->setDisplayMode(sdm);
        else if(entities[i]->getType() == EntityType::FEATHERSTONE)
            ((FeatherstoneEntity*)entities[i])->setDisplayMode(sdm);
    }

    for(size_t i=0; i<actuators.size(); ++i)
        actuators[i]->setDisplayMode(sdm);
}

DisplayMode SimulationManager::getSolidDisplayMode() const
{
    return sdm;
}
    
bool SimulationManager::isOceanEnabled() const
{
    return ocean != nullptr;
}

void SimulationManager::getSleepingThresholds(Scalar& linear, Scalar& angular) const
{
    linear = linSleepThreshold;
    angular = angSleepThreshold;
}

void SimulationManager::getJointErp(Scalar& erp, Scalar& stopErp) const
{
    erp = jointErp;
    stopErp = jointLimitErp;
}

void SimulationManager::InitializeSolver()
{
    dwBroadphase = new btDbvtBroadphase(); //btAxisSweep3(Vector3(-50000.0, -50000.0, -10000.0), Vector3(50000.0, 50000.0, 10000.0));
    dwCollisionConfig = new btSoftBodyRigidBodyCollisionConfiguration();

    //Choose collision dispatcher
    switch(collisionFilter)
    {
        case CollisionFilteringType::COLLISION_INCLUSIVE:
            dwDispatcher = new FilteredCollisionDispatcher(dwCollisionConfig, true);
            break;

        case CollisionFilteringType::COLLISION_EXCLUSIVE:
            dwDispatcher = new FilteredCollisionDispatcher(dwCollisionConfig, false);
            break;
    }
    //dwDispatcher = new btCollisionDispatcher(dwCollisionConfig);
    
    //Choose constraint solver
    if(solver == SolverType::SOLVER_SI)
    {
        mbSolver = new btMultiBodyConstraintSolver();
    }
    else
    {
        btMLCPSolverInterface* mlcp;
    
        switch(solver)
        {
            default:
            case SolverType::SOLVER_DANTZIG:
                mlcp = new btDantzigSolver();
                break;
            
            case SolverType::SOLVER_PGS:
                mlcp = new btSolveProjectedGaussSeidel();
                break;
            
            case SolverType::SOLVER_LEMKE:
                mlcp = new btLemkeSolver();
                //((btLemkeSolver*)mlcp)->m_maxLoops = 10000;
                break;
        }
        
        mbSolver = new btMultiBodyMLCPConstraintSolver(mlcp); //ResearchConstraintSolver(mlcp);
    }
    
    sbSolver = new btDefaultSoftBodySolver();

    //Create dynamics world
    dynamicsWorld = new btSoftMultiBodyDynamicsWorld(dwDispatcher, dwBroadphase, mbSolver, dwCollisionConfig, sbSolver);
    
    //Basic configuration
    dynamicsWorld->getSolverInfo().m_solverMode = SOLVER_USE_WARMSTARTING | SOLVER_SIMD | SOLVER_USE_2_FRICTION_DIRECTIONS; //SOLVER_RANDMIZE_ORDER | SOLVER_ENABLE_FRICTION_DIRECTION_CACHING;
    dynamicsWorld->getSolverInfo().m_warmstartingFactor = Scalar(1.);
    dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize = 256;
    dynamicsWorld->getSolverInfo().m_timeStep = Scalar(1)/getStepsPerSecond();
	
    //Quality/stability
    dynamicsWorld->getSolverInfo().m_tau = Scalar(1.);  //mass factor
    dynamicsWorld->getSolverInfo().m_erp = jointErp; //non-contact constraint Baumgarte factor //0.25
    dynamicsWorld->getSolverInfo().m_erp2 = Scalar(10)/getStepsPerSecond(); //contact constraint Baumgarte factor //0.75
    dynamicsWorld->getSolverInfo().m_frictionERP = Scalar(0.1); //friction constraint Baumgarte factor //0.5
    dynamicsWorld->getSolverInfo().m_numIterations = 100; //number of constraint iterations //100
    dynamicsWorld->getSolverInfo().m_sor = Scalar(1.); //not used
    dynamicsWorld->getSolverInfo().m_maxErrorReduction = Scalar(0.); //not used
    
    //Collision
    dynamicsWorld->getSolverInfo().m_splitImpulse = true; //avoid adding energy to the system
    dynamicsWorld->getSolverInfo().m_splitImpulsePenetrationThreshold = Scalar(-COLLISION_MARGIN); //value close to zero needed for accurate friction // -0.001
    dynamicsWorld->getSolverInfo().m_splitImpulseTurnErp = Scalar(0.1); //rigid body angular velocity Baumgarte factor //1.0
    dynamicsWorld->getDispatchInfo().m_useContinuous = false;
    dynamicsWorld->getDispatchInfo().m_allowedCcdPenetration = Scalar(0.0);
    dynamicsWorld->getDispatchInfo().m_enableSPU = true;
    dynamicsWorld->setApplySpeculativeContactRestitution(false); //to make it work one needs restitution in the m_restitution field
    dynamicsWorld->getSolverInfo().m_restitutionVelocityThreshold = Scalar(0.05); //Velocity at which restitution is overwritten with 0 (bodies stick, stop vibrating)
    
    //Special forces
    dynamicsWorld->getSolverInfo().m_maxGyroscopicForce = Scalar(1e30); //gyroscopic effect
    
    //Unrealistic components
    dynamicsWorld->getSolverInfo().m_globalCfm = Scalar(0.); //global constraint force mixing factor
    dynamicsWorld->getSolverInfo().m_frictionCFM = Scalar(0.); //friction constraint force mixing factor
    dynamicsWorld->getSolverInfo().m_damping = Scalar(0.); //global damping
    dynamicsWorld->getSolverInfo().m_friction = Scalar(0.); //global friction
    dynamicsWorld->getSolverInfo().m_restitution = Scalar(0.); // global restitution
    dynamicsWorld->getSolverInfo().m_singleAxisRollingFrictionThreshold = Scalar(1e30); //single axis rolling velocity threshold
    dynamicsWorld->getSolverInfo().m_linearSlop = Scalar(0.); //position bias
    
    //Override default callbacks
    dynamicsWorld->setWorldUserInfo(this);
    dynamicsWorld->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    gContactAddedCallback = SimulationManager::CustomMaterialCombinerCallback; //Compute combined friction and restitution
    //gContactProcessedCallback = SimulationManager::ContactInfoUpdateCallback; //Update user data
    gContactDestroyedCallback = SimulationManager::ContactInfoDestroyCallback; //Clear user data allocated in contact points
    dynamicsWorld->setSynchronizeAllMotionStates(false);
    
    //Set default params
    g = Scalar(9.81);

    sbInfo.m_broadphase = dwBroadphase;
    sbInfo.m_dispatcher = dwDispatcher;
    sbInfo.m_sparsesdf.Initialize();
    sbInfo.m_sparsesdf.Reset(); 
    sbInfo.m_gravity.setValue(0,0,g);
        
    //Debugging
    debugDrawer = new OpenGLDebugDrawer(btIDebugDraw::DBG_DrawWireframe);
    dynamicsWorld->setDebugDrawer(debugDrawer);
}

void SimulationManager::InitializeScenario()
{
    if(SimulationApp::getApp()->hasGraphics())
    {
		OpenGLState::Init();
		
        OpenGLView* view = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->getView(0);
        if(view == nullptr)
        {
            GraphicalSimulationApp* gApp = (GraphicalSimulationApp*)SimulationApp::getApp();
            trackball = new OpenGLTrackball(glm::vec3(0.f,0.f,-1.f), 5.0, glm::vec3(0.f,0.f,-1.f), 0, 0, gApp->getWindowWidth(), gApp->getWindowHeight(), 90.f, glm::vec2(STD_NEAR_PLANE_DISTANCE, STD_FAR_PLANE_DISTANCE));
            trackball->Rotate(glm::quat(glm::eulerAngleYXZ(0.0, 0.0, 0.25)));
            ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(trackball);
        }
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
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->Finalize();

    simulationFresh = true;
}

void SimulationManager::DestroyScenario()
{
    if(dynamicsWorld != nullptr)
    {
        //remove objects from dynamic world
        for(int i = dynamicsWorld->getNumConstraints()-1; i >= 0; i--)
        {
            btTypedConstraint* constraint = dynamicsWorld->getConstraint(i);
            dynamicsWorld->removeConstraint(constraint);
            delete constraint;
        }
    
        for(int i = dynamicsWorld->getNumCollisionObjects()-1; i >= 0; i--)
        {
            btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if (body && body->getMotionState())
                delete body->getMotionState();
            dynamicsWorld->removeCollisionObject(obj);
            delete obj;
        }
    
        delete dynamicsWorld;
        delete mbSolver;
        delete sbSolver;
        delete dwBroadphase;
        delete dwDispatcher;
        delete dwCollisionConfig;
        delete debugDrawer;
    }
    
    //remove sim manager objects
    for(size_t i=0; i<robots.size(); ++i)
        delete robots[i];
    robots.clear();
    
    for(size_t i=0; i<entities.size(); ++i)
        delete entities[i];
    entities.clear();
    
    if(ocean != nullptr)
    {
        delete ocean;
        ocean = nullptr;
    }
    
    if(atmosphere != nullptr)
    {
        delete atmosphere;
        atmosphere = nullptr;
    }
        
    for(size_t i=0; i<joints.size(); ++i)
        delete joints[i];
    joints.clear();
    
    for(size_t i=0; i<contacts.size(); ++i)
        delete contacts[i];
    contacts.clear();
    
    for(size_t i=0; i<sensors.size(); ++i)
        delete sensors[i];
    sensors.clear();
    
    for(size_t i=0; i<comms.size(); ++i)
        delete comms[i];
    comms.clear();
    
    for(size_t i=0; i<actuators.size(); ++i)
        delete actuators[i];
    actuators.clear();
    
    if(nameManager != nullptr)
        nameManager->ClearNames();
        
    if(materialManager != nullptr)
        materialManager->ClearMaterialsAndFluids();

    if(SimulationApp::getApp() != nullptr && SimulationApp::getApp()->hasGraphics())
	{
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DestroyContent();
		trackball = nullptr;
	}
}

bool SimulationManager::StartSimulation()
{
    simulationFresh = false;
    currentTime = 0;
    simulationTime = 0;
    mlcpFallbacks = 0;
    fdCounter = 0;
    
    //Solve initial conditions problem
    if(!SolveICProblem())
        return false;
    
    //Reset contacts
    for(unsigned int i = 0; i < contacts.size(); i++)
        contacts[i]->ClearHistory();
    
    //Reset sensors
    for(unsigned int i = 0; i < sensors.size(); i++)
        sensors[i]->Reset();

    perfMon.SimulationStarted();
    
    return true;
}

void SimulationManager::ResumeSimulation()
{
    if(!icProblemSolved)
        StartSimulation();
    else
        currentTime = 0;
}

void SimulationManager::StopSimulation()
{
    perfMon.SimulationFinished();
}

bool SimulationManager::SolveICProblem()
{
    //Solve for joint positions
    icProblemSolved = false;
    
    //Should use gravity?
    if(icUseGravity)
        dynamicsWorld->setGravity(Vector3(0,0,g));
    else
        dynamicsWorld->setGravity(Vector3(0,0,0));
    
    //Set IC callback
    dynamicsWorld->setInternalTickCallback(SolveICTickCallback, this, true); //Pre-tick
    dynamicsWorld->setInternalTickCallback(nullptr, this, false); //Post-tick
    
    uint64_t icTime = GetTimeInMicroseconds();
    unsigned int iterations = 0;
    
    do
    {
        if(iterations > icMaxIter) //Check iterations limit
        {
            cError("IC problem not solved! Reached maximum interation count.");
            return false;
        }
        else if((GetTimeInMicroseconds() - icTime)/(double)1e6 > icMaxTime) //Check time limit
        {
            cError("IC problem not solved! Reached maximum time.");
            return false;
        }
        
        //Simulate world
        dynamicsWorld->stepSimulation(icTimeStep, 1, icTimeStep);
        iterations++;
    }
    while(!icProblemSolved);
    
    double solveTime = (GetTimeInMicroseconds() - icTime)/(double)1e6;
    
    //Synchronize body transforms
    dynamicsWorld->synchronizeMotionStates();
    simulationTime = Scalar(0.);

    //Solving time
    cInfo("IC problem solved with %d iterations in %1.6lf s.", iterations, solveTime);
    
    //Set gravity
    dynamicsWorld->setGravity(Vector3(0,0,g));
    
    //Set simulation tick
    dynamicsWorld->setInternalTickCallback(SimulationTickCallback, this, true); //Pre-tick
    dynamicsWorld->setInternalTickCallback(SimulationPostTickCallback, this, false); //Post-tick
    return true;
}

void SimulationManager::AdvanceSimulation()
{
    //Check if initial conditions solved
    if(!icProblemSolved)
        return;

    //Calculate eleapsed time
    uint64_t deltaTime;

    if(currentTime == 0) //Start of simulation
    {
        deltaTime = 0.0;
        simulationTime = 0.0;
        currentTime = getSimulationClock();
        timeOffset = currentTime;
        return;
    }

    uint64_t timeInMicroseconds = getSimulationClock(); //Realtime factor included in clock
    deltaTime = timeInMicroseconds - currentTime; 
    currentTime = timeInMicroseconds;

    if(deltaTime < ssus) //Sleep if clock did not tick one simulation step
    {
        SimulationClockSleep(ssus - deltaTime);
        timeInMicroseconds = getSimulationClock();
        deltaTime += timeInMicroseconds - currentTime;
        currentTime = timeInMicroseconds;
    }
    
    //Step simulation
    SDL_LockMutex(simSettingsMutex);
    perfMon.PhysicsStarted();
    dynamicsWorld->stepSimulation((Scalar)deltaTime/Scalar(1000000.0), 1000000, (Scalar)ssus/Scalar(1000000.0));
    perfMon.PhysicsFinished();
    SDL_UnlockMutex(simSettingsMutex);

    SDL_LockMutex(simInfoMutex);
    Scalar cpuUsageNow = (Scalar)perfMon.getPhysicsTime()/(Scalar)deltaTime * Scalar(100);
    Scalar filter(0.001);
    cpuUsage = filter * cpuUsageNow + (Scalar(1)-filter) * cpuUsage;   
    
    //Inform about MLCP failures
    if(solver != SolverType::SOLVER_SI)
    {
        btMultiBodyMLCPConstraintSolver* mlcp = (btMultiBodyMLCPConstraintSolver*)mbSolver;
        int numFallbacks = mlcp->getNumFallbacks();
        if(numFallbacks)
        {
            mlcpFallbacks += numFallbacks;
            mlcp->setNumFallbacks(0);
#ifdef DEBUG
            cWarning("MLCP solver failed %d times.\n", mlcpFallbacks);
#endif
        }
    }
    
    SDL_UnlockMutex(simInfoMutex);
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
    for(size_t i=0; i<entities.size(); ++i)
        glPipeline->AddToDrawingQueue(entities[i]->Render());

    std::pair<Entity*, int> selected = ((GraphicalSimulationApp*)SimulationApp::getApp())->getSelectedEntity();
    if(selected.first != nullptr)
    {
        if(selected.first->getType() == EntityType::SOLID && ((SolidEntity*)selected.first)->getSolidType() == SolidType::COMPOUND)
            glPipeline->AddToSelectedDrawingQueue(((Compound*)selected.first)->Render(selected.second));
        else
            glPipeline->AddToSelectedDrawingQueue(selected.first->Render());
    }

    //Joints
    for(size_t i=0; i<joints.size(); ++i)
        glPipeline->AddToDrawingQueue(joints[i]->Render());
        
    //Actuators
    for(size_t i=0; i<actuators.size(); ++i)
    {
        glPipeline->AddToDrawingQueue(actuators[i]->Render());
        if(actuators[i]->getType() == ActuatorType::LIGHT)
            ((Light*)actuators[i])->UpdateTransform();
    }
    
    //Sensors
    for(size_t i=0; i<sensors.size(); ++i)
    {
        glPipeline->AddToDrawingQueue(sensors[i]->Render());
        if(sensors[i]->getType() == SensorType::VISION)
            ((VisionSensor*)sensors[i])->UpdateTransform();
    }
    
    //Comms
    for(size_t i=0; i<comms.size(); ++i)
        glPipeline->AddToDrawingQueue(comms[i]->Render());
    
    //Trackball
    if(trackball != nullptr)
        trackball->UpdateCenterPos();
    
    //Contacts
    for(size_t i=0; i<contacts.size(); ++i)
        glPipeline->AddToDrawingQueue(contacts[i]->Render());
    
    //Ocean currents
    if(ocean != nullptr)
        glPipeline->AddToDrawingQueue(ocean->Render(actuators));
}

std::pair<Entity*, int>  SimulationManager::PickEntity(Vector3 eye, Vector3 ray)
{
    ray *= Scalar(100000);
    //btCollisionWorld::ClosestRayResultCallback rayCallback(eye, eye+ray);
    DetailedRayResultCallback rayCallback(eye, eye+ray);
    rayCallback.m_collisionFilterGroup = MASK_DYNAMIC;
    rayCallback.m_collisionFilterMask = MASK_DYNAMIC | MASK_STATIC | MASK_ANIMATED_COLLIDING | MASK_ANIMATED_NONCOLLIDING;
    dynamicsWorld->rayTest(eye, eye+ray, rayCallback);
                
    if(rayCallback.hasHit())
    {
        Entity* ent = (Entity*)rayCallback.m_collisionObject->getUserPointer();
        return std::make_pair(ent, rayCallback.m_childShapeIndex);
    }
    else
        return std::make_pair(nullptr, -1);
}

void SimulationManager::RenderBulletDebug()
{
    dynamicsWorld->debugDrawWorld();
    debugDrawer->Render();
}
 
std::string SimulationManager::CreateMaterial(const std::string& uniqueName, Scalar density, Scalar restitution)
{
    return getMaterialManager()->CreateMaterial(uniqueName, density, restitution);
}

bool SimulationManager::SetMaterialsInteraction(const std::string& firstMaterialName, const std::string& secondMaterialName, Scalar staticFricCoeff, Scalar dynamicFricCoeff)
{
    return getMaterialManager()->SetMaterialsInteraction(firstMaterialName, secondMaterialName, staticFricCoeff, dynamicFricCoeff);
}

std::string SimulationManager::CreateLook(const std::string& name, Color color, float roughness, float metalness, float reflectivity, const std::string& albedoTexturePath, const std::string& normalTexturePath)
{
    if(SimulationApp::getApp()->hasGraphics())
        return ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->CreatePhysicalLook(name, color.rgb, roughness, metalness, reflectivity, albedoTexturePath, normalTexturePath);
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
    btMultiBodyDynamicsWorld* researchWorld = (btMultiBodyDynamicsWorld*)world;
    
    //Clear all forces to ensure that no summing occurs
    researchWorld->clearForces(); //Includes clearing of multibody forces!
    
    //Solve for objects settling
    bool objectsSettled = true;
    
    if(simManager->icUseGravity)
    {
        //Apply gravity to bodies
        for(size_t i = 0; i < simManager->entities.size(); ++i)
        {
            if(simManager->entities[i]->getType() == EntityType::SOLID)
            {
                SolidEntity* solid = (SolidEntity*)simManager->entities[i];
                solid->ApplyGravity(world->getGravity());
            }
            else if(simManager->entities[i]->getType() == EntityType::FEATHERSTONE)
            {
                FeatherstoneEntity* feather = (FeatherstoneEntity*)simManager->entities[i];
                feather->ApplyGravity(world->getGravity());
            }
        }
        
        if(simManager->simulationTime < Scalar(0.01)) //Wait for a few cycles to ensure bodies started moving
            objectsSettled = false;
        else
        {
            //Check if objects settled
            for(size_t i = 0; i < simManager->entities.size(); ++i)
            {
                if(simManager->entities[i]->getType() == EntityType::SOLID)
                {
                    SolidEntity* solid = (SolidEntity*)simManager->entities[i];
                    if(solid->getLinearVelocity().length() > simManager->icLinTolerance * Scalar(100.) || solid->getAngularVelocity().length() > simManager->icAngTolerance * Scalar(100.))
                    {
                        objectsSettled = false;
                        break;
                    }
                }
                else if(simManager->entities[i]->getType() == EntityType::FEATHERSTONE)
                {
                    FeatherstoneEntity* multibody = (FeatherstoneEntity*)simManager->entities[i];
                    
                    //Check base velocity
                    Vector3 baseLinVel = multibody->getLinkLinearVelocity(0);
                    Vector3 baseAngVel = multibody->getLinkAngularVelocity(0);
                    
                    if(baseLinVel.length() > simManager->icLinTolerance * Scalar(100.) || baseAngVel.length() > simManager->icAngTolerance * Scalar(100.0))
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
                                if(Vector3(jVelocity,0,0).length() > simManager->icAngTolerance * Scalar(100.))
                                    objectsSettled = false;
                                break;
                                
                            case btMultibodyLink::ePrismatic:
                                if(Vector3(jVelocity,0,0).length() > simManager->icLinTolerance * Scalar(100.))
                                    objectsSettled = false;
                                break;
                                
                            default:
                                break;
                        }
                        
                        if(!objectsSettled)
                            break;
                    }
                }
            }
        }
    }
    
    //Solve for joint initial conditions
    bool jointsICSolved = true;
    
    for(size_t i = 0; i < simManager->joints.size(); ++i)
        if(!simManager->joints[i]->SolvePositionIC(simManager->icLinTolerance, simManager->icAngTolerance))
            jointsICSolved = false;

    //Check if everything solved
    if(objectsSettled && jointsICSolved)
        simManager->icProblemSolved = true;
    
    //Update time
    simManager->simulationTime += timeStep;
}

//Used to apply and accumulate forces
void SimulationManager::SimulationTickCallback(btDynamicsWorld* world, Scalar timeStep)
{
    SimulationManager* simManager = (SimulationManager*)world->getWorldUserInfo();
    btMultiBodyDynamicsWorld* mbDynamicsWorld = (btMultiBodyDynamicsWorld*)world;
        
    //Clear all forces to ensure that no summing occurs
    mbDynamicsWorld->clearForces(); //Includes clearing of multibody forces!
        
    //loop through all actuators -> apply forces to bodies (free and connected by joints)
    for(size_t i = 0; i < simManager->actuators.size(); ++i)
        simManager->actuators[i]->Update(timeStep);
    
    //loop through all joints -> apply damping forces to bodies connected by joints
    for(size_t i = 0; i < simManager->joints.size(); ++i)
        simManager->joints[i]->ApplyDamping();
    
    //loop through all entities that may need special actions
    for(size_t i = 0; i < simManager->entities.size(); ++i)
    {
        Entity* ent = simManager->entities[i];
        
        if(ent->getType() == EntityType::SOLID)
        {
            SolidEntity* solid = (SolidEntity*)ent;
            solid->ApplyGravity(mbDynamicsWorld->getGravity());
        }
        else if(ent->getType() == EntityType::FEATHERSTONE)
        {
            FeatherstoneEntity* multibody = (FeatherstoneEntity*)ent;
            multibody->ApplyGravity(mbDynamicsWorld->getGravity());
            multibody->ApplyDamping();
        }
        /*else if(ent->getType() == EntityType::CABLE)
        {
            CableEntity* cable = (CableEntity*)ent;
            cable->ApplyGravity(mbDynamicsWorld->getGravity());
        }*/
        else if(ent->getType() == EntityType::FORCEFIELD)
        {
            ForcefieldEntity* ff = (ForcefieldEntity*)ent;
            if(ff->getForcefieldType() == ForcefieldType::TRIGGER)
            {				
                Trigger* trigger = (Trigger*)ff;
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
    bool recompute = simManager->fdCounter % simManager->fdPrescaler == 0;
    ++simManager->fdCounter;
    
    //Aerodynamic forces
    if(simManager->atmosphere != nullptr)
    {
        btBroadphasePairArray& pairArray = simManager->atmosphere->getGhost()->getOverlappingPairCache()->getOverlappingPairArray();
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
                    
                btCollisionObject* co1 = (btCollisionObject*)colPair->m_pProxy0->m_clientObject;
                btCollisionObject* co2 = (btCollisionObject*)colPair->m_pProxy1->m_clientObject;
                
                if(co1 == simManager->atmosphere->getGhost())
                    simManager->atmosphere->ApplyFluidForces(world, co2, recompute);
                else if(co2 == simManager->ocean->getGhost())
                    simManager->atmosphere->ApplyFluidForces(world, co1, recompute);
            }
        }
    }
    
    //Hydrodynamic forces
    if(simManager->ocean != nullptr)
    {
        if(recompute) SDL_LockMutex(simManager->simHydroMutex);
        simManager->perfMon.HydrodynamicsStarted();
        
        btBroadphasePairArray& pairArray = simManager->ocean->getGhost()->getOverlappingPairCache()->getOverlappingPairArray();
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
                    
                btCollisionObject* co1 = (btCollisionObject*)colPair->m_pProxy0->m_clientObject;
                btCollisionObject* co2 = (btCollisionObject*)colPair->m_pProxy1->m_clientObject;
                
                if(co1 == simManager->ocean->getGhost())
                    simManager->ocean->ApplyFluidForces(world, co2, recompute);
                else if(co2 == simManager->ocean->getGhost())
                    simManager->ocean->ApplyFluidForces(world, co1, recompute);
            }
        }
        
        simManager->perfMon.HydrodynamicsFinished();
        if(recompute) SDL_UnlockMutex(simManager->simHydroMutex);
    }
}

//Used to measure body motions and calculate controls
void SimulationManager::SimulationPostTickCallback(btDynamicsWorld *world, Scalar timeStep)
{
    SimulationManager* simManager = (SimulationManager*)world->getWorldUserInfo();
    
    //Update motion data
    for(size_t i = 0; i < simManager->entities.size(); ++i)
    {
        Entity* ent = simManager->entities[i];
            
        if(ent->getType() == EntityType::SOLID)
        {
            SolidEntity* solid = (SolidEntity*)ent;
            solid->UpdateAcceleration(timeStep);
        }
        else if(ent->getType() == EntityType::FEATHERSTONE)
        {
            FeatherstoneEntity* fe = (FeatherstoneEntity*)ent;
            fe->UpdateAcceleration(timeStep);
        }
        else if(ent->getType() == EntityType::ANIMATED)
        {
            AnimatedEntity* anim = (AnimatedEntity*)ent;
            anim->Update(timeStep);
        }
    }

    //Special treatment of suction cup actuator
    for(size_t i = 0; i < simManager->actuators.size(); ++i)
        if(simManager->actuators[i]->getType() == ActuatorType::SUCTION_CUP)
            ((SuctionCup*)simManager->actuators[i])->Engage(simManager);

    //Loop through all sensors -> update measurements
    for(size_t i = 0; i < simManager->sensors.size(); ++i)
        simManager->sensors[i]->Update(timeStep);
        
    //Loop through all comms -> update state and measurements
    for(size_t i = 0; i < simManager->comms.size(); ++i){
        simManager->comms[i]->Update(timeStep);
        if(simManager->comms[i]->getType()==CommType::VLC){
           dynamic_cast<VLC*>(simManager->comms[i])->setWaterType(simManager->getOcean()->getWaterType());
        }
        
    }
    
    //Loop through contact manifolds -> update contacts
    if(simManager->getContact(0) != nullptr) // If at least one contact is defined
    {
        int numManifolds = world->getDispatcher()->getNumManifolds();
        for(int i=0; i<numManifolds; ++i)
        {
            btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
            btCollisionObject* coA = (btCollisionObject*)contactManifold->getBody0();
            btCollisionObject* coB = (btCollisionObject*)contactManifold->getBody1();
            Entity* entA = (Entity*)coA->getUserPointer();
            Entity* entB = (Entity*)coB->getUserPointer();
            Contact* contact = simManager->getContact(entA, entB);
            if(contact != nullptr && contactManifold->getNumContacts() > 0)
                contact->AddContactPoint(contactManifold, contact->getEntityA() != entA, timeStep);        
        }
    }

    //Update simulation time
    simManager->simulationTime += timeStep;
    
    //Optional method to update some post simulation data (like ROS messages...)
    simManager->SimulationStepCompleted(timeStep);
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
