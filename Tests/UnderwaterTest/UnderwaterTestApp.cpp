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
//  UnderwaterTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2025 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterTestApp.h"

#include <actuators/Servo.h>
#include <actuators/Thruster.h>
#include <actuators/VariableBuoyancy.h>
#include <core/Robot.h>
#include <sensors/scalar/Accelerometer.h>
#include <sensors/scalar/IMU.h>
#include <sensors/scalar/DVL.h>
#include <sensors/vision/FLS.h>
#include <sensors/vision/SSS.h>
#include <graphics/IMGUI.h>
#include <comms/USBL.h>
#include <core/Console.h>

UnderwaterTestApp::UnderwaterTestApp(std::string dataDirPath, sf::RenderSettings s, sf::HelperSettings h, UnderwaterTestManager* sim)
    : GraphicalSimulationApp("UnderwaterTest", dataDirPath, s, h, sim), surge_(0), yaw_(0)
{
}

void UnderwaterTestApp::DoHUD()
{
    GraphicalSimulationApp::DoHUD();
#ifdef PARSED_SCENARIO
    sf::Thruster* th1 = dynamic_cast<sf::Thruster*>(getSimulationManager()->getRobot("GIRONA500")->getActuator("GIRONA500/ThrusterSurgePort"));
    sf::Thruster* th2 = dynamic_cast<sf::Thruster*>(getSimulationManager()->getRobot("GIRONA500")->getActuator("GIRONA500/ThrusterSurgeStarboard"));
#else
    sf::Thruster* th1 = dynamic_cast<sf::Thruster*>(getSimulationManager()->getRobot("GIRONA500")->getActuator("ThrusterSurgePort"));
    sf::Thruster* th2 = dynamic_cast<sf::Thruster*>(getSimulationManager()->getRobot("GIRONA500")->getActuator("ThrusterSurgeStarboard"));
#endif
    if (th1 && th2)
    {
        sf::Uid id;
        id.owner = 10;
        
        id.item = 0;
        surge_ = getGUI()->DoSlider(id, 180.f, 10.f, 250.f, sf::Scalar(-1), sf::Scalar(1), surge_, "Surge");

        id.item = 1;
        yaw_ = getGUI()->DoSlider(id, 180.f, 65.f, 250.f, sf::Scalar(-1), sf::Scalar(1), yaw_, "Yaw");

        th1->setSetpoint(-surge_ - yaw_);
        th2->setSetpoint(-surge_ + yaw_);
    }
}
