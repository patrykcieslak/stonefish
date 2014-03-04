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
#include "OpenGLSpotLight.h"
#include "OpenGLTrackball.h"

FallingTestManager::FallingTestManager(btScalar stepsPerSecond) : SimulationManager(stepsPerSecond)
{
}

void FallingTestManager::BuildScenario()
{
    ///////WORLD////////////
    UnitSystem::SetUnitSystem(MMKS, true);
    SetGravity(UnitSystem::Acceleration(MKS, MMKS, btVector3(0,0,9.81)));
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Steel", 7.81, 0.1, 1.0, 1.0);
    
    ///////LOOKS///////////
    Look grey = CreateMatteLook(0.7f, 0.7f, 0.7f, 0.8f);
    
    ////////OBJECTS
    PlaneEntity* floor = new PlaneEntity("Floor", 1000000.f, getMaterialManager()->getMaterial("Steel"), grey, btTransform(btQuaternion(0,-M_PI_2,0), btVector3(0,0,1000)));
    floor->setRenderable(true);
    AddEntity(floor);
    
    for(int i=0; i<100; i++)
    {
        Look color = CreateMatteLook(i/100.f+0.2f, 1.0f - i/100.f * 0.8f, 0.1f, 0.5f);
        BoxEntity* box = new BoxEntity("Box" + std::to_string(i), btVector3(100.f,100.f,100.f), false, getMaterialManager()->getMaterial("Steel"), color);
        box->setRenderable(true);
        AddSolidEntity(box, btTransform(btQuaternion(UnitSystem::Angle(true, i * 25.f), 0, 0), btVector3(sinf(i * M_PI/10.f)*200.f, cosf(i * M_PI/10.f)*200.f, -100.f - i*300.f)));
    }
    
    //////CAMERA & LIGHT//////
    OpenGLSpotLight* spot = new OpenGLSpotLight(btVector3(-1000,-3000,-4000), btVector3(0,500,0), 30, OpenGLLight::ColorFromTemperature(8000, 1200000));
    AddLight(spot);
    
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0, 0, -500.f), 8000.f, btVector3(0,0,-1.f), 0, 0, FallingTestApp::getApp()->getWindowWidth(), FallingTestApp::getApp()->getWindowHeight(), 2, 60.f);
    trackb->Rotate(btQuaternion(M_PI, -M_PI/10.0, 0));
    trackb->Activate();
    AddView(trackb);
}