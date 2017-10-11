//
//  SimulationManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012-2017 Patryk Cieslak. All rights reserved.
//

#include "SimulationManager.h"

#include <chrono>
#include <thread>
#include <BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.h>
#include <BulletDynamics/MLCPSolvers/btDantzigSolver.h>
#include <BulletDynamics/MLCPSolvers/btSolveProjectedGaussSeidel.h>
#include <BulletDynamics/MLCPSolvers/btLemkeSolver.h>
#include <BulletDynamics/MLCPSolvers/btMLCPSolver.h>
#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>
#include "FilteredCollisionDispatcher.h"

#include "SimulationApp.h"
#include "SystemUtil.hpp"
#include "OpenGLTrackball.h"
#include "SolidEntity.h"
#include "Box.h"
#include "StaticEntity.h"
#include "CableEntity.h"
#include "ForcefieldEntity.h"
#include "Ocean.h"
#include "Plane.h"

SimulationManager::SimulationManager(SimulationType t, UnitSystems unitSystem, btScalar stepsPerSecond, SolverType st, CollisionFilteringType cft, HydrodynamicsType ht)
{
    //Set coordinate system
    UnitSystem::SetUnitSystem(unitSystem, false);
    simType = t;
    zUp = simType == SimulationType::MARINE ? false : true;
    
    //Initialize simulation world
    setStepsPerSecond(stepsPerSecond);
	realtimeFactor = btScalar(1);
    solver = st;
    collisionFilter = cft;
	hydroType = ht;
    currentTime = 0;
    physicsTime = 0;
    simulationTime = 0;
    mlcpFallbacks = 0;
    materialManager = NULL;
    dynamicsWorld = NULL;
    dwSolver = NULL;
    dwBroadphase = NULL;
    dwCollisionConfig = NULL;
    dwDispatcher = NULL;
    ocean = NULL;
	trackball = NULL;
    simSettingsMutex = SDL_CreateMutex();
    simInfoMutex = SDL_CreateMutex();
    
    //Set IC solver params
    icProblemSolved = false;
    setICSolverParams(false);
	simulationFresh = false;
    
    //Misc
    drawLightDummies = false;
    drawCameraDummies = false;
}

