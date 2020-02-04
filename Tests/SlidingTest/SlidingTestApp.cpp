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
//  SlidingTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#include "SlidingTestApp.h"
#include <graphics/IMGUI.h>
#include <sensors/ScalarSensor.h>

SlidingTestApp::SlidingTestApp(std::string dataDirPath, sf::RenderSettings s, sf::HelperSettings h, SlidingTestManager* sim) 
                               : GraphicalSimulationApp("Sliding Test", dataDirPath, s, h, sim)
{
}

void SlidingTestApp::DoHUD()
{
    GraphicalSimulationApp::DoHUD();
    
    sf::Uid plot;
    plot.owner = 1;
    plot.item = 0;
  
    std::vector<unsigned short> dims;
    dims.push_back(3);
    dims.push_back(4);
    dims.push_back(5);
    getGUI()->DoTimePlot(plot, getWindowWidth()-310, getWindowHeight() - 240, 300, 200, (sf::ScalarSensor*)getSimulationManager()->getSensor("Odometry"), dims, "Velocity");
}
