//
//  SimulationManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "SimulationManager.h"

#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <BulletDynamics/MLCPSolvers/btDantzigSolver.h>
#include <BulletDynamics/MLCPSolvers/btSolveProjectedGaussSeidel.h>
#include <BulletDynamics/MLCPSolvers/btMLCPSolver.h>
#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>
#include "btFilteredCollisionDispatcher.h"

#include "SimulationApp.h"
#include "OpenGLSolids.h"
#include "OpenGLUtil.h"
#include "OpenGLSky.h"
#include "OpenGLOmniLight.h"
#include "OpenGLSpotLight.h"
#include "OpenGLTrackball.h"
#include "SolidEntity.h"
#include "BoxEntity.h"
#include "TerrainEntity.h"
#include "PlaneEntity.h"
#include "CableEntity.h"
#include "GhostEntity.h"
#include "FluidEntity.h"

//custom material callback - friction from a user defined table
extern ContactAddedCallback gContactAddedCallback;

bool SimulationManager::CustomMaterialCombinerCallback(btManifoldPoint& cp,	const btCollisionObjectWrapper* colObj0Wrap,int partId0,int index0,const btCollisionObjectWrapper* colObj1Wrap,int partId1,int index1)
{
    const btRigidBody* rb0 = btRigidBody::upcast(colObj0Wrap->getCollisionObject());
    Entity* ent0 = (Entity*)rb0->getUserPointer();
    Material* mat0;
    if(ent0->getType() == SOLID)
        mat0 = ((SolidEntity*)ent0)->getMaterial();
    else if(ent0->getType() == PLANE)
        mat0 = ((PlaneEntity*)ent0)->getMaterial();
    else if(ent0->getType() == TERRAIN)
        mat0 = ((TerrainEntity*)ent0)->getMaterial();
    else if(ent0->getType() == CABLE)
        mat0 = ((CableEntity*)ent0)->getMaterial();
    else
    {
        cp.m_combinedFriction = btScalar(1.);
        cp.m_combinedRestitution = btScalar(0.);
        return true;
    }
    
    const btRigidBody* rb1 = btRigidBody::upcast(colObj1Wrap->getCollisionObject());
    Entity* ent1 = (Entity*)rb1->getUserPointer();
    Material* mat1;
    if(ent1->getType() == SOLID)
        mat1 = ((SolidEntity*)ent1)->getMaterial();
    else if(ent1->getType() == PLANE)
        mat1 = ((PlaneEntity*)ent1)->getMaterial();
    else if(ent1->getType() == TERRAIN)
        mat1 = ((TerrainEntity*)ent1)->getMaterial();
    else if(ent1->getType() == CABLE)
        mat1 = ((CableEntity*)ent1)->getMaterial();
    else
    {
        cp.m_combinedFriction = btScalar(1.);
        cp.m_combinedRestitution = btScalar(0.);
        return true;
    }
    
    cp.m_combinedFriction = mat0->staticFriction[mat1->index];
    cp.m_combinedRollingFriction = mat0->dynamicFriction[mat1->index] * 0.1f;
    cp.m_combinedRestitution = mat0->restitution * mat1->restitution;
    
    //printf("%s<->%s %f %f %f\n", mat0->name.c_str(), mat1->name.c_str(), cp.m_combinedFriction, cp.m_combinedRollingFriction, cp.m_combinedRestitution);
    
    return true;
}
///////////////////////////

SimulationManager::SimulationManager(UnitSystems unitSystem, bool zAxisUp, btScalar stepsPerSecond, SolverType st, CollisionFilteringType cft)
{
    UnitSystem::SetUnitSystem(unitSystem, false);
    zUp = zAxisUp;
    setStepsPerSecond(stepsPerSecond);
    gContactAddedCallback = SimulationManager::CustomMaterialCombinerCallback;
    currentTime = 0;
    physicTime = 0;
    drawLightDummies = false;
    drawCameraDummies = false;
    InitializeSolver(st, cft);
}

