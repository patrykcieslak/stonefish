//
//  SimulationManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SimulationManager__
#define __Stonefish_SimulationManager__

#include "UnitSystem.h"
#include "NameManager.h"
#include "MaterialManager.h"
#include "Entity.h"
#include "SolidEntity.h"
#include "FluidEntity.h"
#include "Joint.h"
#include "Contact.h"
#include "Sensor.h"
#include "Actuator.h"
#include "Controller.h"
#include "OpenGLLight.h"
#include "OpenGLCamera.h"
#include "Console.h"
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>

typedef enum {SEQUENTIAL_IMPULSE, DANTZIG, PROJ_GAUSS_SIEDEL, LEMKE} SolverType;
typedef enum {STANDARD, INCLUSIVE, EXCLUSIVE} CollisionFilteringType;

typedef struct
{
    uint type;
    btVector3 location;
} Sticker;

//abstract class
class SimulationManager
{
    friend class OpenGLPipeline;
    
public:
    SimulationManager(UnitSystems unitSystem, bool zAxisUp, btScalar stepsPerSecond, SolverType st = DANTZIG, CollisionFilteringType cft = STANDARD);
	virtual ~SimulationManager(void);
    
    //physics
    virtual void BuildScenario() = 0;
    void DestroyScenario();
    void RestartScenario();
    void StartSimulation();
    void AdvanceSimulation(uint64_t timeInMicroseconds);
    void StopSimulation();
	
    void AddEntity(Entity* ent);
    void AddSolidEntity(SolidEntity* ent, const btTransform& worldTransform);
    void AddJoint(Joint* jnt);
    void AddActuator(Actuator* act);
    void AddSensor(Sensor* sens);
    void AddController(Controller* cntrl);
    Contact* AddContact(Entity* e0, Entity* e1, unsigned int contactHistoryLength = 0);
    void SetFluidEntity(FluidEntity* flu);
    bool CheckContact(Entity* e0, Entity* e1);
    Entity* PickEntity(int x, int y);
    
    double getPhysicsTimeInMiliseconds();
    void setStepsPerSecond(btScalar steps);
    btScalar getStepsPerSecond();
    void getWorldAABB(btVector3& min, btVector3& max);
    CollisionFilteringType getCollisionFilter();
    SolverType getSolverType();
    Entity* getEntity(int index);
    Entity* getEntity(std::string name);
    Joint* getJoint(int index);
    Actuator* getActuator(int index);
    Sensor* getSensor(int index);
    Controller* getController(int index);
    Contact* getContact(Entity* e0, Entity* e1);
    
    void setGravity(const btVector3& g);
    btSoftRigidDynamicsWorld* getDynamicsWorld();
    btScalar getSimulationTime();
    MaterialManager* getMaterialManager();
    bool isZAxisUp();
    
    void AddView(OpenGLView* view);
    OpenGLView* getView(int index);
    bool drawCameraDummies;
    
    void AddLight(OpenGLLight* light);
    OpenGLLight* getLight(int index);
    bool drawLightDummies;
    
protected:
    static void SimulationTickCallback(btDynamicsWorld *world, btScalar timeStep);
    static bool CustomMaterialCombinerCallback(btManifoldPoint& cp,	const btCollisionObjectWrapper* colObj0Wrap,int partId0,int index0,const btCollisionObjectWrapper* colObj1Wrap,int partId1,int index1);
    
    btSoftRigidDynamicsWorld* dynamicsWorld;
    btBroadphaseInterface* dwBroadphase;
    btDefaultCollisionConfiguration* dwCollisionConfig;
    btCollisionDispatcher* dwDispatcher;
    btConstraintSolver* dwSolver;
    MaterialManager* materialManager;
    
private:
    //physics
    void InitializeSolver(SolverType st, CollisionFilteringType cft);
    
    btScalar sps;
    btScalar simulationTime;
    uint64_t currentTime;
    uint64_t physicTime;
    uint64_t ssus;
    SolverType solver;
    CollisionFilteringType collisionFilter;

    std::vector<Entity*> entities;
    std::vector<Joint*> joints;
    std::vector<Sensor*> sensors;
    std::vector<Actuator*> actuators;
    std::vector<Controller*> controllers;
    std::vector<Contact*> contacts;
    FluidEntity* fluid;
    
    bool zUp;

    //graphics
    std::vector<OpenGLView*> views;
    std::vector<OpenGLLight*> lights;
};

#endif