SimulationManager::~SimulationManager()
{
    DestroyScenario();
	delete ocean;
    SDL_DestroyMutex(simSettingsMutex);
    SDL_DestroyMutex(simInfoMutex);
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

void SimulationManager::AddSystemEntity(SystemEntity* ent, const btTransform& worldTransform)
{
    if(ent != NULL)
    {
        entities.push_back(ent);
        ent->AddToDynamicsWorld(dynamicsWorld, worldTransform);
    }
}

void SimulationManager::EnableOcean(Fluid* f)
{
	if(ocean != NULL)
		return;
	
	if(f != NULL)
	{
		ocean = new Ocean("Ocean", f);
	}
	else
	{
		std::string water = getMaterialManager()->CreateFluid("Water", UnitSystem::Density(CGS, MKS, 1.0), 1.308e-3, 1.55); 
		ocean = new Ocean("Ocean", getMaterialManager()->getFluid(water));
	}	
	
	ocean->setRenderable(true);
	ocean->AddToDynamicsWorld(dynamicsWorld);
	ocean->getOpenGLOcean().InitOcean();
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

Contact* SimulationManager::getContact(unsigned int index)
{
    if(index < contacts.size())
        return contacts[index];
    else
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

SimulationType SimulationManager::getSimulationType()
{
	return simType;
}

HydrodynamicsType SimulationManager::getHydrodynamicsType()
{
	return hydroType;
}

Entity* SimulationManager::getEntity(unsigned int index)
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

Joint* SimulationManager::getJoint(unsigned int index)
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

Actuator* SimulationManager::getActuator(unsigned int index)
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

Sensor* SimulationManager::getSensor(unsigned int index)
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

Controller* SimulationManager::getController(unsigned int index)
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

Ocean* SimulationManager::getOcean()
{
	if(simType == MARINE)
		return ocean;
	else
		return NULL;
}

btMultiBodyDynamicsWorld* SimulationManager::getDynamicsWorld()
{
    return dynamicsWorld;
}

bool SimulationManager::isZAxisUp()
{
    return zUp;
}

bool SimulationManager::isSimulationFresh()
{
	return simulationFresh;
}

btScalar SimulationManager::getSimulationTime()
{
    SDL_LockMutex(simInfoMutex);
    btScalar st = simulationTime;
    SDL_UnlockMutex(simInfoMutex);
    return st;
}

MaterialManager* SimulationManager::getMaterialManager()
{
    return materialManager;
}

void SimulationManager::setStepsPerSecond(btScalar steps)
{
    if(sps == steps)
        return;
    
    SDL_LockMutex(simSettingsMutex);
    sps = steps;
    ssus = (uint64_t)(1000000.0/steps);
    SDL_UnlockMutex(simSettingsMutex);
}

btScalar SimulationManager::getStepsPerSecond()
{
    return sps;
}

btScalar SimulationManager::getPhysicsTimeInMiliseconds()
{
    SDL_LockMutex(simInfoMutex);
    btScalar t = (btScalar)physicsTime/btScalar(1000);
    SDL_UnlockMutex(simInfoMutex);
    return t;
}

btScalar SimulationManager::getRealtimeFactor()
{
    SDL_LockMutex(simInfoMutex);
    btScalar rf = realtimeFactor;
    SDL_UnlockMutex(simInfoMutex);
	return rf;
}

void SimulationManager::getWorldAABB(btVector3& min, btVector3& max)
{
    min.setValue(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
    max.setValue(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
    
    for(int i = 0; i < entities.size(); i++)
    {
        btVector3 entAabbMin, entAabbMax;
        entities[i]->GetAABB(entAabbMin, entAabbMax);
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

btVector3 SimulationManager::getGravity()
{
    return btVector3(0., 0., zUp ? -g : g);
}

void SimulationManager::setICSolverParams(bool useGravity, btScalar timeStep, unsigned int maxIterations, btScalar maxTime, btScalar linearTolerance, btScalar angularTolerance)
{
    icUseGravity = useGravity;
    icTimeStep = timeStep > SIMD_EPSILON ? timeStep : btScalar(0.001);
    icMaxIter = maxIterations > 0 ? maxIterations : INT_MAX;
    icMaxTime = maxTime > SIMD_EPSILON ? maxTime : BT_LARGE_FLOAT;
    icLinTolerance = linearTolerance > SIMD_EPSILON ? linearTolerance : btScalar(1e-6);
    icAngTolerance = angularTolerance > SIMD_EPSILON ? angularTolerance : btScalar(1e-6);
}

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
    if(solver == SolverType::SI)
	{
		//Create solver and world
		dwSolver = new btMultiBodyConstraintSolver();
		dynamicsWorld = new btMultiBodyDynamicsWorld(dwDispatcher, dwBroadphase, dwSolver, dwCollisionConfig);
	}
	else
	{
		btMLCPSolverInterface* mlcp;
    
		switch(solver)
		{
			case SolverType::DANTZIG:
				mlcp = new btDantzigSolver();
				break;
            
			case SolverType::PROJ_GAUSS_SIEDEL:
				mlcp = new btSolveProjectedGaussSeidel();
				break;
            
			case SolverType::LEMKE:
				mlcp = new btLemkeSolver();
				//((btLemkeSolver*)mlcp)->m_maxLoops = 10000;
				break;
            
            case SolverType::SI: //Never happens, warning suppression
                break;
		}
		
		//Create solver and world
		dwSolver = new ResearchConstraintSolver(mlcp);
		dynamicsWorld = new ResearchDynamicsWorld(dwDispatcher, dwBroadphase, (ResearchConstraintSolver*)dwSolver, dwCollisionConfig);
	}
    
    //Basic configuration
    dynamicsWorld->getSolverInfo().m_solverMode = SOLVER_USE_WARMSTARTING | SOLVER_SIMD | SOLVER_USE_2_FRICTION_DIRECTIONS | SOLVER_RANDMIZE_ORDER; // | SOLVER_ENABLE_FRICTION_DIRECTION_CACHING; //| SOLVER_RANDMIZE_ORDER;
    dynamicsWorld->getSolverInfo().m_warmstartingFactor = btScalar(1.);
    dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize = 256;
	
    //Quality/stability
	dynamicsWorld->getSolverInfo().m_tau = btScalar(1.);  //mass factor
    dynamicsWorld->getSolverInfo().m_erp = btScalar(0.25);  //non-contact constraint error reduction
    dynamicsWorld->getSolverInfo().m_erp2 = btScalar(0.75); //contact constraint error reduction
    dynamicsWorld->getSolverInfo().m_frictionERP = btScalar(0.5); //friction constraint error reduction
    dynamicsWorld->getSolverInfo().m_numIterations = 100; //number of constraint iterations
    dynamicsWorld->getSolverInfo().m_sor = btScalar(1.0); //not used
    dynamicsWorld->getSolverInfo().m_maxErrorReduction = btScalar(0.); //not used
    
    //Collision
    dynamicsWorld->getSolverInfo().m_splitImpulse = true; //avoid adding energy to the system
    dynamicsWorld->getSolverInfo().m_splitImpulsePenetrationThreshold = btScalar(-0.001); //value close to zero needed for accurate friction/too close to zero causes multibody sinking
    dynamicsWorld->getSolverInfo().m_splitImpulseTurnErp = btScalar(1.); //error reduction for rigid body angular velocity
    dynamicsWorld->getDispatchInfo().m_useContinuous = false;
    dynamicsWorld->getDispatchInfo().m_allowedCcdPenetration = btScalar(-0.001);
    dynamicsWorld->setApplySpeculativeContactRestitution(false); //to make it work one needs restitution in the m_restitution field
    dynamicsWorld->getSolverInfo().m_restingContactRestitutionThreshold = INT_MAX; //not used
    
    //Special forces
    dynamicsWorld->getSolverInfo().m_maxGyroscopicForce = btScalar(1e30); //gyroscopic effect
    
    //Unrealistic components
    dynamicsWorld->getSolverInfo().m_globalCfm = btScalar(0.); //global constraint force mixing factor
    dynamicsWorld->getSolverInfo().m_damping = btScalar(0.); //global damping
    dynamicsWorld->getSolverInfo().m_friction = btScalar(0.); //global friction
    dynamicsWorld->getSolverInfo().m_frictionCFM = btScalar(0.); //friction constraint force mixing factor
    dynamicsWorld->getSolverInfo().m_singleAxisRollingFrictionThreshold = btScalar(1e30); //single axis rolling velocity threshold
    dynamicsWorld->getSolverInfo().m_linearSlop = btScalar(0.); //position bias
    
    //Override default callbacks
    dynamicsWorld->setWorldUserInfo(this);
    dynamicsWorld->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    gContactAddedCallback = SimulationManager::CustomMaterialCombinerCallback;
    dynamicsWorld->setSynchronizeAllMotionStates(false);
    
    //Set default params
    g = btScalar(9.81);
    
    //Create material manager & load standard materials
    materialManager = new MaterialManager();
    
    //Debugging
    debugDrawer = new OpenGLDebugDrawer(btIDebugDraw::DBG_DrawWireframe, zUp);
    dynamicsWorld->setDebugDrawer(debugDrawer);
}

void SimulationManager::InitializeScenario()
{
    switch(simType)
    {
        case TERRESTIAL:
        {
            //Plane
            getMaterialManager()->CreateMaterial("Ground", 1000.0, 1.0);
            std::string path = GetDataPath() + "grid.png";
            int grid = OpenGLContent::getInstance()->CreateSimpleLook(glm::vec3(1.f, 1.f, 1.f), 0.f, 0.1f, path);
            
            Plane* floor = new Plane("Floor", 1000.f, getMaterialManager()->getMaterial("Ground"), btTransform(btQuaternion(0,0,0), btVector3(0,0,0)), grid);
            AddEntity(floor);
		}
            break;
        
        case MARINE:
        {
            //Ocean
			EnableOcean();
        }
            break;
            
        case CUSTOM:
            break;
			
		default:
			break;
    }
    
    //Standard trackball
    trackball = new OpenGLTrackball(btVector3(0,0,1.0), 10.0, btVector3(0,0, 1.0), 0, 0, SimulationApp::getApp()->getWindowWidth(), SimulationApp::getApp()->getWindowHeight(), 90.f, 1000.f, 4, true);
	trackball->Rotate(btQuaternion(0.25, 0.0, 0.0));
	trackball->Activate();
    OpenGLContent::getInstance()->AddView(trackball);
}

void SimulationManager::RestartScenario()
{
    DestroyScenario();
    InitializeSolver();
    InitializeScenario();
    BuildScenario(); //Defined by specific application
	OpenGLContent::getInstance()->Finalize();
	
	simulationFresh = true;
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
        delete debugDrawer;
    }
    
    //remove sim manager objects
    for(int i=0; i<entities.size(); i++)
        delete entities[i];
    entities.clear();
    
    if(ocean != NULL)
	{
        delete ocean;
		ocean = NULL;
	}
    
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
	
    if(materialManager != NULL)
        materialManager->ClearMaterialsAndFluids();
		
	OpenGLContent::getInstance()->DestroyContent();
}

bool SimulationManager::StartSimulation()
{
	simulationFresh = false;
    currentTime = 0;
    physicsTime = 0;
    simulationTime = 0;
    mlcpFallbacks = 0;
    
    //Solve initial conditions problem
    if(!SolveICProblem())
        return false;
    
    //Reset contacts
    for(int i = 0; i < contacts.size(); i++)
        contacts[i]->ClearHistory();
    
    //Reset sensors
    for(int i = 0; i < sensors.size(); i++)
        sensors[i]->Reset();
    
    //Turn on controllers
    for(int i = 0; i < controllers.size(); i++)
        controllers[i]->Start();
    
    return true;
}

void SimulationManager::ResumeSimulation()
{
    if(!icProblemSolved)
        StartSimulation();
    else
    {
        currentTime = 0;

        for(int i = 0; i < controllers.size(); i++)
            controllers[i]->Start();
    }
}

void SimulationManager::StopSimulation()
{
    for(int i=0; i < controllers.size(); i++)
        controllers[i]->Stop();
}

bool SimulationManager::SolveICProblem()
{
    //Solve for joint positions
    icProblemSolved = false;
    
    //Should use gravity?
    if(icUseGravity)
        dynamicsWorld->setGravity(btVector3(0., 0., zUp ? -g : g));
    else
        dynamicsWorld->setGravity(btVector3(0.,0.,0.));
    
    //Set IC callback
    dynamicsWorld->setInternalTickCallback(SolveICTickCallback, this, true); //Pre-tick
    dynamicsWorld->setInternalTickCallback(NULL, this, false); //Post-tick
    
    uint64_t icTime = GetTimeInMicroseconds();
    int iterations = 0;
    
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
    simulationTime = btScalar(0.);

    //Solving time
    cInfo("IC problem solved with %d iterations in %1.6lf s.", iterations, solveTime);
    
    //Set gravity
    dynamicsWorld->setGravity(btVector3(0., 0., zUp ? -g : g));
    
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
	uint64_t timeInMicroseconds = GetTimeInMicroseconds();
	uint64_t deltaTime;
    
    //Start of simulation
    if(currentTime == 0)
	{
        deltaTime = 0.0;
		currentTime = timeInMicroseconds;
		return;
	}
    
    //Calculate and adjust delta
    deltaTime = timeInMicroseconds - currentTime;
    currentTime = timeInMicroseconds;
	
    if(deltaTime < ssus)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(ssus - deltaTime));
        timeInMicroseconds = GetTimeInMicroseconds();
        deltaTime = timeInMicroseconds - (currentTime - deltaTime);
        currentTime = timeInMicroseconds;
    }
    
    //Step simulation
    SDL_LockMutex(simSettingsMutex);
    uint64_t physicsStart = GetTimeInMicroseconds();
    dynamicsWorld->stepSimulation((btScalar)deltaTime * realtimeFactor/btScalar(1000000.0), 1000000, (btScalar)ssus/btScalar(1000000.0));
    uint64_t physicsEnd = GetTimeInMicroseconds();
    SDL_UnlockMutex(simSettingsMutex);
    
    SDL_LockMutex(simInfoMutex);
    physicsTime = physicsEnd - physicsStart;
    
    btScalar factor1 = (btScalar)deltaTime/(btScalar)physicsTime;
    btScalar factor2 = btScalar(1000000.0/60.0)/(btScalar)physicsTime;
    realtimeFactor *=  factor1*factor2;
    realtimeFactor = realtimeFactor < btScalar(0.05) ? btScalar(0.05) : (realtimeFactor > btScalar(1) ? btScalar(1) : realtimeFactor);
	
    //Inform about MLCP failures
	if(solver != SolverType::SI)
	{
		int numFallbacks = ((ResearchConstraintSolver*)dwSolver)->getNumFallbacks();
		if(numFallbacks)
		{
			mlcpFallbacks += numFallbacks;
			//cInfo("MLCP solver failed %d times.", mlcpFallbacks);
		}
		((ResearchConstraintSolver*)dwSolver)->setNumFallbacks(0);
	}
    
    SDL_UnlockMutex(simInfoMutex);
}

void SimulationManager::UpdateDrawingQueue()
{
	//Clear old items
	OpenGLPipeline::getInstance()->ClearDrawingQueue();
	
	//Build new drawing queue
    //Solids, manipulators, systems....
	for(unsigned int i=0; i<entities.size(); ++i)
	{
		std::vector<Renderable> items = entities[i]->Render();
		for(unsigned int h=0; h<items.size(); ++h)
		{
			if(!zUp)
			{
				items[h].model = glm::rotate((float)M_PI, glm::vec3(0,1.f,0)) * items[h].model;
				items[h].csModel = glm::rotate((float)M_PI, glm::vec3(0,1.f,0)) * items[h].csModel;
                items[h].eModel = glm::rotate((float)M_PI, glm::vec3(0,1.f,0)) * items[h].eModel;
			}
			
			OpenGLPipeline::getInstance()->AddToDrawingQueue(items[h]);
		}
	}
    
    //Motors, thrusters, propellers, fins....
    for(unsigned int i=0; i<actuators.size(); ++i)
    {
        std::vector<Renderable> items = actuators[i]->Render();
		for(unsigned int h=0; h<items.size(); ++h)
		{
			if(!zUp)
            {
				items[h].model = glm::rotate((float)M_PI, glm::vec3(0,1.f,0)) * items[h].model;
                items[h].csModel = glm::rotate((float)M_PI, glm::vec3(0,1.f,0)) * items[h].csModel;
			}
			
			OpenGLPipeline::getInstance()->AddToDrawingQueue(items[h]);
		}
    }
}

Entity* SimulationManager::PickEntity(int x, int y)
{
	/*
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
                    Entity* ent = (Entity*)rayCallback.m_collisionObject->getUserPointer();
                    return ent;
                }
                else
                    return NULL;
            }
        }
    }
    */
    return NULL;
}

extern ContactAddedCallback gContactAddedCallback;

bool SimulationManager::CustomMaterialCombinerCallback(btManifoldPoint& cp,	const btCollisionObjectWrapper* colObj0Wrap,int partId0,int index0,const btCollisionObjectWrapper* colObj1Wrap,int partId1,int index1)
{
    Entity* ent0 = (Entity*)colObj0Wrap->getCollisionObject()->getUserPointer();
    Entity* ent1 = (Entity*)colObj1Wrap->getCollisionObject()->getUserPointer();
    
    if(ent0 == NULL || ent1 == NULL)
    {
        cp.m_combinedFriction = btScalar(0.);
        cp.m_combinedRollingFriction = btScalar(0.);
        cp.m_combinedRestitution = btScalar(0.);
        return true;
    }
    
	MaterialManager* mm = SimulationApp::getApp()->getSimulationManager()->getMaterialManager();
	
    Material mat0;
    btVector3 contactVelocity0;
    btScalar contactAngularVelocity0;
    
    if(ent0->getType() == ENTITY_STATIC)
    {
		StaticEntity* sent0 = (StaticEntity*)ent0;
        mat0 = sent0->getMaterial();
        contactVelocity0 = btVector3(0.,0.,0.);
        contactAngularVelocity0 = btScalar(0.);
    }
    else if(ent0->getType() == ENTITY_SOLID)
    {
        SolidEntity* sent0 = (SolidEntity*)ent0;
        mat0 = sent0->getMaterial();
        btVector3 localPoint0 = sent0->getTransform().getBasis() * cp.m_localPointA;
        contactVelocity0 = sent0->getLinearVelocityInLocalPoint(localPoint0);
        contactAngularVelocity0 = sent0->getAngularVelocity().dot(-cp.m_normalWorldOnB);
    }
    else
    {
        cp.m_combinedFriction = btScalar(0.);
        cp.m_combinedRollingFriction = btScalar(0.);
        cp.m_combinedRestitution = btScalar(0.);
        return true;
    }
    
    Material mat1;
    btVector3 contactVelocity1;
    btScalar contactAngularVelocity1;
    
    if(ent1->getType() == ENTITY_STATIC)
    {
		StaticEntity* sent1 = (StaticEntity*)ent1;
        mat1 = sent1->getMaterial();
        contactVelocity1 = btVector3(0.,0.,0.);
        contactAngularVelocity1 = btScalar(0.);
    }
    else if(ent1->getType() == ENTITY_SOLID)
    {
        SolidEntity* sent1 = (SolidEntity*)ent1;
        mat1 = sent1->getMaterial();
        btVector3 localPoint1 = sent1->getTransform().getBasis() * cp.m_localPointB;
        contactVelocity1 = sent1->getLinearVelocityInLocalPoint(localPoint1);
        contactAngularVelocity1 = sent1->getAngularVelocity().dot(cp.m_normalWorldOnB);
    }
    else
    {
        cp.m_combinedFriction = btScalar(0.);
        cp.m_combinedRollingFriction = btScalar(0.);
        cp.m_combinedRestitution = btScalar(0.);
        return true;
    }
    
    btVector3 relLocalVel = contactVelocity1 - contactVelocity0;
    btVector3 normalVel = cp.m_normalWorldOnB * cp.m_normalWorldOnB.dot(relLocalVel);
    btVector3 slipVel = relLocalVel - normalVel;
    btScalar slipVelMod = slipVel.length();
    btScalar sigma = 100;
    // f = (static - dynamic)/(sigma * v^2 + 1) + dynamic
	Friction f = mm->GetMaterialsInteraction(mat0.name, mat1.name);
	cp.m_combinedFriction = (f.fStatic - f.fDynamic)/(sigma * slipVelMod * slipVelMod + btScalar(1)) + f.fDynamic;
	
    //Rolling friction not possible to generalize - needs special treatment
    cp.m_combinedRollingFriction = btScalar(0.);
    
    //Slipping
    if(SimulationApp::getApp()->getSimulationManager()->getCollisionFilter() == INCLUSIVE)
        cp.m_userPersistentData = (void *)(new btVector3(slipVel));
    
    //Damping angular velocity around contact normal (reduce spinning)
    //calculate relative angular velocity
    btScalar relAngularVelocity01 = contactAngularVelocity0 - contactAngularVelocity1;
    btScalar relAngularVelocity10 = contactAngularVelocity1 - contactAngularVelocity0;
    
    //calculate contact normal force and friction torque
    btScalar normalForce = cp.m_appliedImpulse * SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond();
    btScalar T = cp.m_combinedFriction * normalForce * 0.002;

    //apply damping torque
    if(ent0->getType() == ENTITY_SOLID && !btFuzzyZero(relAngularVelocity01))
        ((SolidEntity*)ent0)->ApplyTorque(cp.m_normalWorldOnB * relAngularVelocity01/btFabs(relAngularVelocity01) * T);
    
    if(ent1->getType() == ENTITY_SOLID && !btFuzzyZero(relAngularVelocity10))
        ((SolidEntity*)ent1)->ApplyTorque(cp.m_normalWorldOnB * relAngularVelocity10/btFabs(relAngularVelocity10) * T);
    
    //Restitution
    cp.m_combinedRestitution = mat0.restitution * mat1.restitution;
    
    //printf("%s <-> %s  R:%1.3lf F:%1.3lf\n", ent0->getName().c_str(), ent1->getName().c_str(), cp.m_combinedRestitution, cp.m_combinedFriction);
    
    return true;
}

void SimulationManager::SolveICTickCallback(btDynamicsWorld* world, btScalar timeStep)
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
        for(int i = 0; i < simManager->entities.size(); i++)
        {
            if(simManager->entities[i]->getType() == ENTITY_SOLID)
            {
                SolidEntity* solid = (SolidEntity*)simManager->entities[i];
                solid->ApplyGravity();
            }
            else if(simManager->entities[i]->getType() == ENTITY_FEATHERSTONE)
            {
                FeatherstoneEntity* feather = (FeatherstoneEntity*)simManager->entities[i];
                feather->ApplyGravity(world->getGravity());
            }
        }
        
        if(simManager->simulationTime < btScalar(0.01)) //Wait for a few cycles to ensure bodies started moving
            objectsSettled = false;
        else
        {
            //Check if objects settled
            for(int i = 0; i < simManager->entities.size(); i++)
            {
                if(simManager->entities[i]->getType() == ENTITY_SOLID)
                {
                    SolidEntity* solid = (SolidEntity*)simManager->entities[i];
                    if(solid->getLinearVelocity().length() > simManager->icLinTolerance * btScalar(100.) || solid->getAngularVelocity().length() > simManager->icAngTolerance * btScalar(100.))
                    {
                        objectsSettled = false;
                        break;
                    }
                }
                else if(simManager->entities[i]->getType() == ENTITY_FEATHERSTONE)
                {
                    FeatherstoneEntity* multibody = (FeatherstoneEntity*)simManager->entities[i];
                    
                    //Check base velocity
                    btVector3 baseLinVel = multibody->getLinkLinearVelocity(0);
                    btVector3 baseAngVel = multibody->getLinkAngularVelocity(0);
                    
                    if(baseLinVel.length() > simManager->icLinTolerance * btScalar(100.) || baseAngVel.length() > simManager->icAngTolerance * btScalar(100.0))
                    {
                        objectsSettled = false;
                        break;
                    }
                    
                    //Loop through all joints
                    for(int h = 0; h < multibody->getNumOfJoints(); h++)
                    {
                        btScalar jVelocity;
                        btMultibodyLink::eFeatherstoneJointType jType;
                        multibody->getJointVelocity(h, jVelocity, jType);
                        
                        switch(jType)
                        {
                            case btMultibodyLink::eRevolute:
                                if(UnitSystem::SetAngularVelocity(btVector3(jVelocity,0,0)).length() > simManager->icAngTolerance * btScalar(100.))
                                    objectsSettled = false;
                                break;
                                
                            case btMultibodyLink::ePrismatic:
                                if(UnitSystem::SetVelocity(btVector3(jVelocity,0,0)).length() > simManager->icLinTolerance * btScalar(100.))
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
    
    for(int i = 0; i < simManager->joints.size(); i++)
        if(!simManager->joints[i]->SolvePositionIC(simManager->icLinTolerance, simManager->icAngTolerance))
            jointsICSolved = false;

    //Check if everything solved
    if(objectsSettled && jointsICSolved)
        simManager->icProblemSolved = true;
    
    //Update time
    simManager->simulationTime += timeStep;
}

//Used to apply and accumulate forces
void SimulationManager::SimulationTickCallback(btDynamicsWorld* world, btScalar timeStep)
{
    SimulationManager* simManager = (SimulationManager*)world->getWorldUserInfo();
    btMultiBodyDynamicsWorld* mbDynamicsWorld = (btMultiBodyDynamicsWorld*)world;
    	
    //Clear all forces to ensure that no summing occurs
    mbDynamicsWorld->clearForces(); //Includes clearing of multibody forces!
    
    //loop through all actuators -> apply forces to bodies (free and connected by joints)
    for(unsigned int i = 0; i < simManager->actuators.size(); ++i)
        simManager->actuators[i]->Update(timeStep);
    
    //loop through all joints -> apply damping forces to bodies connected by joints
    for(unsigned int i = 0; i < simManager->joints.size(); ++i)
        simManager->joints[i]->ApplyDamping();
    
    //loop through all entities that may need special actions
    for(unsigned int i = 0; i < simManager->entities.size(); ++i)
    {
        Entity* ent = simManager->entities[i];
        
        if(ent->getType() == ENTITY_SOLID)
        {
            SolidEntity* solid = (SolidEntity*)ent;
            solid->ApplyGravity();
        }
        else if(ent->getType() == ENTITY_FEATHERSTONE)
        {
            FeatherstoneEntity* multibody = (FeatherstoneEntity*)ent;
            multibody->ApplyGravity(mbDynamicsWorld->getGravity());
            multibody->ApplyDamping();
        }
        else if(ent->getType() == ENTITY_CABLE)
        {
            CableEntity* cable = (CableEntity*)ent;
            cable->ApplyGravity();
        }
        else if(ent->getType() == ENTITY_SYSTEM)
        {
            SystemEntity* system = (SystemEntity*)ent;
            system->UpdateActuators(timeStep);
            system->ApplyGravity(mbDynamicsWorld->getGravity());
			system->ApplyDamping();
        }
    }
    
	//ocean forces
    if(simManager->ocean != NULL)
    {
        btBroadphasePairArray& pairArray = simManager->ocean->getGhost()->getOverlappingPairCache()->getOverlappingPairArray();
        int numPairs = pairArray.size();
        
        if(numPairs > 0)
        {    
            for(int h=0; h<numPairs; h++)
            {
                const btBroadphasePair& pair = pairArray[h];
                btBroadphasePair* colPair = world->getPairCache()->findPair(pair.m_pProxy0,pair.m_pProxy1);
                if (!colPair)
                    continue;
                    
                btCollisionObject* co1 = (btCollisionObject*)colPair->m_pProxy0->m_clientObject;
                btCollisionObject* co2 = (btCollisionObject*)colPair->m_pProxy1->m_clientObject;
                
                if(co1 == simManager->ocean->getGhost())
                    simManager->ocean->ApplyFluidForces(simManager->getHydrodynamicsType(), world, co2);
                else if(co2 == simManager->ocean->getGhost())
                    simManager->ocean->ApplyFluidForces(simManager->getHydrodynamicsType(), world, co1);
            }
        }
    }
}

//Used to measure body motions and calculate controls
void SimulationManager::SimulationPostTickCallback(btDynamicsWorld *world, btScalar timeStep)
{
    SimulationManager* simManager = (SimulationManager*)world->getWorldUserInfo();
	
	//Update acceleration data
    for(unsigned int i = 0 ; i < simManager->entities.size(); ++i)
    {
        Entity* ent = simManager->entities[i];
            
        if(ent->getType() == ENTITY_SOLID)
        {
            SolidEntity* solid = (SolidEntity*)ent;
            solid->UpdateAcceleration();
        }
		else if(ent->getType() == ENTITY_SYSTEM)
		{
			SystemEntity* sys = (SystemEntity*)ent;
			sys->UpdateAcceleration(timeStep);
		}
    }
	
	//loop through all sensors -> update measurements
    for(unsigned int i = 0; i < simManager->sensors.size(); ++i)
        simManager->sensors[i]->Update(timeStep);
    
    //loop through all controllers
    for(unsigned int i = 0; i < simManager->controllers.size(); ++i)
        simManager->controllers[i]->Update(timeStep);
    
	//loop through all entities that may need special actions
    for(unsigned int i = 0; i < simManager->entities.size(); ++i)
    {
        Entity* ent = simManager->entities[i];
        
        if(ent->getType() == ENTITY_SYSTEM)
        {
            SystemEntity* system = (SystemEntity*)ent;
            system->UpdateSensors(timeStep);
            system->UpdateControllers(timeStep);
        }
    }
	
    //Update simulation time
    simManager->simulationTime += timeStep;
}
