//
//  SimulationManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "SimulationManager.h"

#include <BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.h>
#include <BulletDynamics/MLCPSolvers/btDantzigSolver.h>
#include <BulletDynamics/MLCPSolvers/btSolveProjectedGaussSeidel.h>
#include <BulletDynamics/MLCPSolvers/btLemkeSolver.h>
#include <BulletDynamics/MLCPSolvers/btMLCPSolver.h>
#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>
#include "FilteredCollisionDispatcher.h"

#include "SimulationApp.h"
#include "OpenGLSolids.h"
#include "SystemUtil.h"
#include "OpenGLSky.h"
#include "OpenGLOmniLight.h"
#include "OpenGLSpotLight.h"
#include "OpenGLTrackball.h"
#include "SolidEntity.h"
#include "BoxEntity.h"
#include "StaticEntity.h"
#include "CableEntity.h"
#include "GhostEntity.h"
#include "FluidEntity.h"

#pragma mark Constructors
SimulationManager::SimulationManager(UnitSystems unitSystem, bool zAxisUp, btScalar stepsPerSecond, SolverType st, CollisionFilteringType cft)
{
    UnitSystem::SetUnitSystem(unitSystem, false);
    zUp = zAxisUp;
    setStepsPerSecond(stepsPerSecond);
    currentTime = 0;
    physicTime = 0;
    simulationTime = 0;
    drawLightDummies = false;
    drawCameraDummies = false;
    fluid = NULL;
    icProblemSolved = false;
    solver = st;
    collisionFilter = cft;
    materialManager = NULL;
    dynamicsWorld = NULL;
    dwSolver = NULL;
    dwBroadphase = NULL;
    dwCollisionConfig = NULL;
    dwDispatcher = NULL;
}

#pragma mark - Destructor
SimulationManager::~SimulationManager()
{
    DestroyScenario();
}

#pragma mark - Accessors
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

