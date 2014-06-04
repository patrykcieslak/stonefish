//
//  FallingTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "FallingTestManager.h"

#include "FallingTestApp.h"
#include "PlaneEntity.h"
#include "ObstacleEntity.h"
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
#include "Trajectory.h"

FallingTestManager::FallingTestManager(btScalar stepsPerSecond) : SimulationManager(MKS, true, stepsPerSecond, DANTZIG, STANDARD)
{
}

void FallingTestManager::BuildScenario()
{
    OpenGLPipeline::getInstance()->SetVisibleElements(true, false, false, false, false);
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Ground", 1000.0, 1.0);
    getMaterialManager()->CreateMaterial("Steel", 1000.0, 0.5);
    getMaterialManager()->SetMaterialsInteraction("Ground", "Ground", 0.5, 0.3);
    getMaterialManager()->SetMaterialsInteraction("Ground", "Steel", 0.5, 0.3);
    getMaterialManager()->SetMaterialsInteraction("Steel", "Steel", 0.5, 0.3);
    
    ///////LOOKS///////////
    char path[1024];
    GetDataPath(path, 1024-32);
    strcat(path, "grid.png");
    
    Look grey = CreateMatteLook(1.0f, 1.0f, 1.0f, 0.0f, path);
    //Look color;
    
    ////////OBJECTS
    PlaneEntity* floor = new PlaneEntity("Floor", 100.f, getMaterialManager()->getMaterial("Ground"), grey, btTransform(btQuaternion(0,0,0), btVector3(0,0,0)));
    AddEntity(floor);
    
    //GetDataPath(path, 1024-32);
    //strcat(path, "cone.obj");
    //MeshEntity* mesh = new MeshEntity("Cone", path, 1000.0, getMaterialManager()->getMaterial("Steel"), grey, true);
    //AddSolidEntity(mesh, btTransform(btQuaternion(0,0,0), btVector3(2000.f, 0.f, 0.f)));
    //mesh->SetArbitraryPhysicalProperties(1000, btVector3(10000000.0,10000000.0,10000000.0), btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)));
    
    //ObstacleEntity* terrain = new ObstacleEntity("Terrain", path, 10000.f, getMaterialManager()->getMaterial("Steel"), grey, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0,0,0)), false);
    //AddEntity(terrain);
    
    Look color = CreateGlossyLook(1.f, 0.6f, 0.2f, 0.5f, 0.1f);
    
    SphereEntity* sphere = new SphereEntity("Sphere", 1.f, getMaterialManager()->getMaterial("Steel"), color);
    sphere->setRenderable(true);
    AddSolidEntity(sphere, btTransform(btQuaternion(0,0,0), btVector3(0.f, 0.f, 5.f)));
    
    Trajectory* traj = new Trajectory("Trajectory", sphere, btVector3(0,0,0), 1000);
    traj->setRenderable(true);
    AddSensor(traj);
    
    //sphere->SetArbitraryPhysicalProperties(100.0, btVector3(10000000.0,10000000.0,10000000.0), btTransform(btQuaternion::getIdentity(), btVector3(0,0,200.0)));

    /*for(int i=0; i<10; i++)
    {
        color = CreateMatteLook(i/10.f+0.2f, 1.0f - i/10.f * 0.8f, 0.1f, 0.5f);
        BoxEntity* box = new BoxEntity("Box" + std::to_string(i), btVector3(500.f,500.f,500.f), getMaterialManager()->getMaterial("Steel"), color);
        box->setRenderable(true);
        AddSolidEntity(box, btTransform(btQuaternion(UnitSystem::Angle(true, i * 25.f), 0., 0.), btVector3(sinf(i * M_PI/10.f)*5000.f, -i*1000.f, 3000.f + i*300.f)));
        
        if(i == 5)
        {
            ADC* adc = new ADC(12, 3.3);
            Accelerometer* acc = new Accelerometer("Acc", box, btTransform(btQuaternion(0.,0.,0.), btVector3(0.,0.,0.)), Z_AXIS, -3, 3, 0.3, 1.5, 0.00000028, adc, true, 1000);
            box->setDisplayCoordSys(true);
            AddSensor(acc);
        }
    }*/
    
    //////CAMERA & LIGHT//////
    //OpenGLOmniLight* omni = new OpenGLOmniLight(btVector3(5000.f, 5000.f, 10000.f), OpenGLLight::ColorFromTemperature(4500, 1000));
    //AddLight(omni);
    //OpenGLSpotLight* spot = new OpenGLSpotLight(btVector3(5000.f, 5000.f, 10000.f), btVector3(0.f,0.f,0.f), 30.f, OpenGLLight::ColorFromTemperature(4500, 1000));
    //AddLight(spot);
    //spot = new OpenGLSpotLight(btVector3(10000.f, -12000.f, 5000.f), btVector3(5000.f,-5000.f,0.f), 30.f, OpenGLLight::ColorFromTemperature(5600, 500));
    //AddLight(spot);
    
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0, 0, 0.5f), 20.f, btVector3(0,0,1.f), 0, 0, FallingTestApp::getApp()->getWindowWidth(), FallingTestApp::getApp()->getWindowHeight(), 60.f, 100.f, false);
    trackb->Rotate(btQuaternion(M_PI, 0, M_PI/8.0));
    trackb->Activate();
    AddView(trackb);
}