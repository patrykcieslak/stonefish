//
//  SimulationManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "SimulationManager.h"

#include <BulletDynamics/MLCPSolvers/btDantzigSolver.h>
#include <BulletDynamics/MLCPSolvers/btSolveProjectedGaussSeidel.h>
#include <BulletDynamics/MLCPSolvers/btMLCPSolver.h>

#include "SimulationApp.h"
#include "OpenGLSolids.h"
#include "OpenGLUtil.h"
#include "OpenGLSky.h"
#include "OpenGLOmniLight.h"
#include "OpenGLSpotLight.h"
#include "OpenGLTrackball.h"
#include "BoxEntity.h"
#include "PlaneEntity.h"
#include "CableEntity.h"
#include "GhostEntity.h"
#include "FluidEntity.h"
#include "CompoundEntity.h"

SimulationManager::SimulationManager(UnitSystems unitSystem, bool zAxisUp, btScalar stepsPerSecond)
{
    UnitSystem::SetUnitSystem(unitSystem, false);
    zUp = zAxisUp;
    setStepsPerSecond(stepsPerSecond);
    
    currentTime = 0;
    physicTime = 0;
    drawLightDummies = false;
    drawCameraDummies = false;
}

SimulationManager::~SimulationManager(void)
{
    DestroyScenario();
    
    OpenGLLight::Destroy();
    OpenGLSky::Destroy();
    OpenGLSky::Destroy();
    OpenGLGBuffer::DeleteShaders();
}