void SimulationManager::SetFluidEntity(FluidEntity *flu)
{
    if(flu != NULL)
    {
        //if(fluid != NULL)
        //remove old fluid
        
        fluid = flu;
        fluid->AddToDynamicsWorld(dynamicsWorld);
    }
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

void SimulationManager::AddPathGenerator(PathGenerator *pg)
{
    if(pg != NULL)
        pathGenerators.push_back(pg);
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

Contact* SimulationManager::AddContact(Entity *entA, Entity *entB, size_type contactHistoryLength)
{
    Contact* contact = getContact(entA, entB);
    
    if(contact == NULL)
    {
        contact = new Contact(entA, entB, contactHistoryLength);
        contacts.push_back(contact);
    }
    
    return contact;
}

bool SimulationManager::CheckContact(Entity *entA, Entity *entB)
{
    bool inContact = false;
    
    for(int i = 0; i < contacts.size(); i++)
    {
        if(contacts[i]->getEntityA() == entA)
            inContact = contacts[i]->getEntityB() == entB;
        else if(contacts[i]->getEntityB() == entA)
            inContact = contacts[i]->getEntityA() == entB;
        
        if(inContact)
            break;
    }
    
    return inContact;
}

Contact* SimulationManager::getContact(Entity* entA, Entity* entB)
{
    for(int i = 0; i < contacts.size(); i++)
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
    
    return NULL;
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

Entity* SimulationManager::getEntity(std::string name)
{
    for(int i = 0; i < entities.size(); i++)
        if(entities[i]->getName() == name)
            return entities[i];
    
    return NULL;
}

Joint* SimulationManager::getJoint(int index)
{
    if(index < joints.size())
        return joints[index];
    else
        return NULL;
}

Joint* SimulationManager::getJoint(std::string name)
{
    for(int i = 0; i < joints.size(); i++)
        if(joints[i]->getName() == name)
            return joints[i];
    
    return NULL;
}

Actuator* SimulationManager::getActuator(int index)
{
    if(index < actuators.size())
        return actuators[index];
    else
        return NULL;
}

Actuator* SimulationManager::getActuator(std::string name)
{
    for(int i = 0; i < actuators.size(); i++)
        if(actuators[i]->getName() == name)
            return actuators[i];
    
    return NULL;
}

Sensor* SimulationManager::getSensor(int index)
{
    if(index < sensors.size())
        return sensors[index];
    else
        return NULL;
}

Sensor* SimulationManager::getSensor(std::string name)
{
    for(int i = 0; i < sensors.size(); i++)
        if(sensors[i]->getName() == name)
            return sensors[i];
    
    return NULL;
}

Controller* SimulationManager::getController(int index)
{
    if(index < controllers.size())
        return controllers[index];
    else
        return NULL;
}

Controller* SimulationManager::getController(std::string name)
{
    for(int i = 0; i < controllers.size(); i++)
        if(controllers[i]->getName() == name)
            return controllers[i];
    
    return NULL;
}

ResearchDynamicsWorld* SimulationManager::getDynamicsWorld()
{
    return dynamicsWorld;
}

bool SimulationManager::isZAxisUp()
{
    return zUp;
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
    min.setValue(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
    max.setValue(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
    
    for(int i = 0; i < entities.size(); i++)
    {
        btVector3 entAabbMin, entAabbMax;
        
        if(entities[i]->getType() == ENTITY_SOLID)
        {
            SolidEntity* solid = (SolidEntity*)entities[i];
            solid->GetAABB(entAabbMin, entAabbMax);
        }
        else if(entities[i]->getType() == ENTITY_STATIC)
        {
            StaticEntity* stat = (StaticEntity*)entities[i];
            if(stat->getStaticType() != STATIC_PLANE)
                stat->GetAABB(entAabbMin, entAabbMax);
            else
                continue;
        }
        else
            continue;
        
        if(entAabbMin.x() < min.x()) min.setX(entAabbMin.x());
        if(entAabbMin.y() < min.y()) min.setY(entAabbMin.y());
        if(entAabbMin.z() < min.z()) min.setZ(entAabbMin.z());
        if(entAabbMax.x() > max.x()) max.setX(entAabbMax.x());
        if(entAabbMax.y() > max.y()) max.setY(entAabbMax.y());
        if(entAabbMax.z() > max.z()) max.setZ(entAabbMax.z());
    }
}

void SimulationManager::setGravity(btScalar gravityConstant)
{
    g = UnitSystem::SetAcceleration(btVector3(0., 0., gravityConstant)).getZ();
}

#pragma mark - Simulation
void SimulationManager::InitializeSolver()
{
    dwBroadphase = new btDbvtBroadphase();
    dwCollisionConfig = new btDefaultCollisionConfiguration();
  
    //Choose collision dispatcher
    switch(collisionFilter)
    {
        case STANDARD:
            dwDispatcher = new btCollisionDispatcher(dwCollisionConfig);
            break;
            
        case INCLUSIVE:
            dwDispatcher = new FilteredCollisionDispatcher(dwCollisionConfig, true);
            break;

        case EXCLUSIVE:
            dwDispatcher = new FilteredCollisionDispatcher(dwCollisionConfig, false);
            break;
    }
    
    //Choose constraint solver
    btMLCPSolverInterface* mlcp;
    
    switch(solver)
    {
        case DANTZIG:
            mlcp = new btDantzigSolver();
            break;
            
        case PROJ_GAUSS_SIEDEL:
            mlcp = new btSolveProjectedGaussSeidel();
            break;
            
        case LEMKE:
            mlcp = new btLemkeSolver();
            //((btLemkeSolver*)mlcp)->m_maxLoops = 10000;
            break;
    }
    
    //Create solver
    dwSolver = new ResearchConstraintSolver(mlcp);
    dynamicsWorld = new ResearchDynamicsWorld(dwDispatcher, dwBroadphase, dwSolver, dwCollisionConfig);
    dynamicsWorld->getSolverInfo().m_solverMode = SOLVER_USE_WARMSTARTING | SOLVER_SIMD | SOLVER_USE_2_FRICTION_DIRECTIONS | SOLVER_ENABLE_FRICTION_DIRECTION_CACHING; //| SOLVER_RANDMIZE_ORDER;
    dynamicsWorld->getSolverInfo().m_warmstartingFactor = 1.0;
    dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize = 1;

    //Quality/stability
    dynamicsWorld->getSolverInfo().m_tau = 1.0;  //mass factor
    dynamicsWorld->getSolverInfo().m_erp = 0.5;  //constraint error reduction in one step
    dynamicsWorld->getSolverInfo().m_erp2 = 1.0; //constraint error reduction in one step for split impulse
    dynamicsWorld->getSolverInfo().m_numIterations = 100; //number of constraint iterations
    dynamicsWorld->getSolverInfo().m_sor = 0; //not used
    dynamicsWorld->getSolverInfo().m_maxErrorReduction = 0; //not used
    
    //Collision
    dynamicsWorld->getSolverInfo().m_splitImpulse = true; //avoid adding energy to the system
    dynamicsWorld->getSolverInfo().m_splitImpulsePenetrationThreshold = -0.000001; //value close to zero needed for accurate friction
    dynamicsWorld->getSolverInfo().m_splitImpulseTurnErp = 1.0; //error reduction for rigid body angular velocity
    dynamicsWorld->getDispatchInfo().m_useContinuous = false;
    dynamicsWorld->getDispatchInfo().m_allowedCcdPenetration = -0.001;
    dynamicsWorld->setApplySpeculativeContactRestitution(true);
    dynamicsWorld->getSolverInfo().m_restingContactRestitutionThreshold = 1e30; //not used
    
    //Special forces
    dynamicsWorld->getSolverInfo().m_maxGyroscopicForce = 1e30; //gyroscopic effect
    
    //Unrealistic components
    dynamicsWorld->getSolverInfo().m_globalCfm = 0.0; //global constraint force mixing factor
    dynamicsWorld->getSolverInfo().m_damping = 0.0; //global damping
    dynamicsWorld->getSolverInfo().m_friction = 0.0; //global friction
    dynamicsWorld->getSolverInfo().m_singleAxisRollingFrictionThreshold = 1e30; //single axis rolling velocity threshold
    dynamicsWorld->getSolverInfo().m_linearSlop = 0.0; //position bias
    
    //Override default callbacks
    dynamicsWorld->setWorldUserInfo(this);
    dynamicsWorld->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    gContactAddedCallback = SimulationManager::CustomMaterialCombinerCallback;
    dynamicsWorld->setSynchronizeAllMotionStates(false);
    
    //Set default params
    g = btScalar(9.81);
    
    //Create material manager & load standard materials
    materialManager = new MaterialManager();
}

void SimulationManager::RestartScenario()
{
    DestroyScenario();
    InitializeSolver();
    BuildScenario();
}

void SimulationManager::DestroyScenario()
{
    if(dynamicsWorld != NULL)
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
        delete dwSolver;
        delete dwBroadphase;
        delete dwDispatcher;
        delete dwCollisionConfig;
    }
    
    //remove sim manager objects
    for(int i=0; i<entities.size(); i++)
        delete entities[i];
    entities.clear();
    
    if(fluid != NULL)
        delete fluid;
    
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
    
    for(int i=0; i<pathGenerators.size(); i++)
        delete pathGenerators[i];
    pathGenerators.clear();
    
    for(int i=0; i<views.size(); i++)
        delete views[i];
    views.clear();
    
    for(int i=0; i<lights.size(); i++)
        delete lights[i];
    lights.clear();
    
    if(materialManager != NULL)
        materialManager->ClearMaterialsAndFluids();
}

bool SimulationManager::StartSimulation()
{
    currentTime = 0;
    physicTime = 0;
    simulationTime = 0;
    
    //Solve initial conditions problem
    if(!SolveICProblem())
        return false;
    
    //Turn on controllers
    for(int i = 0; i < controllers.size(); i++)
        controllers[i]->Start();
    
    return true;
}

bool SimulationManager::SolveICProblem()
{
    //Solve for joint positions
    icProblemSolved = false;
    dynamicsWorld->setGravity(btVector3(0.,0.,0.)); //disable gravity
    dynamicsWorld->setInternalTickCallback(SolveICTickCallback, this, true);
    
    uint64_t icTime = GetTimeInMicroseconds();
    
    int iterations = 0;
    
    do
    {
        //Check iterations limit
        if(iterations > 100000)
        {
            cError("IC problem not solved! Reached maximum interation count.");
            return false;
        }
        iterations++;
        
        //Simulate world
        dynamicsWorld->stepSimulation(btScalar(0.001), 1, btScalar(0.001));
    }
    while(!icProblemSolved);
    
    double solveTime = (GetTimeInMicroseconds() - icTime)/(double)1e6;
    
    //Synchronize body transforms
    dynamicsWorld->synchronizeMotionStates();

    //Solving time
    cInfo("IC problem solved with %d iterations in %1.6lf s.", iterations, solveTime);
    
    //Set gravity
    dynamicsWorld->setGravity(btVector3(0., 0., zUp ? -g : g));
    
    //Set simulation tick
    dynamicsWorld->setInternalTickCallback(SimulationTickCallback, this, true);
    
    return true;
}

void SimulationManager::AdvanceSimulation(uint64_t timeInMicroseconds)
{
    if(!icProblemSolved)
        return;
    
	uint64_t deltaTime;
    if(currentTime == 0)
        deltaTime = 0.0;
    else if(timeInMicroseconds < currentTime)
        deltaTime = timeInMicroseconds + (UINT64_MAX - currentTime);
    else
        deltaTime = timeInMicroseconds - currentTime;
    currentTime = timeInMicroseconds;
    
    physicTime = GetTimeInMicroseconds();
    dynamicsWorld->stepSimulation((btScalar)deltaTime/btScalar(1000000.0), 1000000, (btScalar)ssus/btScalar(1000000.0));
    physicTime = GetTimeInMicroseconds() - physicTime;
    
    //Inform about MLCP failures
    int numFallbacks = dwSolver->getNumFallbacks();
    if(numFallbacks)
    {
        static int totalFailures = 0;
        totalFailures+=numFallbacks;
        cInfo("MLCP solver failed %d times.", totalFailures);
    }
    dwSolver->setNumFallbacks(0);
}

void SimulationManager::StopSimulation()
{
    for(int i=0; i < controllers.size(); i++)
        controllers[i]->Stop();
}

#pragma mark - To Be Moved Somewhere
Entity* SimulationManager::PickEntity(int x, int y)
{
    for(int i = 0; i < views.size(); i++)
    {
        if(views[i]->isActive())
        {
            btVector3 ray = views[i]->Ray(x, y);
            if(ray.length2() > 0)
            {
                btCollisionWorld::ClosestRayResultCallback rayCallback(views[i]->GetEyePosition(), ray);
                dynamicsWorld->rayTest(views[i]->GetEyePosition(), ray, rayCallback);
                if (rayCallback.hasHit())
                {
                    const btRigidBody* body = btRigidBody::upcast(rayCallback.m_collisionObject);
                    if (body)
                        return (Entity*)body->getUserPointer();
                }
                
                return NULL;
            }
        }
    }
    return NULL;
}

#pragma mark - Callbacks
extern ContactAddedCallback gContactAddedCallback;

bool SimulationManager::CustomMaterialCombinerCallback(btManifoldPoint& cp,	const btCollisionObjectWrapper* colObj0Wrap,int partId0,int index0,const btCollisionObjectWrapper* colObj1Wrap,int partId1,int index1)
{
    const btRigidBody* rbA = btRigidBody::upcast(colObj0Wrap->getCollisionObject());
    const btRigidBody* rbB = btRigidBody::upcast(colObj1Wrap->getCollisionObject());
    
    if(rbA == NULL || rbB == NULL)
    {
        cp.m_combinedFriction = btScalar(0.);
        cp.m_combinedRollingFriction = btScalar(0.);
        cp.m_combinedRestitution = btScalar(0.);
        return true;
    }
    
    Entity* entA = (Entity*)rbA->getUserPointer();
    Material* matA;
    if(entA->getType() == ENTITY_SOLID)
        matA = ((SolidEntity*)entA)->getMaterial();
    else if(entA->getType() == ENTITY_STATIC)
        matA = ((StaticEntity*)entA)->getMaterial();
    else if(entA->getType() == ENTITY_CABLE)
        matA = ((CableEntity*)entA)->getMaterial();
    else
    {
        cp.m_combinedFriction = btScalar(0.);
        cp.m_combinedRollingFriction = btScalar(0.);
        cp.m_combinedRestitution = btScalar(0.);
        return true;
    }
    
    Entity* entB = (Entity*)rbB->getUserPointer();
    Material* matB;
    if(entB->getType() == ENTITY_SOLID)
        matB = ((SolidEntity*)entB)->getMaterial();
    else if(entB->getType() == ENTITY_STATIC)
        matB = ((StaticEntity*)entB)->getMaterial();
    else if(entB->getType() == ENTITY_CABLE)
        matB = ((CableEntity*)entB)->getMaterial();
    else
    {
        cp.m_combinedFriction = btScalar(0.);
        cp.m_combinedRollingFriction = btScalar(0.);
        cp.m_combinedRestitution = btScalar(0.);
        return true;
    }
    
    //Calculate friction coefficient based on relative velocity
    btVector3 relLocalVel = rbB->getVelocityInLocalPoint(rbB->getCenterOfMassTransform().getBasis() * cp.m_localPointB) - rbA->getVelocityInLocalPoint(rbA->getCenterOfMassTransform().getBasis() * cp.m_localPointA);
    btVector3 normalVel = cp.m_normalWorldOnB * cp.m_normalWorldOnB.dot(relLocalVel);
    btVector3 slipVel = relLocalVel - normalVel;
    btScalar slipVelMod = slipVel.length();
    btScalar sigma = 100;
    // f = (static - dynamic)/(sigma * v^2 + 1) + dynamic
    cp.m_combinedFriction = (matA->staticFriction[matB->index] - matA->dynamicFriction[matB->index])/(sigma * slipVelMod * slipVelMod + btScalar(1.)) + matA->dynamicFriction[matB->index];
    
    //Rolling friction not possible to generalize - needs special treatment
    cp.m_combinedRollingFriction = btScalar(0.);
    
    //Slipping
    if(SimulationApp::getApp()->getSimulationManager()->getCollisionFilter() != STANDARD)
        cp.m_userPersistentData = (void *)(new btVector3(slipVel));
    
    //Restitution
    cp.m_combinedRestitution = matA->restitution * matB->restitution;
    
    return true;
}

void SimulationManager::SolveICTickCallback(btDynamicsWorld* world, btScalar timeStep)
{
    SimulationManager* simManager = (SimulationManager*)world->getWorldUserInfo();
    
    //Clear all forces to ensure that no summing occurs
    world->clearForces();
    
    //Solve for initial conditions
    int solved = 0;
    
    for(int i = 0; i < simManager->joints.size(); i++)
        solved += (int)simManager->joints[i]->SolvePositionIC(1e-6, 1e-6);
    
    if(solved == simManager->joints.size())
        simManager->icProblemSolved = true;
}

void SimulationManager::SimulationTickCallback(btDynamicsWorld* world, btScalar timeStep)
{
    SimulationManager* simManager = (SimulationManager*)world->getWorldUserInfo();
    
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
        if(ent->getType() == ENTITY_SOLID)
        {
            SolidEntity* simple = (SolidEntity*)ent;
            simple->ApplyGravity();
            //simple->getRigidBody()->setDamping(0, 0);
        }
        else if(ent->getType() == ENTITY_CABLE)
        {
            CableEntity* cable = (CableEntity*)ent;
            cable->ApplyGravity();
        }
        /*else if(simManager->entities[i]->getType() == GHOST) //ghost entities - action triggers
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
        }*/
    }
    
    //Update simulation time
    simManager->simulationTime += timeStep;
}
