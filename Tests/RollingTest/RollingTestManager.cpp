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
#include "OpenGLSpotLight.h"
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
    getMaterialManager()->CreateMaterial("Steel", UnitSystem::Density(CGS, MMKS, 7.8), 0.3, 0.5, 1.0);
    getMaterialManager()->CreateMaterial("Plastic", UnitSystem::Density(CGS, MMKS, 1.5), 0.3, 0.5, 0.02);
    
    ///////LOOKS///////////
    Look grey = CreateMatteLook(0.7f, 0.7f, 0.7f, 0.8f);
    Look shiny = CreateGlossyLook(1.f, 0.5f, 0.2f, 0.5f, 0.f);
    
    ////////OBJECTS
    PlaneEntity* floor = new PlaneEntity("Floor", 1000000.f, getMaterialManager()->getMaterial("Steel"), grey, btTransform(btQuaternion(0,M_PI/90.f,M_PI_2), btVector3(0,0,-1000.f)));
    floor->setRenderable(true);
    AddEntity(floor);
    
    TorusEntity* torus = new TorusEntity("Torus", 500, 100.f, getMaterialManager()->getMaterial("Plastic"), shiny);
    torus->setRenderable(true);
    AddSolidEntity(torus, btTransform(btQuaternion(M_PI_4,0,0), btVector3(0,0,1000.f)));
    
    //////CAMERA & LIGHT//////
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0, 0, 500.f), 2000.f, btVector3(0,0,1.f), 0, 0, SimulationApp::getApp()->getWindowWidth(), SimulationApp::getApp()->getWindowHeight(), 0, 60.f);
    trackb->Rotate(btQuaternion(M_PI, 0.0, -M_PI/10.0));
    trackb->Activate();
    AddView(trackb);
}