void SimulationManager::InitializeRendering()
{
    OpenGLGBuffer::LoadShaders();
    OpenGLView::Init();
    OpenGLSky::Init();
    OpenGLSky::Generate(10.f,0.f);
    OpenGLLight::Init();
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

void SimulationManager::AddMachine(Machine *mach)
{
    if(mach != NULL)
        machines.push_back(mach);
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

Machine* SimulationManager::getMachine(int index)
{
    if(index < machines.size())
        return machines[index];
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

void SimulationManager::BuildScenario()
{
    //initialize dynamic world
    dwCollisionConfig = new btSoftBodyRigidBodyCollisionConfiguration(); //dwCollisionConfig = new btDefaultCollisionConfiguration();
    dwDispatcher = new btCollisionDispatcher(dwCollisionConfig); //btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);
    dwBroadphase = new btDbvtBroadphase();
    //dwSolver = new btSequentialImpulseConstraintSolver();
    btDantzigSolver* mlcp = new btDantzigSolver();
    //btSolveProjectedGaussSeidel* mlcp = new btSolveProjectedGaussSeidel();
    dwSolver = new btMLCPSolver(mlcp);
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
    dynamicsWorld->getSolverInfo().m_solverMode = SOLVER_ENABLE_FRICTION_DIRECTION_CACHING | SOLVER_USE_2_FRICTION_DIRECTIONS | SOLVER_SIMD | SOLVER_USE_WARMSTARTING | SOLVER_RANDMIZE_ORDER;
    dynamicsWorld->getSolverInfo().m_tau = 0.8;
    dynamicsWorld->getSolverInfo().m_damping = GLOBAL_DAMPING;
    dynamicsWorld->getSolverInfo().m_friction = GLOBAL_FRICTION;
    dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize = 64;
    dynamicsWorld->getSolverInfo().m_maxGyroscopicForce = 10e6;
    dynamicsWorld->getSolverInfo().m_singleAxisRollingFrictionThreshold = 100.f;
    dynamicsWorld->getSolverInfo().m_numIterations = 50;
    dynamicsWorld->getSolverInfo().m_timeStep = btScalar(1.f/getStepsPerSecond());
    
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
    
    for(int i=0; i<sensors.size(); i++)
        delete sensors[i];
    sensors.clear();
    
    for(int i=0; i<actuators.size(); i++)
        delete actuators[i];
    actuators.clear();
    
    for(int i=0; i<machines.size(); i++)
        delete machines[i];
    machines.clear();
    
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
    
    int substeps = 30000;
    
    physicTime = GetTimeInMicroseconds();
    dynamicsWorld->stepSimulation((btScalar)deltaTime/btScalar(1000000.0), substeps, (btScalar)ssus/btScalar(1000000.0));
    simulationTime += (btScalar)deltaTime/btScalar(1000000.0);
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
    dynamicsWorld->synchronizeMotionStates();
}

void SimulationManager::StopSimulation()
{
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

///////////////////////////////
/////3D RENDERING PIPELINE/////
///////////////////////////////

void SimulationManager::Render()
{
    //Move lights attached to rigid bodies
    for(int i=0; i<lights.size(); i++)
    {
        lights[i]->UpdateLight();
            //render shadowmaps
    }
    
    //Render all camera views
    for(int i=0; i<views.size(); i++)
    {
        if(views[i]->isActive()) //Render only if active
        {
            GLint* viewport = views[i]->GetViewport();
            views[i]->SetViewport();
            OpenGLLight::SetCamera(views[i]);
            
            //Fill normal gbuffer
            views[i]->getGBuffer()->Start(0);
            views[i]->SetProjection();
            views[i]->SetViewTransform();
            
            //render simple entities
            for(int h=0; h<entities.size(); h++)
                entities[h]->Render();
            
            //render machines
            for(int h=0; h<machines.size(); h++)
                machines[h]->Render();
            
            //render dummies, icons, joints
            for(int h=0; h<joints.size(); h++)
                if(joints[h]->isRenderable())
                    joints[h]->Render();
            
            views[i]->getGBuffer()->Stop();
            
            //Render scene
            //SSAO
            glActiveTextureARB(GL_TEXTURE0_ARB);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, views[i]->getGBuffer()->getNormalsTexture(0));
            
            glActiveTextureARB(GL_TEXTURE1_ARB);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, OpenGLView::getRandomTexture());
            
            OpenGLView::SetTextureUnits(0, 1);
            views[i]->RenderSSAO();
            
            //Scene
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, views[i]->getSceneFBO());
            glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            RenderView(views[i], views[i]->GetViewTransform(), true);
            //Render dummies
            //Render water,fog, etc.
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
            
            //Render water surface
            for(int h=0; h<fluids.size(); h++)
            {
                btVector3 surfN, surfP;
                fluids[h]->GetSurface(surfN, surfP);
            
                //Fill reflection gbuffer
                views[i]->getGBuffer()->Start(1);
                views[i]->SetProjection();
                views[i]->SetReflectedViewTransform(fluids[h]);

                for(int j=0; j<entities.size(); j++) //render simple entities
                    entities[j]->Render();
                
                for(int j=0; j<machines.size(); j++) //render machines
                    machines[j]->Render();
                
                views[i]->getGBuffer()->Stop();
            
                //Render reflection map
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, views[i]->getSceneFBO());
                glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                RenderView(views[i], views[i]->GetReflectedViewTransform(surfN, surfP), false);
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                
                //Fill refraction gbuffer
                /*views[i]->getGBuffer()->Start(1);
                views[i]->SetProjection();
                views[i]->SetRefractedViewTransform(fluids[h]);
                
                for(int j=0; j<entities.size(); j++) //render simple entities
                    entities[j]->Render();
                
                for(int j=0; j<machines.size(); j++) //render machines
                    machines[j]->Render();
                
                views[i]->getGBuffer()->Stop();
                
                //Render refraction map
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, views[i]->getSceneFBO());
                glDrawBuffer(GL_COLOR_ATTACHMENT2_EXT);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                RenderView(views[i], views[i]->GetRefractedViewTransform(surfN, surfP, fluids[h]->getFluid()) ,false);
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);*/
                
                /////////Render fluid surface
                
                /* glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                glEnable(GL_STENCIL_TEST);
                glStencilFunc(GL_ALWAYS, 1, 1);
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                glDisable(GL_DEPTH_TEST);
                
                fluids[i]->Render();
                
                glEnable(GL_DEPTH_TEST);
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                glStencilFunc(GL_EQUAL, 1, 1);
                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                */
                
                //Water surface shader
                //DrawScreenAlignedQuad();*/
                
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_CULL_FACE);
                glDisable(GL_BLEND);
                //glEnable(GL_BLEND);
                //glBlendEquation(GL_FUNC_);
                //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, views[i]->getSceneFBO());
                glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
                views[i]->RenderFluid(fluids[h]);
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
            }
        
            
            //////////FINAL RENDER///////
            glEnable(GL_SCISSOR_TEST);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            
            glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
            glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
            delete viewport;
            
            glActiveTextureARB(GL_TEXTURE0_ARB);
            glBindTexture(GL_TEXTURE_2D, views[i]->getSceneTexture());
            SetupOrtho();
            DrawScreenAlignedQuad(); //introduce distortion?
            
            glDisable(GL_SCISSOR_TEST);
            
            //Debugging
            //OpenGLSky::ShowCubemap(CONVOLUTION_DIFFUSE, 0, 0, 400, 400);
            //OpenGLSky::ShowCubemap(CONVOLUTION_REFLECT, 400, 0, 400, 400);
            //views[i]->getGBuffer()->ShowTexture(POSITION1, 0,0,250,200); // FBO debugging
            //views[i]->getGBuffer()->ShowTexture(POSITION2, 0,200,250,200); // FBO debugging
        }
    }
}

