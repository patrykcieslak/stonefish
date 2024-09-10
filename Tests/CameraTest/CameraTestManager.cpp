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
//  Copyright (c) 2024 Patryk Cieslak. All rights reserved.
//

#include "CameraTestManager.h"

#include <utils/UnitSystem.h>
#include <utils/SystemUtil.hpp>
#include <entities/statics/Plane.h>
#include <entities/AnimatedEntity.h>
#include <entities/animation/PWLTrajectory.h>
#include <entities/animation/BSTrajectory.h>
#include <sensors/vision/ColorCamera.h>
#include <sensors/vision/DepthCamera.h>
#include <sensors/vision/ThermalCamera.h>

CameraTestManager::CameraTestManager(sf::Scalar stepsPerSecond)
   : SimulationManager(stepsPerSecond, sf::SolverType::SOLVER_SI, sf::CollisionFilteringType::COLLISION_EXCLUSIVE)
{
}

void CameraTestManager::BuildScenario()
{
    EnableOcean();

    ///////MATERIALS////////
    CreateMaterial("Ground", 1000.0, 1.0);
    CreateMaterial("Steel", sf::UnitSystem::Density(sf::CGS, sf::MKS, 1.0), 0.1);
    SetMaterialsInteraction("Ground", "Ground", 0.5, 0.3);
    SetMaterialsInteraction("Ground", "Steel", 0.5, 0.3);
    SetMaterialsInteraction("Steel", "Steel", 0.5, 0.3);
    
    ///////LOOKS///////////
    CreateLook("Grid", sf::Color::Gray(1.f), 0.8f, 0.f, 0.0f, sf::GetShaderPath() + "grid.png");
    CreateLook("Green", sf::Color::RGB(0.1f, 0.1f, 0.1f), 0.2f, 0.0f);
    
    ////////OBJECTS
    // sf::Plane* floor = new sf::Plane("Floor", 10000.f, "Ground", "Grid");
    // AddStaticEntity(floor, sf::Transform::getIdentity());

    sf::PWLTrajectory* traj = new sf::PWLTrajectory(sf::PlaybackMode::BOOMERANG);
    traj->AddKeyPoint(sf::Scalar(0.), sf::Transform(sf::Quaternion(0.,0.,0.), sf::Vector3(-5.0, -2.0, -1.0)));
    traj->AddKeyPoint(sf::Scalar(1.), sf::Transform(sf::Quaternion(0.,0.,0.), sf::Vector3(-5.0, 0.0, -1.0)));
    traj->AddKeyPoint(sf::Scalar(2.), sf::Transform(sf::Quaternion(0.,0.,0.), sf::Vector3(-2.0, 0.0, -1.0)));
    traj->AddKeyPoint(sf::Scalar(6.), sf::Transform(sf::Quaternion(0.,0.,0.), sf::Vector3(-2.0, 0.0, -1.0)));
    sf::AnimatedEntity* sphere1 = new sf::AnimatedEntity("Sphere1", traj, sf::Scalar(1.), sf::Transform::getIdentity(), "Steel", "Green");
    AddAnimatedEntity(sphere1);

    sf::PWLTrajectory* camTraj = new sf::PWLTrajectory(sf::PlaybackMode::BOOMERANG);
    camTraj->AddKeyPoint(sf::Scalar(0.), sf::Transform(sf::Quaternion(0.,0.,0.), sf::Vector3(0.0, 0.0, -1.0)));
    camTraj->AddKeyPoint(sf::Scalar(2.), sf::Transform(sf::Quaternion(0.,-0.5,0.), sf::Vector3(0.0, 0.0, -1.0)));
    camTraj->AddKeyPoint(sf::Scalar(4.), sf::Transform(sf::Quaternion(0.,-1.4,0.), sf::Vector3(5.0, 0.0, -1.0)));
    camTraj->AddKeyPoint(sf::Scalar(6.), sf::Transform(sf::Quaternion(0.,0.,0.), sf::Vector3(5.0, 2.0, -1.0)));

    sf::AnimatedEntity* camFrame = new sf::AnimatedEntity("CamFrame", camTraj);
    AddAnimatedEntity(camFrame);

    sf::ColorCamera* cCam = new sf::ColorCamera("ColorCamera", 350, 200, sf::Scalar(90.0));
    cCam->setDisplayOnScreen(true, 850, 0, 1.0);
    cCam->AttachToSolid(camFrame, sf::Transform(sf::Quaternion(-M_PI_2, 0.0, M_PI_2), sf::Vector3(0,0,0)));
    AddSensor(cCam);

    sf::DepthCamera* dCam = new sf::DepthCamera("DepthCamera", 350, 200, sf::Scalar(90), sf::Scalar(0.01), sf::Scalar(10.0));
    dCam->setNoise(0.01);
    dCam->setDisplayOnScreen(true, 850, 200, 1.0);
    dCam->AttachToSolid(camFrame, sf::Transform(sf::Quaternion(-M_PI_2, 0.0, M_PI_2), sf::Vector3(0,0,0)));
    AddSensor(dCam);

    sf::ThermalCamera* tCam = new sf::ThermalCamera("ThermalCamera", 350, 200, sf::Scalar(90.0), -100.f, 100.f); 
    tCam->setNoise(0.5);
    tCam->setDisplaySettings(sf::ColorMap::GREY, -20, 50);
    tCam->setDisplayOnScreen(true, 850, 400, 1.0);
    tCam->AttachToSolid(camFrame, sf::Transform(sf::Quaternion(-M_PI_2, 0.0, M_PI_2), sf::Vector3(0,0,0)));
    AddSensor(tCam);
}