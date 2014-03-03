//
//  SimulationManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SimulationManager__
#define __Stonefish_SimulationManager__

#include "common.h"
#include "MaterialManager.h"
#include "Entity.h"
#include "SolidEntity.h"
#include "FluidEntity.h"
#include "Joint.h"
#include "Sensor.h"
#include "Actuator.h"
#include "Machine.h"
#include "OpenGLLight.h"
#include "OpenGLCamera.h"

class SimulationManager
{
public:
    SimulationManager(btScalar stepsPerSecond);
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
    
    void AddEntity(Entity* ent);
    void AddSolidEntity(SolidEntity* ent, const btTransform& worldTransform);
    void AddFluidEntity(FluidEntity* flu);
    void AddJoint(Joint* jnt);
    void AddActuator(Actuator* act);
    void AddSensor(Sensor* sens);
    void AddMachine(Machine* mach);
    
    Entity* getEntity(int index);
    Entity* getEntity(std::string name);
    Joint* getJoint(int index);
    Actuator* getActuator(int index);
    Sensor* getSensor(int index);
    Machine* getMachine(int index);
    
    void SetGravity(const btVector3& g);
    btDynamicsWorld* getDynamicsWorld();
    MaterialManager* getMaterialManager();
    
    //graphics
    void Render();
    void InitializeRendering();
    
    void AddView(OpenGLView* view);
    void RenderView(OpenGLView *view, const btTransform& viewTransform, bool ssao);
    OpenGLView* getView(int index);
    bool drawCameraDummies;
    
    void AddLight(OpenGLLight* light);
    OpenGLLight* getLight(int index);
    bool drawLightDummies;
    
private:
    //physics
    static void SimulationTickCallback(btDynamicsWorld *world, btScalar timeStep);
    
    btScalar sps;
    uint64_t currentTime;
    uint64_t physicTime;
    uint64_t ssus;
    
    std::vector<Entity*> entities;
    std::vector<FluidEntity*> fluids;
    std::vector<Joint*> joints;
    std::vector<Sensor*> sensors;
    std::vector<Actuator*> actuators;
    std::vector<Machine*> machines;
    
    btDynamicsWorld* dynamicsWorld;
    MaterialManager* materialManager;

    //graphics
    std::vector<OpenGLView*> views;
    std::vector<OpenGLLight*> lights;
};

#endif