void SimulationManager::RenderView(OpenGLView *view, const btTransform& viewTransform, bool ssao)
{
    //1. Enter deferred rendering
    SetupOrtho();
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    //glClearColor(0.f, 0.f, 0.f, 1.f);
    //glClear(GL_COLOR_BUFFER_BIT);
    
    //2. Draw sky
    OpenGLSky::Render(view, viewTransform, zUp);
    
    //3. Bind deferred textures to texture units
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, view->getGBuffer()->getDiffuseTexture());
    
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, view->getGBuffer()->getPositionTexture(ssao ? 0 : 1));
    
    glActiveTextureARB(GL_TEXTURE2_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, view->getGBuffer()->getNormalsTexture(ssao ? 0 : 1));
    
    glActiveTextureARB(GL_TEXTURE3_ARB);
    glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, OpenGLSky::getDiffuseCubemap());
    
    glActiveTextureARB(GL_TEXTURE4_ARB);
    glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, OpenGLSky::getReflectionCubemap());
    
    //4. Bind SSAO texture if needed
    if(ssao)
    {
        glActiveTextureARB(GL_TEXTURE5_ARB);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, view->getSSAOTexture());
    }
    
    OpenGLLight::SetTextureUnits(0, 2, 1, 3, 4, 5);
    
    //5. Render ambient pass - sky, sun, ssao
    OpenGLLight::UseAmbientShader(viewTransform, zUp);
    DrawScreenAlignedQuad();
    
    //6. Render light passes
    glBlendFunc(GL_ONE, GL_ONE); //accumulate light
    
    for(int h=0; h<lights.size(); h++)
    {
        if(lights[h]->isActive()) //If light is active
        {
            lights[h]->Render();
            DrawScreenAlignedQuad();
        }
    }
    
    //7. Reset OpenGL
    glUseProgramObjectARB(0);
    
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glActiveTextureARB(GL_TEXTURE2_ARB);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glActiveTextureARB(GL_TEXTURE3_ARB);
    glDisable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    glActiveTextureARB(GL_TEXTURE4_ARB);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glDisable(GL_TEXTURE_CUBE_MAP);
    
    glActiveTextureARB(GL_TEXTURE5_ARB);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

///////////////////////////////
///////////////////////////////
///////////////////////////////


//static members
void SimulationManager::SimulationTickCallback(btDynamicsWorld *world, btScalar timeStep)
{
    SimulationManager* simManager = (SimulationManager*)world->getWorldUserInfo();
    
    //clear all forces to ensure that no summing occurs
    world->clearForces();
    
    //btJointFeedback* fb = simManager->getJoint(0)->getConstraint()->getJointFeedback();
    //printf("%f\n", fb->m_appliedForceBodyA.getX());
    
    //loop through all sensors
    for(int i=0; i<simManager->sensors.size(); i++)
        simManager->sensors[i]->Update(timeStep);

    //loop through all machines
    /*for(int i=0; i<simManager->machines.size(); i++)
    {
        //SimulationManager->machines[i]->Update(timeStep);
        simManager->machines[i]->ApplyForces();
    }*/
    
    //loop through all controllers
    
    
    //loop through all actuators
    for(int i=0; i< simManager->actuators.size(); i++)
        simManager->actuators[i]->Update(timeStep);
    
    //loop through all entities that may need special actions
    for(int i=0; i<simManager->entities.size(); i++)
    {
        Entity* ent = simManager->entities[i];
        
        //apply gravity only to dynamic objects
        if(ent->getType() == SOLID)
        {
            SolidEntity* simple = (SolidEntity*)ent;
            simple->ApplyGravity();
            simple->getRigidBody()->setDamping(0, 0);
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
