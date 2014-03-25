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

RollingTestManager::RollingTestManager(btScalar stepsPerSecond) : SimulationManager(MMKS, true, stepsPerSecond)
{
}

void RollingTestManager::BuildScenario()
{
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Steel", 7.81, 0.5, 1.0, 1.0);
    getMaterialManager()->CreateMaterial("Plastic", 1.5, 0.5, 1.0, 1.0);
    
    ///////LOOKS///////////
    Look grey = CreateMatteLook(0.7f, 0.7f, 0.7f, 0.8f);
    
    ////////OBJECTS
    /*PlaneEntity* floor = new PlaneEntity("Floor", 1000000.f, getMaterialManager()->getMaterial("Steel"), grey, btTransform(btQuaternion(0,-M_PI_2,0), btVector3(0,0,1000)));
    floor->setRenderable(true);
    floor->getRigidBody()->setRollingFriction(1.f);
    AddEntity(floor);*/
    BoxEntity* box = new BoxEntity("Floor", btVector3(100000.f,100000.f,10.f), true, getMaterialManager()->getMaterial("Steel"), grey);
    box->setRenderable(true);
    AddEntity(box);
    
    Look shiny = CreateGlossyLook(1.f, 0.5f, 0.2f, 0.5f, 0.f);
    /*TorusEntity* torus = new TorusEntity("Torus", 1000.f, 400.f, false, getMaterialManager()->getMaterial("Plastic"), shiny);
    torus->setRenderable(true);
    AddSolidEntity(torus, btTransform(btQuaternion(M_PI/180.0,0,M_PI_2), btVector3(0,0,-1000.f)));
    //torus->getRigidBody()->setAngularVelocity(btVector3(10.0,0.0,0.0));
    torus->getRigidBody()->setRollingFriction(1.f);
    */
    CylinderEntity* cylinder = new CylinderEntity("Cyl", 1000.f, 400.f, false, getMaterialManager()->getMaterial("Plastic"), shiny);
    cylinder->setRenderable(true);
    AddSolidEntity(cylinder, btTransform(btQuaternion(0,M_PI/360.0,0), btVector3(0,0,-1000.f)));
    
    //////CAMERA & LIGHT//////
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0, 0, -500.f), 8000.f, btVector3(0,0,-1.f), 0, 0, SimulationApp::getApp()->getWindowWidth(), SimulationApp::getApp()->getWindowHeight(), 0, 60.f);
    trackb->Rotate(btQuaternion(M_PI, -M_PI/10.0, 0));
    trackb->Activate();
    AddView(trackb);
}