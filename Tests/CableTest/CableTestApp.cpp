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
//  CableTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 13/11/2025.
//  Copyright (c) 2025-2026 Patryk Cieslak. All rights reserved.
//

#include "CableTestApp.h"

#include <actuators/Motor.h>
#include <actuators/Servo.h>
#include <graphics/IMGUI.h>
#include <core/Robot.h>
#include <entities/forcefields/Uniform.h>

CableTestApp::CableTestApp(std::string_view dataDirPath, sf::RenderSettings s, sf::HelperSettings h, std::unique_ptr<CableTestManager> sim)
    : sf::GraphicalSimulationApp("CableTest", dataDirPath, s, h, std::move(sim))
{
}

void CableTestApp::DoHUD()
{
    sf::GraphicalSimulationApp::DoHUD();
    
    sf::Servo* servo = dynamic_cast<sf::Servo*>(GetSimulationManager()->GetRobot("Winch")->GetActuator("Servo"));
    if (servo != nullptr)
    {
        sf::Uid winchSlider;
        winchSlider.owner = 10;
        winchSlider.item = 0; 
        servo->SetDesiredVelocity(GetGui()->DoSlider(winchSlider, 180.f, 10.f, 300.f, -1.0, 1.0, servo->GetDesiredVelocity(), "Servo Velocity [rad/s]"));
    }

    sf::Uniform* uniformCurrent = dynamic_cast<sf::Uniform*>(GetSimulationManager()->GetOcean()->GetVelocityField(0));
    if (uniformCurrent != nullptr)
    {
        sf::Uid currentXSlider;
        currentXSlider.owner = 10;
        currentXSlider.item = 1;
        uniformCurrent->SetVelocity(sf::Vector3(GetGui()->DoSlider(currentXSlider, 180.f, 65.f, 300.f, -5.0, 5.0, uniformCurrent->GetVelocity().getX(), "Current Velocity X [m/s]"), 0.0, 0.0));
    }
}
