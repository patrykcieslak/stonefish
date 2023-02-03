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
//  JointsTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 01/02/2023.
//  Copyright (c) 2023 Patryk Cieslak. All rights reserved.
//

#include "JointsTestApp.h"

#include <actuators/Servo.h>
#include <core/Robot.h>
#include <core/Console.h>
#include <graphics/IMGUI.h>

JointsTestApp::JointsTestApp(std::string dataDirPath, sf::RenderSettings s, sf::HelperSettings h, JointsTestManager* sim)
    : GraphicalSimulationApp("Joints Test", dataDirPath, s, h, sim)
{
}

void JointsTestApp::DoHUD()
{
    GraphicalSimulationApp::DoHUD();

    // sf::Uid id;
    // id.owner = 10;
    // id.item = 0;

    // sf::Servo* srv1 = (sf::Servo*)getSimulationManager()->getActuator("Servo1");
    // sf::Scalar sp = getGUI()->DoSlider(id, 180.f, 10.f, 150.f, sf::Scalar(-1), sf::Scalar(1), srv1->getPosition(), "Servo1");
    // srv1->setDesiredPosition(sp);
}