SimulationManager::~SimulationManager(void)
{
    DestroyScenario();
}

void SimulationManager::AddEntity(Entity *ent)
{
    if(ent != NULL)
    {
        entities.push_back(ent);
        ent->AddToDynamicsWorld(dynamicsWorld);
    }
}

void SimulationManager::AddSolidEntity(SolidEntity* ent, const btTransform& worldTransform)
{
    if(ent != NULL)
    {
        entities.push_back(ent);
        ent->AddToDynamicsWorld(dynamicsWorld, worldTransform);
    }
}

void SimulationManager::AddFluidEntity(FluidEntity *flu)
{
    if(flu != NULL)
    {
        entities.push_back(flu);
        fluids.push_back(flu);
        flu->AddToDynamicsWorld(dynamicsWorld);
    }
}

void SimulationManager::AddJoint(Joint *jnt)
{
    if(jnt != NULL)
    {
        joints.push_back(jnt);
        jnt->AddToDynamicsWorld(dynamicsWorld);
    }
}

void SimulationManager::AddActuator(Actuator *act)
{
    if(act != NULL)
        actuators.push_back(act);
}

void SimulationManager::AddContact(Entity *e0, Entity *e1)
{
    bool contactExists = CheckContact(e0, e1);
    if(!contactExists)
    {
        Contact* c = new Contact(e0, e1);
        contacts.push_back(c);
    }
}

bool SimulationManager::CheckContact(Entity *e0, Entity *e1)
{
    bool inContact = false;
    
    for(int i = 0; i < contacts.size(); i++)
    {
        if(contacts[i]->getEntity0() == e0)
            inContact = contacts[i]->getEntity1() == e1;
        else if(contacts[i]->getEntity1() == e0)
            inContact = contacts[i]->getEntity0() == e1;
        
        if(inContact)
            break;
    }
    
    return inContact;
}

CollisionFilteringType SimulationManager::getCollisionFilter()
{
    return collisionFilter;
}

SolverType SimulationManager::getSolverType()
{
    return solver;
}

Entity* SimulationManager::getEntity(int index)
{
    if(index < entities.size())
        return entities[index];
    else
        return NULL;
}

void SimulationManager::AddSensor(Sensor *sens)
{
    if(sens != NULL)
        sensors.push_back(sens);
}

void SimulationManager::AddController(Controller *cntrl)
{
    if(cntrl != NULL)
        controllers.push_back(cntrl);
}

Entity* SimulationManager::getEntity(std::string name)
{
    for(int i=0; i<entities.size(); i++)
    {
        if(entities[i]->getName().compare(name) == 0)
            return entities[i];
    }
    
    return NULL;
}

Joint* SimulationManager::getJoint(int index)
{
    if(index < joints.size())
        return joints[index];
    else
        return NULL;
}

Actuator* SimulationManager::getActuator(int index)
{
    if(index < actuators.size())
        return actuators[index];
    else
        return NULL;
}

Sensor* SimulationManager::getSensor(int index)
{
    if(index < sensors.size())
        return sensors[index];
    else
        return NULL;
}

Controller* SimulationManager::getController(int index)
{
    if(index < controllers.size())
        return controllers[index];
    else
        return NULL;
}

void SimulationManager::AddView(OpenGLView* view)
{
    views.push_back(view);
}

OpenGLView* SimulationManager::getView(int index)
{
    if(index < views.size())
        return views[index];
    else
        return NULL;
}

void SimulationManager::AddLight(OpenGLLight* light)
{
    lights.push_back(light);
}

OpenGLLight* SimulationManager::getLight(int index)
{
    if(index < lights.size())
        return lights[index];
    else
        return NULL;
}

void SimulationManager::getWorldAABB(btVector3& min, btVector3& max)
{
    dynamicsWorld->getBroadphase()->getBroadphaseAabb(min, max);
}

