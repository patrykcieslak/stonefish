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
//  main.cpp
//  Stonefish
//
//  Created by Roger Pi on 06/10/2024.
//  Copyright (c) 2024 Roger Pi. All rights reserved.
//


#include <core/GraphicalSimulationApp.h>
#include "ConfigurableThrusterTestManager.h"

int main(int argc, const char * argv[])
{
    sf::RenderSettings s;
    s.windowW = 1200;
    s.windowH = 900;
    s.aa = sf::RenderQuality::HIGH;
    s.shadows = sf::RenderQuality::HIGH;
    s.ao = sf::RenderQuality::HIGH;
    s.atmosphere = sf::RenderQuality::HIGH;
    s.ocean = sf::RenderQuality::HIGH;
    
    sf::HelperSettings h;
    h.showFluidDynamics = false;
    h.showCoordSys = false;
    h.showBulletDebugInfo = false;
    h.showSensors = false;
    h.showActuators = false;
    h.showForces = false;
    h.showJoints = false;
    
    ConfigurableThrusterTestManager simulationManager(500.0);
    sf::GraphicalSimulationApp app("ThrusterTest", std::string(DATA_DIR_PATH), s, h, &simulationManager);
    app.Run();
    
    return 0;
}

