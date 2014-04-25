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
#include "BoxEntity.h"
#include "SphereEntity.h"
#include "TorusEntity.h"
#include "CylinderEntity.h"
#include "OpenGLOmniLight.h"
#include "OpenGLSpotLight.h"
#include "OpenGLTrackball.h"
#include "OpenGLUtil.h"
#include "Accelerometer.h"
#include "ADC.h"

FallingTestManager::FallingTestManager(btScalar stepsPerSecond) : SimulationManager(MMKS, true, stepsPerSecond)
{
}

void FallingTestManager::BuildScenario()
{
    SimulationManager::BuildScenario();
    SimulationApp::getApp()->getRenderer()->SetVisibleElements(false, false, false, false, false);
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Steel", UnitSystem::Density(CGS, MMKS, 7.8), 0.4);
    getMaterialManager()->SetMaterialsInteraction("Steel", "Steel", 0.2, 0.01);
    
    ///////LOOKS///////////
    char path[1024];
    GetCWD(path, 1024);
    strcat(path, "/");
    strcat(path, SimulationApp::getApp()->getDataPath());
    strcat(path, "/");
    strcat(path, "grid.png");
    
    Look grey = CreateMatteLook(1.0f, 1.0f, 1.0f, 0.0f, path);
    //Look color;
    
    ////////OBJECTS
    PlaneEntity* floor = new PlaneEntity("Floor", 100000.f, getMaterialManager()->getMaterial("Steel"), grey, btTransform(btQuaternion(0, 0, M_PI_2), btVector3(0,0,0)));
    floor->setRenderable(true);
    AddEntity(floor);
    
    Look color = CreateGlossyLook(1.f, 0.6f, 0.2f, 0.5f, 0.1f);
    SphereEntity* sphere = new SphereEntity("Sphere", 1000.f, getMaterialManager()->getMaterial("Steel"), color);
    sphere->setRenderable(true);
    AddSolidEntity(sphere, btTransform(btQuaternion(0,0,0), btVector3(0, 0, 1000.f)));
    
    /*BoxEntity* box = new BoxEntity("Floor", btVector3(100000.f,100000.f,100.f), getMaterialManager()->getMaterial("Steel"), grey, true);
    box->setRenderable(true);
    AddEntity(box);*/
    
    for(int i=0; i<10; i++)
    {
        color = CreateMatteLook(i/10.f+0.2f, 1.0f - i/10.f * 0.8f, 0.1f, 0.5f);
        BoxEntity* box = new BoxEntity("Box" + std::to_string(i), btVector3(500.f,500.f,500.f), getMaterialManager()->getMaterial("Steel"), color);
        box->setRenderable(true);
        AddSolidEntity(box, btTransform(btQuaternion(UnitSystem::Angle(true, i * 25.f), 0., 0.), btVector3(sinf(i * M_PI/10.f)*5000.f, -i*1000.f, 1000.f + i*300.f)));
        
        if(i == 5)
        {
            ADC* adc = new ADC(12, 3.3);
            Accelerometer* acc = new Accelerometer("Acc", box, btTransform(btQuaternion(0.,0.,0.), btVector3(0.,0.,0.)), Z_AXIS, -3, 3, 0.3, 1.5, 0.00000028, adc, true, 1000);
            box->setDisplayCoordSys(true);
            AddSensor(acc);
        }
    }
    /*
    for(int i=0; i<10; i++)
    {
        color = CreateMatteLook(1.f, 0.6f, 0.2f, 0.9f);
        SphereEntity* sphere = new SphereEntity("Sphere" + std::to_string(i), i*50.f, getMaterialManager()->getMaterial("Steel"), color);
        sphere->setRenderable(true);
        AddSolidEntity(sphere, btTransform(btQuaternion(0,0,0), btVector3(i * 100.f, sin(i)*100.f, i*200.f+10.f)));
    }*/
    
    //////CAMERA & LIGHT//////
    //OpenGLOmniLight* omni = new OpenGLOmniLight(btVector3(5000.f, 5000.f, 10000.f), OpenGLLight::ColorFromTemperature(4500, 1000));
    //AddLight(omni);
    //OpenGLSpotLight* spot = new OpenGLSpotLight(btVector3(5000.f, 5000.f, 10000.f), btVector3(0.f,0.f,0.f), 30.f, OpenGLLight::ColorFromTemperature(4500, 1000));
    //AddLight(spot);
    
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0, 0, 500.f), 5000.f, btVector3(0,0,1.f), 0, 0, FallingTestApp::getApp()->getWindowWidth(), FallingTestApp::getApp()->getWindowHeight(), 0, 60.f);
    trackb->Rotate(btQuaternion(M_PI, 0, -M_PI/8.0));
    trackb->Activate();
    AddView(trackb);
}