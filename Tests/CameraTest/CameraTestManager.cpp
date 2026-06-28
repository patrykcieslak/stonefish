/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  CameraTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 09/02/2024.
//  Copyright (c) 2024-2025 Patryk Cieslak. All rights reserved.
//

#include "CameraTestManager.h"

#include <utils/UnitSystem.h>
#include <utils/SystemUtil.hpp>
#include <entities/statics/Plane.h>
#include <entities/statics/Obstacle.h>
#include <entities/AnimatedEntity.h>
#include <entities/animation/PWLTrajectory.h>
#include <entities/animation/BSTrajectory.h>
#include <entities/animation/CRTrajectory.h>
#include <sensors/vision/ColorCamera.h>
#include <sensors/vision/DepthCamera.h>
#include <sensors/vision/ThermalCamera.h>
#include <sensors/vision/OpticalFlowCamera.h>
#include <sensors/vision/EventBasedCamera.h>
#include <sensors/vision/SegmentationCamera.h>
#include <iostream>

#ifdef DEBUG
#include <iostream>
#endif

CameraTestManager::CameraTestManager(sf::Scalar stepsPerSecond)
   : SimulationManager(stepsPerSecond, sf::Solver::SI, sf::CollisionFilter::EXCLUSIVE)
{
}

void CameraTestManager::BuildScenario()
{
    EnableOcean();
    getAtmosphere()->SetConditions(20.0, 101300.0, 0.7);
    getOcean()->SetConditions(15.0);

    ///////MATERIALS////////
    CreateMaterial("Ground", 1000.0, 1.0);
    CreateMaterial("Steel", sf::UnitSystem::Density(sf::CGS, sf::MKS, 1.0), 0.1);
    SetMaterialsInteraction("Ground", "Ground", 0.5, 0.3);
    SetMaterialsInteraction("Ground", "Steel", 0.5, 0.3);
    SetMaterialsInteraction("Steel", "Steel", 0.5, 0.3);

    ///////LOOKS///////////
    CreateLook("Grid", sf::Color::Gray(1.f), 0.8f, 0.f, 0.0f, sf::GetShaderPath() + "grid.png");
    CreateLook("Green", sf::Color::RGB(0.1f, 0.1f, 0.1f), 0.2f, 0.0f);
#ifdef TEST_THERMAL_CAMERA
    CreateLook("Boat", sf::Color::Gray(1.f), 0.9f, 0.f, 0.0f, sf::GetDataPath() + "aquadelmo_tex.png");
    CreateLook("Skin", sf::Color::HSV(0.07, 0.516, 0.8), 1.0f, 0.0f, 0.0f, "", "", "", std::make_pair(36.6f, 36.6f));
#else
    CreateLook("Canyon", sf::Color::Gray(1.f), 0.9f, 0.f, 0.0f, sf::GetDataPath() + "canyon.png");
#endif

    ////////OBJECTS
    sf::Plane* floor = new sf::Plane("Floor", 10000.f, "Ground", "Grid");
    AddStaticEntity(floor, sf::Transform(sf::Quaternion(0.,0.,0.), sf::Vector3(0.0, 0.0, 100.0)));

#ifdef TEST_THERMAL_CAMERA
    sf::Obstacle* boat = new sf::Obstacle("Boat", sf::GetDataPath() + "aquadelmo.obj", 1.0, sf::I4(), sf::GetDataPath() + "aquadelmo_phy.obj", 1.0, sf::I4(), true, "Steel", "Boat");
	AddStaticEntity(boat, sf::Transform(sf::Quaternion(1.3,0,0.0), sf::Vector3(0.0,0.0,0)));
    sf::Obstacle* human = new sf::Obstacle("Human", sf::GetDataPath() + "human.obj", 0.1, sf::I4(), true, "Steel", "Skin");
	AddStaticEntity(human, sf::Transform(sf::Quaternion(1.3,0,0.0), sf::Vector3(0.0,0.0,0.0)));
#else    
    sf::Obstacle* canyon = new sf::Obstacle("Canyon", sf::GetDataPath() + "canyon.obj", 1.0, sf::I4(), false, "Ground", "Canyon");
    AddStaticEntity(canyon, sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 10.0)));
#endif
    
    sf::BSTrajectory* camTraj = new sf::BSTrajectory(sf::PlaybackMode::REPEAT);
#ifdef TEST_THERMAL_CAMERA
    camTraj->AddKeyPoint(sf::Scalar(0.), sf::Transform(sf::Quaternion(0.,0.,0.), sf::Vector3(7.0, 0.0, -1.0)));
    camTraj->AddKeyPoint(sf::Scalar(5.), sf::Transform(sf::Quaternion(M_PI_2,0.,0.), sf::Vector3(0.0, 7.0, -1.0)));
    camTraj->AddKeyPoint(sf::Scalar(10.), sf::Transform(sf::Quaternion(M_PI,0.,0.), sf::Vector3(-7.0, 0.0, -1.0)));
    camTraj->AddKeyPoint(sf::Scalar(15.), sf::Transform(sf::Quaternion(1.5*M_PI,0.,0.), sf::Vector3(0.0, -7.0, -1.0)));
    camTraj->AddKeyPoint(sf::Scalar(20.), sf::Transform(sf::Quaternion(0.,0.,0.), sf::Vector3(7.0, 0.0, -1.0)));