void SimulationManager::InitializeSolver(SolverType st, CollisionFilteringType cft)
{
    solver = st;
    collisionFilter = cft;
    
    dwBroadphase = new btDbvtBroadphase();
    dwCollisionConfig = new btSoftBodyRigidBodyCollisionConfiguration();
    //dwCollisionConfig = new btDefaultCollisionConfiguration();
    //btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);

    switch(collisionFilter)
    {
        case STANDARD:
            dwDispatcher = new btCollisionDispatcher(dwCollisionConfig);
            break;
            
        case INCLUSIVE:
        case EXCLUSIVE:
            dwDispatcher = new btFilteredCollisionDispatcher(dwCollisionConfig);
            break;
    }
    
    switch(solver)
    {
        case SEQUENTIAL_IMPULSE:
            dwSolver = new btSequentialImpulseConstraintSolver();
            break;
            
        case DANTZIG:
        {
            btDantzigSolver* mlcp = new btDantzigSolver();
            dwSolver = new btMLCPSolver(mlcp);
        }
            break;
            
        case PROJ_GAUSS_SIEDEL:
        {
            btSolveProjectedGaussSeidel* mlcp = new btSolveProjectedGaussSeidel();
            dwSolver = new btMLCPSolver(mlcp);
        }
            break;
    }
    
    btSoftBodySolver* softBodySolver = 0;
    dynamicsWorld = new btSoftRigidDynamicsWorld(dwDispatcher, dwBroadphase, dwSolver, dwCollisionConfig, softBodySolver); //dynamicsWorld = new btDiscreteDynamicsWorld(dwDispatcher, dwBroadphase, dwSolver, dwCollisionConfig);
    
    dynamicsWorld->getSolverInfo().m_erp = GLOBAL_ERP;
    dynamicsWorld->getSolverInfo().m_erp2 = GLOBAL_ERP2;
    dynamicsWorld->getSolverInfo().m_maxErrorReduction = MAX_ERROR_REDUCTION;
    dynamicsWorld->getSolverInfo().m_globalCfm = 0.0;
    dynamicsWorld->getSolverInfo().m_splitImpulse = true;
    dynamicsWorld->getSolverInfo().m_splitImpulsePenetrationThreshold = -0.01;
    dynamicsWorld->getSolverInfo().m_splitImpulseTurnErp = 0.2;
    dynamicsWorld->getSolverInfo().m_sor = 1.0;
    dynamicsWorld->getSolverInfo().m_linearSlop = 0.0;
    dynamicsWorld->getSolverInfo().m_warmstartingFactor = 1.0;
    dynamicsWorld->getSolverInfo().m_solverMode = SOLVER_ENABLE_FRICTION_DIRECTION_CACHING | SOLVER_USE_2_FRICTION_DIRECTIONS | SOLVER_SIMD | SOLVER_RANDMIZE_ORDER;
    dynamicsWorld->getSolverInfo().m_tau = 0.8;
    dynamicsWorld->getSolverInfo().m_damping = GLOBAL_DAMPING;
    dynamicsWorld->getSolverInfo().m_friction = GLOBAL_FRICTION;
    dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize = 64;
    dynamicsWorld->getSolverInfo().m_maxGyroscopicForce = 10e6;
    dynamicsWorld->getSolverInfo().m_singleAxisRollingFrictionThreshold = 100.f;
    //dynamicsWorld->getSolverInfo().m_numIterations = 50;
    //dynamicsWorld->getSolverInfo().m_timeStep = btScalar(1.f/getStepsPerSecond());
    
    //dynamicsWorld->getDispatchInfo().m_useContinuous = USE_CONTINUOUS_COLLISION;
    //dynamicsWorld->getDispatchInfo().m_allowedCcdPenetration = ALLOWED_CCD_PENETRATION;
    
    dynamicsWorld->setWorldUserInfo(this);
    dynamicsWorld->setInternalTickCallback(SimulationTickCallback, this, true);
    dynamicsWorld->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    
    //set standard world params
    setGravity(UnitSystem::Acceleration(MKS, UnitSystem::GetUnitSystem(), btVector3(btScalar(0.), btScalar(0.), (zUp ? btScalar(-9.81) : btScalar(9.81)) )));
    
    //create material manager & load standard materials
    materialManager = new MaterialManager();
}

