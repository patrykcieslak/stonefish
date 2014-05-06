//
//  RollingTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "RollingTestManager.h"

#include "RollingTestApp.h"
#include "PlaneEntity.h"
#include "BoxEntity.h"
#include "TorusEntity.h"
#include "CylinderEntity.h"
#include "OpenGLOmniLight.h"
#include "OpenGLTrackball.h"
#include "FixedJoint.h"
#include "RevoluteJoint.h"

RollingTestManager::RollingTestManager(btScalar stepsPerSecond) : SimulationManager(MMKS, true, stepsPerSecond)
{
}

void RollingTestManager::BuildScenario()
{
    SimulationManager::BuildScenario();
    
    //--------------------Using MMSK unit system--------------------
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Concrete", UnitSystem::Density(CGS, MMKS, 4.0), 0.7);
    getMaterialManager()->CreateMaterial("Rubber", UnitSystem::Density(CGS, MMKS, 2.0), 0.5);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Rubber", 0.7, 0.3);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Concrete", 0.9, 0.7);
    getMaterialManager()->SetMaterialsInteraction("Rubber", "Rubber", 0.7, 0.5);
    
    ///////LOOKS///////////
    Look grey = CreateMatteLook(0.7f, 0.7f, 0.7f, 0.0f);
    Look shiny = CreateMatteLook(0.2f, 0.2f, 0.2f, 0.5f);
    
    ////////OBJECTS
    PlaneEntity* floor = new PlaneEntity("Floor", 1000000.f, getMaterialManager()->getMaterial("Concrete"), grey, btTransform(btQuaternion(0,M_PI/90.f,M_PI_2), btVector3(0,0,0.f)));
    floor->setRenderable(true);
    AddEntity(floor);
    
    TorusEntity* torus = new TorusEntity("Torus", 200.f, 20.f, getMaterialManager()->getMaterial("Rubber"), shiny);
    torus->setRenderable(true);
    AddSolidEntity(torus, btTransform(btQuaternion(M_PI_4,0.f,0.1f), btVector3(0,0,1000.f)));
    
    //////CAMERA & LIGHT//////
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0, 0, 500.f), 2000.f, btVector3(0,0,1.f), 0, 0, SimulationApp::getApp()->getWindowWidth(), SimulationApp::getApp()->getWindowHeight(), 0, 60.f);
    trackb->Rotate(btQuaternion(M_PI, 0.0, -M_PI/10.0));
    trackb->Activate();
    AddView(trackb);
}