#else
    camTraj->AddKeyPoint(sf::Scalar(0.), sf::Transform(sf::Quaternion(-2.67, 0., 0.), sf::Vector3(-9.6, -4.8, 9.0)));
    camTraj->AddKeyPoint(sf::Scalar(2.), sf::Transform(sf::Quaternion(-2.67, 0., 0.), sf::Vector3(-9.6, -4.8, 9.0)));
    camTraj->AddKeyPoint(sf::Scalar(4.5), sf::Transform(sf::Quaternion(-1.86, 0., 0.), sf::Vector3(-6.33, 1.4, 9.0)));
    camTraj->AddKeyPoint(sf::Scalar(6.5), sf::Transform(sf::Quaternion(-2.85, 0., 0.), sf::Vector3(-2.13, 8.08, 9.0)));
    camTraj->AddKeyPoint(sf::Scalar(8.), sf::Transform(sf::Quaternion(-3.5, 0., 0.), sf::Vector3(5.03, 7.02, 8.0)));
    camTraj->AddKeyPoint(sf::Scalar(9.), sf::Transform(sf::Quaternion(M_PI_2, 0.,0.), sf::Vector3(7.0, 5.0, 9.0)));
    camTraj->AddKeyPoint(sf::Scalar(10.), sf::Transform(sf::Quaternion(M_PI_2, 0.,0.), sf::Vector3(7.0, 0.0, 9.0)));
    camTraj->AddKeyPoint(sf::Scalar(12.), sf::Transform(sf::Quaternion(0.85, 0.,0.), sf::Vector3(6.85, -7.2, 9.0)));
    camTraj->AddKeyPoint(sf::Scalar(14.), sf::Transform(sf::Quaternion(0.0, 0.,0.), sf::Vector3(-5.75, -11.0, 9.0)));
    camTraj->AddKeyPoint(sf::Scalar(17.), sf::Transform(sf::Quaternion(0.0, 0.,0.), sf::Vector3(-11.0, -11.0, 9.0)));
#endif

    sf::AnimatedEntity* camFrame = new sf::AnimatedEntity("CamFrame", camTraj);
    AddAnimatedEntity(camFrame);

    sf::ColorCamera* cCam = new sf::ColorCamera("ColorCamera", 400, 300, sf::Scalar(90.0));
    cCam->setDisplayOnScreen(true, 200, 0, 1.0);
    cCam->AttachToSolid(camFrame, sf::Transform(sf::Quaternion(-M_PI_2, 0.0, M_PI_2), sf::Vector3(0,0,0)));
    AddSensor(cCam);

#ifdef TEST_THERMAL_CAMERA
    sf::ThermalCamera* tCam = new sf::ThermalCamera("ThermalCamera", 400, 300, sf::Scalar(90.0), -100.f, 100.f); 
    tCam->setNoise(0.2);
    tCam->setDisplaySettings(sf::ColorMap::JET, 5.0, 50.0);
    tCam->setDisplayOnScreen(true, 600, 0, 1.0);
    tCam->AttachToSolid(camFrame, sf::Transform(sf::Quaternion(-M_PI_2, 0.0, M_PI_2), sf::Vector3(0,0,0)));
    AddSensor(tCam);
#else
    sf::DepthCamera* dCam = new sf::DepthCamera("DepthCamera", 400, 300, sf::Scalar(90), sf::Scalar(0.01), sf::Scalar(10.0));
    dCam->setNoise(0.01);
    dCam->setDisplayOnScreen(true, 200, 300, 1.0);
    dCam->AttachToSolid(camFrame, sf::Transform(sf::Quaternion(-M_PI_2, 0.0, M_PI_2), sf::Vector3(0,0,0)));
    AddSensor(dCam);

    sf::OpticalFlowCamera* ofCam = new sf::OpticalFlowCamera("OpticalFlowCamera", 400, 300, sf::Scalar(90));
    ofCam->setDisplayOnScreen(true, 600, 0, 1.0);
    ofCam->setDisplaySettings(800.0);
    ofCam->AttachToSolid(camFrame, sf::Transform(sf::Quaternion(-M_PI_2, 0.0, M_PI_2), sf::Vector3(0,0,0)));
    AddSensor(ofCam);

    sf::EventBasedCamera* evbCam = new sf::EventBasedCamera("EventBasedCamera", 400, 300, sf::Scalar(90.0), 0.1f, 0.1f, 1000, 10.0);
    evbCam->setNoise(0.03, 0.03);
    evbCam->setDisplayOnScreen(true, 600, 300, 1.0);
    evbCam->AttachToSolid(camFrame, sf::Transform(sf::Quaternion(-M_PI_2, 0.0, M_PI_2), sf::Vector3(0,0,0)));
#ifdef DEBUG
    evbCam->InstallNewDataHandler([this](sf::EventBasedCamera* cam)
    {
        std::cout << "EBC last event count: " << cam->getLastEventCount() << std::endl;
    });
#endif
    AddSensor(evbCam);
#endif
    sf::SegmentationCamera* sCam = new sf::SegmentationCamera("SegmentationCamera", 400, 300, sf::Scalar(90.0));
    sCam->setDisplayOnScreen(true, 200, 600, 1.0);
    sCam->AttachToSolid(camFrame, sf::Transform(sf::Quaternion(-M_PI_2, 0.0, M_PI_2), sf::Vector3(0,0,0)));
    AddSensor(sCam);
}