void SimulationManager::DestroyScenario()
{
    //remove objects from dynamic world
    for (int i=dynamicsWorld->getNumConstraints()-1; i>=0 ;i--)
	{
		btTypedConstraint* constraint = dynamicsWorld->getConstraint(i);
		dynamicsWorld->removeConstraint(constraint);
		delete constraint;
	}
    
    for(int i=dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
			delete body->getMotionState();
        dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}
    
    //remove sim manager objects
    for(int i=0; i<entities.size(); i++)
        delete entities[i];
    entities.clear();
    fluids.clear();
    
    for(int i=0; i<joints.size(); i++)
        delete joints[i];
    joints.clear();
    
    for(int i=0; i<contacts.size(); i++)
        delete contacts[i];
    contacts.clear();
    
    for(int i=0; i<sensors.size(); i++)
        delete sensors[i];
    sensors.clear();
    
    for(int i=0; i<actuators.size(); i++)
        delete actuators[i];
    actuators.clear();
    
    for(int i=0; i<controllers.size(); i++)
        delete controllers[i];
    controllers.clear();
    
    for(int i=0; i<views.size(); i++)
        delete views[i];
    views.clear();
    
    for(int i=0; i<lights.size(); i++)
        delete lights[i];
    lights.clear();
    
    //materialManager->clearMaterialsAndFluids();
    delete materialManager;
    delete dynamicsWorld;
    delete dwCollisionConfig;
    delete dwDispatcher;
    delete dwSolver;
    delete dwBroadphase;
}

void SimulationManager::RestartScenario()
{
    DestroyScenario();
    BuildScenario();
    dynamicsWorld->synchronizeMotionStates();
}

void SimulationManager::AdvanceSimulation(uint64_t timeInMicroseconds)
{
	uint64_t deltaTime;
    if(currentTime == 0)
        deltaTime = 0.0;
    else if(timeInMicroseconds < currentTime)
        deltaTime = timeInMicroseconds + (UINT64_MAX - currentTime);
    else
        deltaTime = timeInMicroseconds - currentTime;
    currentTime = timeInMicroseconds;
    
    int substeps = 1000000;
    
    physicTime = GetTimeInMicroseconds();
    dynamicsWorld->stepSimulation((btScalar)deltaTime/btScalar(1000000.0), substeps, (btScalar)ssus/btScalar(1000000.0));
    physicTime = GetTimeInMicroseconds() - physicTime;
    
   if (dynamicsWorld->getConstraintSolver()->getSolverType()==BT_MLCP_SOLVER)
   {
        btMLCPSolver* solver = (btMLCPSolver*) dynamicsWorld->getConstraintSolver();
        int numFallbacks = solver->getNumFallbacks();
        if (numFallbacks)
        {
            static int totalFailures = 0;
            totalFailures+=numFallbacks;
            printf("MLCP solver failed %d times, falling back to btSequentialImpulseSolver (SI)\n",totalFailures);
        }
        solver->setNumFallbacks(0);
    }
}

void SimulationManager::StartSimulation()
{
    currentTime = 0;
    physicTime = 0;
    
    for(int i=0; i < controllers.size(); i++)
        controllers[i]->Start();
}

void SimulationManager::StopSimulation()
{
    for(int i=0; i < controllers.size(); i++)
        controllers[i]->Stop();
}

