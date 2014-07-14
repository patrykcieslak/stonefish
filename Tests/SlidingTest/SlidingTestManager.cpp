//
//  SlidingTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "SlidingTestManager.h"

#include "SlidingTestApp.h"
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

SlidingTestManager::SlidingTestManager(btScalar stepsPerSecond) : SimulationManager(MKS, true, stepsPerSecond, DANTZIG, STANDARD)
{
}

void SlidingTestManager::BuildScenario()
{
    OpenGLPipeline::getInstance()->setVisibleHelpers(true, false, false, false, false, true, true);
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Ground", 1000.0, 1.0);
    getMaterialManager()->CreateMaterial("Steel", 1000.0, 0.0);
    getMaterialManager()->SetMaterialsInteraction("Ground", "Ground", 0.0, 0.0);
    getMaterialManager()->SetMaterialsInteraction("Ground", "Steel", 0.25, 0.1);
    getMaterialManager()->SetMaterialsInteraction("Steel", "Steel", 0.25, 0.2);
    
    ///////LOOKS///////////
    char path[1024];
    GetDataPath(path, 1024-32);
    strcat(path, "grid.png");
    
    Look grey = CreateOpaqueLook(glm::vec3(1.f, 1.f, 1.f), 0.3f, 0.8f, 1.4f, path);
    //Look color;
    
    ////////OBJECTS
    btScalar angle = M_PI/180.0 * 14.9;
    
    PlaneEntity* floor = new PlaneEntity("Floor", 100.f, getMaterialManager()->getMaterial("Ground"), grey, btTransform(btQuaternion(0,angle,0), btVector3(0,0,0)));
    AddEntity(floor);
    
    //GetDataPath(path, 1024-32);
    //strcat(path, "cone.obj");
    //MeshEntity* mesh = new MeshEntity("Cone", path, 1000.0, getMaterialManager()->getMaterial("Steel"), grey, true);
    //AddSolidEntity(mesh, btTransform(btQuaternion(0,0,0), btVector3(2000.f, 0.f, 0.f)));
    //mesh->SetArbitraryPhysicalProperties(1000, btVector3(10000000.0,10000000.0,10000000.0), btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)));
    
    //ObstacleEntity* terrain = new ObstacleEntity("Terrain", path, 10000.f, getMaterialManager()->getMaterial("Steel"), grey, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0,0,0)), false);
    //AddEntity(terrain);
    
    Look color = CreateOpaqueLook(glm::vec3(1.f,0.6f,0.2f), 0.5f, 0.2f, 1.5f);
    
    BoxEntity* box = new BoxEntity("Box", btVector3(0.1,0.1,0.1), getMaterialManager()->getMaterial("Steel"), color);
    AddSolidEntity(box, btTransform(btQuaternion(0,angle,0), btVector3(0, 0, 0.052)));
    
    Trajectory* traj = new Trajectory("Trajectory", box, btVector3(0,0,0));
    traj->setRenderable(true);
    AddSensor(traj);
    
    //////CAMERA & LIGHT//////
    OpenGLOmniLight* omni = new OpenGLOmniLight(btVector3(0,0,1.0), OpenGLLight::ColorFromTemperature(4000, 10000));
    omni->GlueToEntity(box);
    AddLight(omni);
    
    //OpenGLSpotLight* spot = new OpenGLSpotLight(btVector3(0,0,0.5), btVector3(1.0,0,0.0), 30, OpenGLLight::ColorFromTemperature(4000, 10));
    //spot->GlueToEntity(box);
    //AddLight(spot);
    
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0, 0, 0.2), 2.f, btVector3(0,0,1.f), 0, 0, SlidingTestApp::getApp()->getWindowWidth(), SlidingTestApp::getApp()->getWindowHeight(), 60.f, 100.f, false);
    trackb->Rotate(btQuaternion(M_PI, 0, M_PI/8.0));
    trackb->Activate();
    AddView(trackb);
    
    /*OpenGLCamera* cam = new OpenGLCamera(btVector3(0,0,1.0), btVector3(1.0,0.0,0.0), btVector3(0.0,0.0,1.0), 10, 10, 400, 200, 50, 100, false);
    cam->Activate();
    cam->GlueToEntity(box);
    AddView(cam);*/
}