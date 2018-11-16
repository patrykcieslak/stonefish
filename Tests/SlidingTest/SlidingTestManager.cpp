//
//  SlidingTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "SlidingTestManager.h"

#include "SlidingTestApp.h"
#include <entities/statics/Plane.h>
#include <entities/solids/Box.h>
#include <actuators/Light.h>
#include <sensors/Trajectory.h>
#include <utils/SystemUtil.hpp>

SlidingTestManager::SlidingTestManager(btScalar stepsPerSecond) : SimulationManager(UnitSystems::MKS, true, stepsPerSecond, DANTZIG, EXCLUSIVE)
{
}

void SlidingTestManager::BuildScenario()
{
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->setVisibleHelpers(true, false, false, false, false, true, true);
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Ground", 1000.0, 1.0);
    getMaterialManager()->CreateMaterial("Steel", 1000.0, 0.0);
    getMaterialManager()->SetMaterialsInteraction("Ground", "Ground", 0.0, 0.0);
    getMaterialManager()->SetMaterialsInteraction("Ground", "Steel", 0.25, 0.1);
    getMaterialManager()->SetMaterialsInteraction("Steel", "Steel", 0.25, 0.2);
    
    ///////LOOKS///////////
    int grid = OpenGLContent::getInstance()->CreateSimpleLook(glm::vec3(1.f, 1.f, 1.f), 0.f, 0.1f, GetShaderPath() + "grid.png");
    int green = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(0.3f, 1.0f, 0.2f), 0.2f, 0.f);
    
    ////////OBJECTS
    btScalar angle = M_PI/180.0 * 14.9;
    
    Plane* floor = new Plane("Floor", 1000.f, getMaterialManager()->getMaterial("Ground"), grid);
    AddStaticEntity(floor, btTransform(btQuaternion(0,angle,0), btVector3(0,0,0)));
  
    Box* box = new Box("Box", btVector3(0.1,0.1,0.1), getMaterialManager()->getMaterial("Steel"), green);
    AddSolidEntity(box, btTransform(btQuaternion(0,angle,0), btVector3(0, 0, 0.052)));
    
    Trajectory* traj = new Trajectory("Trajectory", box, btTransform::getIdentity());
    traj->setRenderable(true);
    AddSensor(traj);
    
    //////CAMERA & LIGHT//////
    Light* omni = new Light("Omni", btVector3(0,0,1.0), OpenGLLight::ColorFromTemperature(4000, 10000));
    AddActuator(omni);
    
    //OpenGLSpotLight* spot = new OpenGLSpotLight(btVector3(0,0,0.5), btVector3(1.0,0,0.0), 30, OpenGLLight::ColorFromTemperature(4000, 10));
    //spot->GlueToEntity(box);
    //AddLight(spot);
}
