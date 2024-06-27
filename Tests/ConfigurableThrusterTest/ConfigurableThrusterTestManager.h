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
//  ConfigurableThrusterTestManager.h
//  Stonefish
//
//  Created by Roger Pi on 06/10/2024.
//  Copyright (c) 2024 Roger Pi. All rights reserved.
//

#pragma once

#include <core/SimulationManager.h>
#include <memory>

#include "data_tamer/data_tamer.hpp"
#include "data_tamer/sinks/mcap_sink.hpp"
#include <actuators/ConfigurableThruster.h>


class ConfigurableThrusterTestManager : public sf::SimulationManager
{
public:
  ConfigurableThrusterTestManager(sf::Scalar stepsPerSecond);

  virtual ~ConfigurableThrusterTestManager();

  void BuildScenario();

  void SimulationStepCompleted(sf::Scalar timeStep) override;

protected:
  std::shared_ptr<DataTamer::MCAPSink> mcap_sink_;
  std::shared_ptr<DataTamer::LogChannel> channel_;

  sf::Scalar setpoint_;
  sf::Scalar omega_;
  sf::Scalar thrust_;

  sf::Scalar timestamp_;

  std::shared_ptr<sf::td::RotorDynamics> rotor_model_;
  std::shared_ptr<sf::td::ThrustModel> thrust_model_;

  sf::ConfigurableThruster* thruster_;
};
