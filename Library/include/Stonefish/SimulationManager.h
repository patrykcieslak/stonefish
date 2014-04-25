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
#include "Sensor.h"
#include "Actuator.h"
#include "OpenGLLight.h"
#include "OpenGLCamera.h"

#define USE_CONTINUOUS_COLLISION false
#define ALLOWED_CCD_PENETRATION 0.001
#define MAX_ERROR_REDUCTION 100.0
#define GLOBAL_ERP 0.2
#define GLOBAL_ERP2 0.8
#define GLOBAL_DAMPING 0.1
#define GLOBAL_FRICTION 0.0

typedef struct
{
    uint type;
    btVector3 location;
} Sticker;

class SimulationManager //abstract class!
{
    friend class OpenGLPipeline;
    
public:
    SimulationManager(UnitSystems unitSystem, bool zAxisUp, btScalar stepsPerSecond);
	virtual ~SimulationManager(void);
    
    //physics
    virtual void BuildScenario();
    void DestroyScenario();
    void RestartScenario();
    void StartSimulation();
    void AdvanceSimulation(uint64_t timeInMicroseconds);
    void StopSimulation();
	double getPhysicsTimeInMiliseconds();
    void setStepsPerSecond(btScalar steps);
    btScalar getStepsPerSecond();
    void getWorldAABB(btVector3& min, btVector3& max);
    void AddEntity(Entity* ent);
    void AddSolidEntity(SolidEntity* ent, const btTransform& worldTransform);
    void AddFluidEntity(FluidEntity* flu);
    void AddJoint(Joint* jnt);
    void AddActuator(Actuator* act);
    void AddSensor(Sensor* sens);
    
    Entity* getEntity(int index);
    Entity* getEntity(std::string name);
    Joint* getJoint(int index);
    Actuator* getActuator(int index);
    Sensor* getSensor(int index);
    
    void setGravity(const btVector3& g);
    btDynamicsWorld* getDynamicsWorld();
    btScalar getSimulationTime();
    MaterialManager* getMaterialManager();
    
    void AddView(OpenGLView* view);
    OpenGLView* getView(int index);
    bool drawCameraDummies;
    
    void AddLight(OpenGLLight* light);
    OpenGLLight* getLight(int index);
    bool drawLightDummies;
    
protected:
    static void SimulationTickCallback(btDynamicsWorld *world, btScalar timeStep);
    static bool CustomMaterialCombinerCallback(btManifoldPoint& cp,	const btCollisionObjectWrapper* colObj0Wrap,int partId0,int index0,const btCollisionObjectWrapper* colObj1Wrap,int partId1,int index1);
    
    btDynamicsWorld* dynamicsWorld;
    btBroadphaseInterface* dwBroadphase;
    btDefaultCollisionConfiguration* dwCollisionConfig;
    btCollisionDispatcher* dwDispatcher;
    btConstraintSolver* dwSolver;
    MaterialManager* materialManager;
    
private:
    //physics
    btScalar sps;
    btScalar simulationTime;
    uint64_t currentTime;
    uint64_t physicTime;
    uint64_t ssus;

    std::vector<Entity*> entities;
    std::vector<FluidEntity*> fluids;
    std::vector<Joint*> joints;
    std::vector<Sensor*> sensors;
    std::vector<Actuator*> actuators;
    
    bool zUp;

    //graphics
    std::vector<OpenGLView*> views;
    std::vector<OpenGLLight*> lights;
};

#endif
