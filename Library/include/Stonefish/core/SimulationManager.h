//
//  SimulationManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SimulationManager__
#define __Stonefish_SimulationManager__

//Engine core
#include <core/ResearchDynamicsWorld.h>
#include <core/ResearchConstraintSolver.h>
#include <core/UnitSystem.h>
#include <core/NameManager.h>
#include <core/MaterialManager.h>

//Entities
#include <entities/Entity.h>
#include <entities/SolidEntity.h>
#include <entities/FeatherstoneEntity.h>
#include <entities/SystemEntity.h>
#include <entities/forcefields/Ocean.h>

//Dynamic elements
#include <joints/Joint.h>
#include <actuators/Actuator.h>
#include <sensors/Sensor.h>
#include <sensors/Contact.h>
#include <controllers/Controller.h>

//Graphics, debugging and logging
#include <graphics/OpenGLTrackball.h>
#include <graphics/OpenGLAtmosphere.h>
#include <graphics/OpenGLDebugDrawer.h>
#include <graphics/Console.h>

//Simulation algorithm settings
typedef enum {SI, DANTZIG, PROJ_GAUSS_SIEDEL, LEMKE, NNCG} SolverType;
typedef enum {INCLUSIVE, EXCLUSIVE} CollisionFilteringType;

typedef struct 
{
    Entity* A;
    Entity* B;
}
Collision;

/*! 
    An abstract class managing all of the simulated entities, solver settings and equipped with custom callbacks.
 */
class SimulationManager
{
    friend class OpenGLPipeline;
    
public:
    SimulationManager(UnitSystems unitSystem = MKS, bool zAxisUp = false, btScalar stepsPerSecond = btScalar(60), SolverType st = SI, CollisionFilteringType cft = EXCLUSIVE, HydrodynamicsType ht = GEOMETRY_BASED);
	virtual ~SimulationManager(void);
    
    //physics
    virtual void BuildScenario() = 0;
    virtual void SimulationStepCompleted();
    bool SolveICProblem();
    void DestroyScenario();
    void RestartScenario();
    bool StartSimulation();
    void ResumeSimulation();
    void StopSimulation();
    void AdvanceSimulation();
    void UpdateDrawingQueue();
	
    void EnableOcean(Liquid* f = NULL);
	void AddEntity(Entity* ent);
    void AddStaticEntity(StaticEntity* ent, const btTransform& worldTransform);
    void AddSolidEntity(SolidEntity* ent, const btTransform& worldTransform);
    void AddFeatherstoneEntity(FeatherstoneEntity* ent, const btTransform& worldTransform);
    void AddSystemEntity(SystemEntity* ent, const btTransform& worldTransform);
    void AddJoint(Joint* jnt);
    void AddActuator(Actuator* act);
    void AddSensor(Sensor* sens);
    void AddController(Controller* cntrl);
    Contact* AddContact(Entity* entA, Entity* entB, size_type contactHistoryLength = 1);
    void EnableCollision(Entity* entA, Entity* entB);
    void DisableCollision(Entity* entA, Entity* entB);
    
	Entity* PickEntity(int x, int y);
    int CheckCollision(Entity* entA, Entity* entB);

    btScalar getPhysicsTimeInMiliseconds();
    void setStepsPerSecond(btScalar steps);
    void setICSolverParams(bool useGravity, btScalar timeStep = btScalar(0.001), unsigned int maxIterations = 100000, btScalar maxTime = BT_LARGE_FLOAT, btScalar linearTolerance = btScalar(1e-6), btScalar angularTolerance = btScalar(1e-6));
    btScalar getStepsPerSecond();
    void getWorldAABB(btVector3& min, btVector3& max);
    CollisionFilteringType getCollisionFilter();
    SolverType getSolverType();
	HydrodynamicsType getHydrodynamicsType();
    
    Entity* getEntity(unsigned int index);
    Entity* getEntity(std::string name);
    Joint* getJoint(unsigned int index);
    Joint* getJoint(std::string name);
    Contact* getContact(unsigned int index);
    Contact* getContact(Entity* entA, Entity* entB);
    Actuator* getActuator(unsigned int index);
    Actuator* getActuator(std::string name);
    Sensor* getSensor(unsigned int index);
    Sensor* getSensor(std::string name);
    Controller* getController(unsigned int index);
    Controller* getController(std::string name);
	Ocean* getOcean();
    
    void setGravity(btScalar gravityConstant);
    btVector3 getGravity();
    btMultiBodyDynamicsWorld* getDynamicsWorld();
    btScalar getSimulationTime();
	btScalar getRealtimeFactor();
    MaterialManager* getMaterialManager();
	NameManager* getNameManager();
    OpenGLTrackball* getTrackball();
    bool isZAxisUp();
    bool isSimulationFresh();
    bool drawCameraDummies;
    bool drawLightDummies;
    
protected:
    static void SolveICTickCallback(btDynamicsWorld* world, btScalar timeStep);
    static void SimulationTickCallback(btDynamicsWorld* world, btScalar timeStep);
    static void SimulationPostTickCallback(btDynamicsWorld* world, btScalar timeStep);
    static bool CustomMaterialCombinerCallback(btManifoldPoint& cp,	const btCollisionObjectWrapper* colObj0Wrap,int partId0,int index0,const btCollisionObjectWrapper* colObj1Wrap,int partId1,int index1);
    
    btMultiBodyDynamicsWorld* dynamicsWorld;
	btMultiBodyConstraintSolver* dwSolver;
    btCollisionDispatcher* dwDispatcher;
    btBroadphaseInterface* dwBroadphase;
    btDefaultCollisionConfiguration* dwCollisionConfig;
    
    MaterialManager* materialManager;
    
private:
    void InitializeSolver();
    void InitializeScenario();
	
	SolverType solver;
    CollisionFilteringType collisionFilter;
	HydrodynamicsType hydroType;
	btScalar sps;
	btScalar realtimeFactor;
    unsigned int hydroPrescaler;
    unsigned int hydroCounter;
    SDL_mutex* simSettingsMutex;
    SDL_mutex* simInfoMutex;
    
	btScalar simulationTime;
    uint64_t currentTime;
    uint64_t physicsTime;
    uint64_t ssus;
	bool icUseGravity;
	btScalar icTimeStep;
    unsigned int icMaxIter;
    btScalar icMaxTime;
    btScalar icLinTolerance;
    btScalar icAngTolerance;
    unsigned int mlcpFallbacks;
    bool icProblemSolved;
	bool simulationFresh;
    
	NameManager* nameManager;
    std::vector<Entity*> entities;
    std::vector<Joint*> joints;
    std::vector<Sensor*> sensors;
    std::vector<Actuator*> actuators;
    std::vector<Controller*> controllers;
    std::vector<Contact*> contacts;
    std::vector<Collision> collisions;
    Ocean* ocean;
    btScalar g;
    bool zUp;

    //graphics
    OpenGLTrackball* trackball;
    OpenGLDebugDrawer* debugDrawer;
};

#endif
