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
//  ConfigurableThrusterTestManager.cpp
//  Stonefish
//
//  Created by Roger Pi on 06/10/2024.
//  Copyright (c) 2024 Roger Pi. All rights reserved.
//

#include "ConfigurableThrusterTestManager.h"

#include <entities/statics/Plane.h>
#include <entities/statics/Obstacle.h>
#include <entities/solids/Box.h>
#include <actuators/Light.h>
#include <sensors/scalar/Odometry.h>
#include <utils/SystemUtil.hpp>
#include <utils/UnitSystem.h>
#include <entities/solids/Polyhedron.h>

#include <core/FeatherstoneRobot.h>
#include <entities/FeatherstoneEntity.h>

ConfigurableThrusterTestManager::ConfigurableThrusterTestManager(sf::Scalar stepsPerSecond)
  : SimulationManager(stepsPerSecond, sf::SolverType::SOLVER_SI, sf::CollisionFilteringType::COLLISION_EXCLUSIVE)
{
  rotor_model_ =  //
      std::make_shared<sf::rm::FirstOrder>(0.99999999);

  thrust_model_ =  //
      std::make_shared<sf::tm::BasicThrustConversion>(0.005);

  mcap_sink_ = std::make_shared<DataTamer::MCAPSink>("thruster_test.mcap");

  // Create a channel and attach a sink. A channel can have multiple sinks
  channel_ = DataTamer::LogChannel::create("output");
  channel_->addDataSink(mcap_sink_);

  channel_->registerValue("setpoint", &setpoint_);
  channel_->registerValue("omega", &omega_);
  channel_->registerValue("thrust", &thrust_);

  double out = 0;
  double in = 0;

  auto channel2 = DataTamer::LogChannel::create("conv");
  channel2->addDataSink(mcap_sink_);

  channel2->registerValue("out", &out);
  channel2->registerValue("in", &in);

  for (in = -200; in < 200; in += 0.1)
  {
    out = thrust_model_->f(in);
    channel2->takeSnapshot();
    // sleep 0.01
    std::this_thread::sleep_for(std::chrono::nanoseconds(10));
  }
}

ConfigurableThrusterTestManager::~ConfigurableThrusterTestManager()
{
  // if (thruster_)
  // {
  //   delete thruster_;
  // }
}

void ConfigurableThrusterTestManager::BuildScenario()
{
  ///////MATERIALS////////
  CreateMaterial("Dummy", sf::UnitSystem::Density(sf::CGS, sf::MKS, 0.9), 0.3);
  CreateMaterial("Fiberglass", sf::UnitSystem::Density(sf::CGS, sf::MKS, 1.5), 0.9);
  CreateMaterial("Rock", sf::UnitSystem::Density(sf::CGS, sf::MKS, 3.0), 0.6);

  SetMaterialsInteraction("Dummy", "Dummy", 0.5, 0.2);
  SetMaterialsInteraction("Fiberglass", "Fiberglass", 0.5, 0.2);
  SetMaterialsInteraction("Rock", "Rock", 0.9, 0.7);
  SetMaterialsInteraction("Fiberglass", "Dummy", 0.5, 0.2);
  SetMaterialsInteraction("Rock", "Dummy", 0.6, 0.4);
  SetMaterialsInteraction("Rock", "Fiberglass", 0.6, 0.4);

  ///////LOOKS///////////
  CreateLook("yellow", sf::Color::RGB(1.f, 0.9f, 0.f), 0.3f, 0.f);
  CreateLook("grey", sf::Color::RGB(0.3f, 0.3f, 0.3f), 0.4f, 0.5f);
  CreateLook("black", sf::Color::RGB(0.1f, 0.1f, 0.1f), 0.4f, 0.5f);
  CreateLook("seabed", sf::Color::RGB(0.7f, 0.7f, 0.5f), 0.9f, 0.f, 0.f, "", sf::GetDataPath() + "sand_normal.png");
  CreateLook("propeller", sf::Color::RGB(1.f, 1.f, 1.f), 0.3f, 0.f, 0.f, sf::GetDataPath() + "propeller_tex.png");

  ////////OBJECTS
  EnableOcean(0.0);
  getOcean()->setWaterType(0.2);
  getAtmosphere()->SetupSunPosition(0.0, 60.0);

  sf::Plane* floor = new sf::Plane("Floor", sf::Scalar(10000), "Rock", "seabed");
  AddStaticEntity(floor, sf::Transform(sf::IQ(), sf::Vector3(0, 0, 5)));

  sf::BodyPhysicsSettings phy;
  phy.mode = sf::BodyPhysicsMode::SUBMERGED;
  phy.collisions = true;
  phy.buoyancy = false;

  sf::Polyhedron* prop1 =                                      //
      new sf::Polyhedron("Propeller", phy,                     //
                         sf::GetDataPath() + "propeller.obj",  //
                         sf::Scalar(1),                        //
                         sf::I4(),                             //
                         "Dummy", "propeller");

  thruster_ =                                       //
      new sf::ConfigurableThruster("ThrusterTest",  //
                                   prop1,           //
                                   rotor_model_,    //
                                   thrust_model_,   //
                                   true, false);

  sf::FeatherstoneRobot* dummy_robot = new sf::FeatherstoneRobot("DummyRobot", true);

  // Box
  sf::Box* box = new sf::Box("Box", phy, sf::Vector3(0.01, 0.01, 0.01), sf::I4(), "Dummy", "grey");

  dummy_robot->DefineLinks(box);
  dummy_robot->BuildKinematicStructure();

  dummy_robot->AddLinkActuator(thruster_, "Box", sf::Transform(sf::Quaternion(0, 0, 0), sf::Vector3(0.2, 0, 0)));

  AddRobot(dummy_robot, sf::Transform(sf::IQ(), sf::Vector3(0, 0, 2)));

  thruster_->setWatchdog(-1);
  thruster_->setSetpoint(100);
}

void ConfigurableThrusterTestManager::SimulationStepCompleted(sf::Scalar timeStep)
{
  timestamp_ += timeStep;

  if (timestamp_ > 10)
  {
    thruster_->setSetpoint(-100);
  }

  setpoint_ = thruster_->getSetpoint();
  omega_ = thruster_->getOmega();
  thrust_ = thruster_->getThrust();

  channel_->takeSnapshot();
}