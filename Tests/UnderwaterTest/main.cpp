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
//  UnderwaterTest
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright(c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterTestApp.h"
#include "UnderwaterTestManager.h"
#include <cfenv>

int main(int argc, const char * argv[])
{
    //feenableexcept(FE_ALL_EXCEPT & ~FE_INEXACT);
    //feenableexcept(FE_INVALID | FE_OVERFLOW);
    
    sf::RenderSettings s;
    s.windowW = 1200;
    s.windowH = 900;
    s.msaa = false;
    s.shadows = sf::RenderQuality::QUALITY_HIGH;
    s.ao = sf::RenderQuality::QUALITY_LOW;
    s.atmosphere = sf::RenderQuality::QUALITY_MEDIUM;
    s.ocean = sf::RenderQuality::QUALITY_HIGH;
    
    sf::HelperSettings h;
    h.showFluidDynamics = false;
    h.showCoordSys = true;
    h.showBulletDebugInfo = false;
    h.showSensors = false;
    h.showActuators = true;
    h.showForces = false;
    
    UnderwaterTestManager* simulationManager = new UnderwaterTestManager(200.0);
    UnderwaterTestApp app(std::string(DATA_DIR_PATH), s, h, simulationManager);
    app.Run(false);
    
    return 0;
}