void SimulationManager::setGravity(const btVector3& g)
{
#ifdef USE_SOFT_BODY_DYNAMICS
    dynamicsWorld->setGravity(UnitSystem::SetAcceleration(g));
    softBodyWorldInfo.m_gravity = UnitSystem::SetAcceleration(g);
#else
    dynamicsWorld->setGravity(UnitSystem::SetAcceleration(g));
#endif
}

btDynamicsWorld* SimulationManager::getDynamicsWorld()
{
    return dynamicsWorld;
}

btScalar SimulationManager::getSimulationTime()
{
    return simulationTime;
}

MaterialManager* SimulationManager::getMaterialManager()
{
    return materialManager;
}

void SimulationManager::setStepsPerSecond(btScalar steps)
{
    sps = steps;
    ssus = (uint64_t)(1000000.0/steps);
}

btScalar SimulationManager::getStepsPerSecond()
{
    return sps;
}

double SimulationManager::getPhysicsTimeInMiliseconds()
{
    return (double)physicTime/1000.0;
}

//static members
void SimulationManager::SimulationTickCallback(btDynamicsWorld *world, btScalar timeStep)
{
    SimulationManager* simManager = (SimulationManager*)world->getWorldUserInfo();
    simManager->simulationTime += timeStep;
    
    //clear all forces to ensure that no summing occurs
    world->clearForces();
    
    //loop through all sensors -> update measurements
    for(int i = 0; i < simManager->sensors.size(); i++)
        simManager->sensors[i]->Update(timeStep);

    //loop through all controllers
    for(int i = 0; i < simManager->controllers.size(); i++)
        simManager->controllers[i]->Update(timeStep);
    
    //loop through all actuators -> apply forces to bodies (free and connected by joints)
    for(int i = 0; i < simManager->actuators.size(); i++)
        simManager->actuators[i]->Update(timeStep);
    
    //loop through all joints -> apply damping forces to bodies connected by joints
    for(int i = 0; i < simManager->joints.size(); i++)
        simManager->joints[i]->ApplyDamping();
    
    //loop through all entities that may need special actions
    for(int i = 0; i < simManager->entities.size(); i++)
    {
        Entity* ent = simManager->entities[i];
        
        //apply gravity only to dynamic objects
        if(ent->getType() == SOLID)
        {
            SolidEntity* simple = (SolidEntity*)ent;
            simple->ApplyGravity();
            //simple->getRigidBody()->setDamping(0, 0);
        }
        else if(ent->getType() == CABLE)
        {
            CableEntity* cable = (CableEntity*)ent;
            cable->ApplyGravity();
        }
        else if(simManager->entities[i]->getType() == GHOST) //ghost entities - action triggers
        {
            btManifoldArray manifoldArray;
            GhostEntity* ghost = (GhostEntity*)simManager->entities[i];
            btBroadphasePairArray& pairArray = ghost->getGhost()->getOverlappingPairCache()->getOverlappingPairArray();
            int numPairs = pairArray.size();
            
            //pool filled with liquid - buoyancy force
            if(numPairs > 0 && ghost->getGhostType() == FLUID)
            {
                FluidEntity* fluid = (FluidEntity*)simManager->entities[i];
                
                for(int h=0; h<numPairs; h++)
                {
                    manifoldArray.clear();
                    const btBroadphasePair& pair = pairArray[h];
                    btBroadphasePair* colPair = world->getPairCache()->findPair(pair.m_pProxy0,pair.m_pProxy1);
                    if (!colPair)
                        continue;
                    
                    btCollisionObject* co1 = (btCollisionObject*)colPair->m_pProxy0->m_clientObject;
                    btCollisionObject* co2 = (btCollisionObject*)colPair->m_pProxy1->m_clientObject;
                    
                    if(co1 == fluid->getGhost())
                        fluid->ApplyFluidForces(world, co2);
                    else if(co2 == fluid->getGhost())
                        fluid->ApplyFluidForces(world, co1);
                }
            }
        }
    }
}
