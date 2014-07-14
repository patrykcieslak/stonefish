//
//  UnderwaterTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterTestManager.h"

#include "UnderwaterTestApp.h"
#include "PlaneEntity.h"
#include "BoxEntity.h"
#include "SphereEntity.h"
#include "TorusEntity.h"
#include "CylinderEntity.h"
#include "OpenGLOmniLight.h"
#include "OpenGLSpotLight.h"
#include "OpenGLTrackball.h"
#include "SystemUtil.h"
#include "Accelerometer.h"
#include "ADC.h"
#include "PoolEntity.h"
#include "ObstacleEntity.h"

UnderwaterTestManager::UnderwaterTestManager(btScalar stepsPerSecond) : SimulationManager(MKS, false, stepsPerSecond, DANTZIG, STANDARD)
{
}

void UnderwaterTestManager::BuildScenario()
{
    //General
    OpenGLPipeline::getInstance()->setRenderingEffects(true, true, true, true);
    OpenGLPipeline::getInstance()->setVisibleHelpers(true, false, false, false, false, false, false);
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Steel", UnitSystem::Density(CGS, MKS, 7.81), 0.4);
    getMaterialManager()->SetMaterialsInteraction("Steel", "Steel", 0.9, 0.5);
    getMaterialManager()->CreateFluid("Water", UnitSystem::Density(CGS, MKS, 1.0), 0.1, 1.55);
    
    ///////LOOKS///////////
    char path[1024];
    GetDataPath(path, 1024-32);
    strcat(path, "grid.png");
    
    Look grey = CreateOpaqueLook(glm::vec3(1.0f, 1.0f, 1.0f), 0.5f, 0.3f , 1.5f, path);
    Look wall = CreateOpaqueLook(glm::vec3(0.9f, 0.9f, 0.9f), 0.5f, 0.8f, 1.5f);
    
    ////////OBJECTS
    PlaneEntity* floor = new PlaneEntity("Floor", 1000.f, getMaterialManager()->getMaterial("Steel"), grey, btTransform(btQuaternion(0,0,0), btVector3(0,0,0)));
    AddEntity(floor);
    
    GetDataPath(path, 1024-32);
    strcat(path, "tank.obj");
    
    ObstacleEntity* tank = new ObstacleEntity("Tank", path, 1.f, getMaterialManager()->getMaterial("Steel"), wall, btTransform(btQuaternion(0,0,-M_PI), btVector3(0,0,-5.5f)), true);
    //tank->SetWireframe(true);
    AddEntity(tank);
    
    PoolEntity* pool = new PoolEntity("Pool", 7.f, 9.f, btTransform(btQuaternion::getIdentity(), btVector3(0,0,-4.5)), getMaterialManager()->getFluid("Water"));
    pool->setRenderable(true);
    SetFluidEntity(pool);
    
    Look color = CreateOpaqueLook(glm::vec3(1.f, 0.6f, 0.2f), 0.5f, 0.5f, 1.5f);
    BoxEntity* box = new BoxEntity("Box1", btVector3(0.5f,0.5f,15.0f), getMaterialManager()->getMaterial("Steel"), color);
    AddSolidEntity(box, btTransform(btQuaternion(0,0,0.1), btVector3(0,0,-5)));
    
    //////CAMERA & LIGHT//////
    //OpenGLOmniLight* omni = new OpenGLOmniLight(btVector3(5000.f, 5000.f, 10000.f), OpenGLLight::ColorFromTemperature(4500, 1000));
    //AddLight(omni);
    //OpenGLSpotLight* spot = new OpenGLSpotLight(btVector3(0.f, -10.f, -40.f), btVector3(0.f,20.f,0.f), 40.f, OpenGLLight::ColorFromTemperature(4500, 10000));
    //AddLight(spot);
    //spot = new OpenGLSpotLight(btVector3(10000.f, -12000.f, 5000.f), btVector3(5000.f,-5000.f,0.f), 30.f, OpenGLLight::ColorFromTemperature(5600, 500));
    //AddLight(spot);
    
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0, 0.f, -12.f), 20.f, btVector3(0, 0, -1.f), 0, 0, UnderwaterTestApp::getApp()->getWindowWidth(), UnderwaterTestApp::getApp()->getWindowHeight(), 60.f, 500.f, false);
    //trackb->Rotate(btQuaternion(M_PI, 0, M_PI/8.0));
    trackb->Activate();
    AddView(trackb);
}