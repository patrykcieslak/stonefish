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
//  FluidDynamicsTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 01/07/2024.
//  Copyright (c) 2024 Patryk Cieslak. All rights reserved.
//

#include "FluidDynamicsTestApp.h"

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
#include <utils/SystemUtil.hpp>
#include <comms/USBL.h>
#include <core/Console.h>

FluidDynamicsTestApp::FluidDynamicsTestApp(std::string dataDirPath, sf::RenderSettings s, sf::HelperSettings h, FluidDynamicsTestManager* sim)
    : GraphicalSimulationApp("FluidDynamics Test", dataDirPath, s, h, sim)
{
}

void FluidDynamicsTestApp::DoHUD()
{
    GraphicalSimulationApp::DoHUD();
}
