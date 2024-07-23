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
//  FluidDynamicsTest
//
//  Created by Patryk Cieslak on 01/07/2024.
//  Copyright(c) 2024 Patryk Cieslak. All rights reserved.
//

#include "FluidDynamicsTestApp.h"
#include "FluidDynamicsTestManager.h"
#include <cfenv>

int main(int argc, const char * argv[])
{
    //feenableexcept(FE_ALL_EXCEPT & ~FE_INEXACT);
    //feenableexcept(FE_INVALID | FE_OVERFLOW);
    
    sf::RenderSettings s;
    s.windowW = 1200;
    s.windowH = 800;
    s.aa = sf::RenderQuality::HIGH;
    s.shadows = sf::RenderQuality::HIGH;
    s.ao = sf::RenderQuality::HIGH;
    s.atmosphere = sf::RenderQuality::MEDIUM;
    s.ocean = sf::RenderQuality::HIGH;
    s.ssr = sf::RenderQuality::HIGH;
    
    sf::HelperSettings h;
    h.showFluidDynamics = false;
    h.showCoordSys = false;
    h.showBulletDebugInfo = false;
    h.showSensors = false;
    h.showActuators = false;
    h.showForces = false;
    
    FluidDynamicsTestManager simulationManager(500.0);
    simulationManager.setRealtimeFactor(1.0);
    FluidDynamicsTestApp app(std::string(DATA_DIR_PATH), s, h, &simulationManager);
    app.Run();
    
    return 0;
}

