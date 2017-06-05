//
//  UnderwaterTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright(c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterTestManager.h"

#include "UnderwaterTestApp.h"
#include "PlaneEntity.h"
#include "MeshEntity.h"
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
#include "SeaEntity.h"
#include "ObstacleEntity.h"
#include "UnderwaterVehicle.h"

UnderwaterTestManager::UnderwaterTestManager(btScalar stepsPerSecond) : SimulationManager(MKS, false, stepsPerSecond, DANTZIG, STANDARD)
{
}

void UnderwaterTestManager::BuildScenario()
{
    //General
    OpenGLPipeline::getInstance()->setRenderingEffects(true, true, true, false);
    OpenGLPipeline::getInstance()->setVisibleHelpers(true, false, false, false, false, false, false);
    //OpenGLPipeline::getInstance()->setDebugSimulation(true);
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Concrete", UnitSystem::Density(CGS, MKS, 1.1), 0.4);
    getMaterialManager()->CreateMaterial("Cork", UnitSystem::Density(CGS, MKS, 0.9), 0.2);
    getMaterialManager()->CreateFluid("Water", UnitSystem::Density(CGS, MKS, 1.0), 1.308e-3, 1.55);
    
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Concrete", 0.9, 0.5);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Cork", 0.9, 0.7);
    getMaterialManager()->SetMaterialsInteraction("Cork", "Cork", 0.9, 0.7);
    
    ///////LOOKS///////////
    Look yellow = CreateOpaqueLook(glm::vec3(1.f, 0.6f, 0.2f), 0.5f, 0.5f, 1.5f);
    Look green = CreateOpaqueLook(glm::vec3(0.3f, 1.0f, 0.2f), 0.5f, 0.5f, 1.5f);
    
    ////////OBJECTS
    SeaEntity* sea = new SeaEntity("Sea", getMaterialManager()->getFluid("Water"));
    sea->setRenderable(true);
    SetFluidEntity(sea);
    
    //Setup model paths
    char path[1024];
    GetDataPath(path, 1024-32);
    strcat(path,"icosphere.obj");
    
    //Reference solid
    MeshEntity* hull1 = new MeshEntity("Solid", path, btScalar(1.), getMaterialManager()->getMaterial("Cork"), green);
    //AddSolidEntity(hull1, btTransform(btQuaternion::getIdentity(), btVector3(0.0,0.0,-2.0)));

    //Vehicle
    MeshEntity* hull2 = new MeshEntity("Solid", path, btScalar(1.), getMaterialManager()->getMaterial("Concrete"), yellow);
    
    UnderwaterVehicle* rov = new UnderwaterVehicle("ROV");
    rov->AddExternalPart(hull2, btTransform(btQuaternion::getIdentity(), btVector3(0,0,-1.0)));
    rov->AddExternalPart(hull1, btTransform(btQuaternion::getIdentity(), btVector3(0,-0.7,0)));
    rov->AddExternalPart(hull1, btTransform(btQuaternion::getIdentity(), btVector3(0,0.7,0)));
    //rov->AddInternalPart(hull1, btTransform(btQuaternion::getIdentity(), btVector3(0,0.0,1.0)));
    AddSystemEntity(rov, btTransform(btQuaternion(0.1,0,0), btVector3(0,0,0)));
    
    //////CAMERA & LIGHT//////
    //OpenGLOmniLight* omni = new OpenGLOmniLight(btVector3(50.f, 50.f, 50.f), OpenGLLight::ColorFromTemperature(4500, 1000));
    //AddLight(omni);
    //OpenGLSpotLight* spot = new OpenGLSpotLight(btVector3(0.f, -10.f, -40.f), btVector3(0.f,20.f,0.f), 40.f, OpenGLLight::ColorFromTemperature(4500, 10000));
    //AddLight(spot);
    //spot = new OpenGLSpotLight(btVector3(10000.f, -12000.f, 5000.f), btVector3(5000.f,-5000.f,0.f), 30.f, OpenGLLight::ColorFromTemperature(5600, 500));
    //AddLight(spot);
    
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0,0,0), 10.f, btVector3(0,0,-1), 0, 0, UnderwaterTestApp::getApp()->getWindowWidth(), UnderwaterTestApp::getApp()->getWindowHeight(), 60.f, 500.f, false);
    //trackb->Rotate(btQuaternion(0,0,-M_PI/8.0) * btQuaternion(M_PI/6,0,0));
    trackb->Rotate(btQuaternion(0,0, M_PI/12.0) * btQuaternion(M_PI*0.4,0,0));
    trackb->Activate();
    AddView(trackb);
}
