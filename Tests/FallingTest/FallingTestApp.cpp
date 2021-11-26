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
//  FallingTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2021 Patryk Cieslak. All rights reserved.
//

#include "FallingTestApp.h"

#include <sensors/ScalarSensor.h>
#include <graphics/IMGUI.h>

FallingTestApp::FallingTestApp(std::string dataDirPath, sf::RenderSettings s, sf::HelperSettings h, FallingTestManager* sim)
    : sf::GraphicalSimulationApp("Falling Test", dataDirPath, s, h, sim)
{
}

void FallingTestApp::DoHUD()
{
    sf::GraphicalSimulationApp::DoHUD();
    
    sf::Uid slider;
    slider.owner = 1;
    slider.item = 0;
    getSimulationManager()->setStepsPerSecond(getGUI()->DoSlider(slider, 180.f, 10.f, 120.f, 100.0, 2000.0, getSimulationManager()->getStepsPerSecond(), "Steps/s"));
    
    /*sf::Uid plot;
    plot.owner = 1;
    plot.item = 1;
    std::vector<unsigned short> dims;
    dims.push_back(2);
    getGUI()->DoTimePlot(plot, getWindowWidth()-310, getWindowHeight() - 240, 300, 200, (sf::ScalarSensor*)getSimulationManager()->getSensor("Odom"), dims, "Height");*/

    /*
    dims.clear();
    dims.push_back(0);
    plot.item = 2;
    getGUI()->DoTimePlot(plot, getWindowWidth()-310, getWindowHeight() - 450, 300, 200, (sf::ScalarSensor*)getSimulationManager()->getSensor("Encoder"), dims, "Angle");
    */
}
