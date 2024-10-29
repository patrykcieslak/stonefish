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
//  Actuator.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/8/13.
//  Copyright (c) 2013-2024 Patryk Cieslak. All rights reserved.
//

#include "actuators/Actuator.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Actuator::Actuator(std::string uniqueName)
{
    name = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
    dm = DisplayMode::GRAPHICAL;
    setWatchdog(Scalar(-1));
}

Actuator::~Actuator()
{
    if(SimulationApp::getApp() != nullptr)
        SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name);
}

void Actuator::setDisplayMode(DisplayMode m)
{
    dm = m;
}

void Actuator::setWatchdog(Scalar timeout)
{
    watchdogTimeout = timeout;
    watchdog = Scalar(0);
}

std::string Actuator::getName() const
{
    return name;
}

void Actuator::Update(Scalar dt)
{
    if(watchdogTimeout > Scalar(0))
    {
        watchdog += dt;
        if(watchdog > watchdogTimeout)
            WatchdogTimeout();
    }
}

std::vector<Renderable> Actuator::Render()
{
    std::vector<Renderable> items(0);
    return items;
}

void Actuator::WatchdogTimeout()
{
}

void Actuator::ResetWatchdog()
{
    watchdog = Scalar(0);
}

}
