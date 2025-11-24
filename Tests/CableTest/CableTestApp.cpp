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
//  Copyright (c) 2025 Patryk Cieslak. All rights reserved.
//

#include "CableTestApp.h"

#include <actuators/Motor.h>
#include <actuators/Servo.h>
#include <graphics/IMGUI.h>
#include <core/Robot.h>
#include <entities/forcefields/Uniform.h>

CableTestApp::CableTestApp(std::string dataDirPath, sf::RenderSettings s, sf::HelperSettings h, CableTestManager* sim)
    : sf::GraphicalSimulationApp("CableTest", dataDirPath, s, h, sim)
{
}

void CableTestApp::DoHUD()
{
    sf::GraphicalSimulationApp::DoHUD();
    
    sf::Servo* servo = dynamic_cast<sf::Servo*>(getSimulationManager()->getRobot("Winch")->getActuator("Servo"));
    if (servo != nullptr)
    {
        sf::Uid winchSlider;
        winchSlider.owner = 10;
        winchSlider.item = 0; 
        servo->setDesiredVelocity(getGUI()->DoSlider(winchSlider, 180.f, 10.f, 300.f, -1.0, 1.0, servo->getDesiredVelocity(), "Servo Velocity [rad/s]"));
    }

    sf::Uniform* uniformCurrent = dynamic_cast<sf::Uniform*>(getSimulationManager()->getOcean()->getVelocityField(0));
    if (uniformCurrent != nullptr)
    {
        sf::Uid currentXSlider;
        currentXSlider.owner = 10;
        currentXSlider.item = 1;
        uniformCurrent->setVelocity(sf::Vector3(getGUI()->DoSlider(currentXSlider, 180.f, 65.f, 300.f, -5.0, 5.0, uniformCurrent->getVelocity().getX(), "Current Velocity X [m/s]"), 0.0, 0.0));
    }

    sf::Ocean* ocean = getSimulationManager()->getOcean();
    sf::OceanParams oceanParams = ocean->getOpenGLOcean()->getOceanParams();

    GLfloat offset = 120.f;

    sf::Uid windSpeedSlider;
    windSpeedSlider.owner = 11;
    windSpeedSlider.item = 0;
    oceanParams.windSpeed = getGUI()->DoSlider(windSpeedSlider, 180.f, offset, 300.f, 0.0, 20.0, oceanParams.windSpeed, "Wind Speed [m/s]");
    offset += 55.f;
    
    sf::Uid fetchLengthSlider;
    fetchLengthSlider.owner = 11;
    fetchLengthSlider.item = 1;
    oceanParams.fetchLength = getGUI()->DoSlider(fetchLengthSlider, 180.f, offset, 300.f, 1.0, 10000.0, oceanParams.fetchLength, "Fetch Length [m]");
    offset += 55.f;

    sf::Uid windDirectionSlider;
    windDirectionSlider.owner = 11;
    windDirectionSlider.item = 2;
    oceanParams.windDirection = getGUI()->DoSlider(windDirectionSlider, 180.f, offset, 300.f, 0.0, 2.0*M_PI, oceanParams.windDirection, "Wind Direction [rad]");
    offset += 55.f;

    sf::Uid depthSlider;
    depthSlider.owner = 11;
    depthSlider.item = 3;
    oceanParams.depth = getGUI()->DoSlider(depthSlider, 180.f, offset, 300.f, 1.0, 100.0, oceanParams.depth, "Water Depth [m]");
    offset += 55.f;

    sf::Uid swellSlider;
    swellSlider.owner = 11;
    swellSlider.item = 4;
    oceanParams.swell = getGUI()->DoSlider(swellSlider, 180.f, offset, 300.f, 0.0, 1.0, oceanParams.swell, "Swell Factor");
    offset += 55.f;

    sf::Uid spreadBlendSlider;
    spreadBlendSlider.owner = 11;
    spreadBlendSlider.item = 5;
    oceanParams.spreadBlend = getGUI()->DoSlider(spreadBlendSlider, 180.f, offset, 300.f, 0.0, 1.0, oceanParams.spreadBlend, "Spread Blend");
    offset += 55.f; 

    sf::Uid shortWavesFadeSlider;
    shortWavesFadeSlider.owner = 11;
    shortWavesFadeSlider.item = 6;
    oceanParams.shortWavesFade = getGUI()->DoSlider(shortWavesFadeSlider, 180.f, offset, 300.f, 0.0, 1.0, oceanParams.shortWavesFade, "Short Waves Fade");
    offset += 55.f;

    ocean->getOpenGLOcean()->UpdateOceanParams(oceanParams);
}
