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
#include "TorusEntity.h"
#include "CylinderEntity.h"
#include "OpenGLSpotLight.h"
#include "OpenGLTrackball.h"
#include "AccelerationSensor.h"
#include "ADC.h"

FallingTestManager::FallingTestManager(btScalar stepsPerSecond) : SimulationManager(MMKS, true, stepsPerSecond)
{
}

void FallingTestManager::BuildScenario()
{
    SimulationManager::BuildScenario();
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Steel", UnitSystem::Density(CGS, MMKS, 7.8), 0.5, 1.0, 1.0);
    getMaterialManager()->CreateMaterial("Plastic", UnitSystem::Density(CGS, MMKS, 1.5), 0.5, 1.0, 1.0);
    
    ///////LOOKS///////////
    Look grey = CreateMatteLook(0.7f, 0.7f, 0.7f, 0.8f);
    Look color;
    
    ////////OBJECTS
    BoxEntity* box = new BoxEntity("Floor", btVector3(2000.f,2000.f,10.f), getMaterialManager()->getMaterial("Steel"), grey, true);
    box->setRenderable(true);
    AddEntity(box);
    
    for(int i=0; i<100; i++)
    {
        color = CreateMatteLook(i/100.f+0.2f, 1.0f - i/100.f * 0.8f, 0.1f, 0.5f);
        BoxEntity* box = new BoxEntity("Box" + std::to_string(i), btVector3(100.f,100.f,100.f), getMaterialManager()->getMaterial("Steel"), color);
        box->setRenderable(true);
        AddSolidEntity(box, btTransform(btQuaternion(UnitSystem::Angle(true, i * 25.f), 0., 0.), btVector3(sinf(i * M_PI/10.f)*200.f, cosf(i * M_PI/10.f)*200.f, 1000.f + i*300.f)));
        
        if(i == 50)
        {
            ADC* adc = new ADC(12, 3.3);
            AccelerationSensor* acc = new AccelerationSensor("Acc", box, btTransform(btQuaternion(0.,0.,0.), btVector3(0.,0.,0.)), Y_AXIS, -3, 3, 0.3, 1.5, 0.00000028, adc);
            box->setDisplayCoordSys(true);
            AddSensor(acc);
        }
    }
    
    //////CAMERA & LIGHT//////
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0, 0, 500.f), 3000.f, btVector3(0,0,1.f), 0, 0, FallingTestApp::getApp()->getWindowWidth(), FallingTestApp::getApp()->getWindowHeight(), 0, 60);
    trackb->Rotate(btQuaternion(M_PI, 0, -M_PI/8.0));
    trackb->Activate();
    AddView(trackb);
}