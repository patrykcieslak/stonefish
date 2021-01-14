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
//  ConsoleSimulationApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2018-2020 Patryk Cieslak. All rights reserved.
//

#include "core/ConsoleSimulationApp.h"

#include <chrono>
#include <thread>
#include "core/SimulationManager.h"
#include "utils/SystemUtil.hpp"

namespace sf
{

ConsoleSimulationApp::ConsoleSimulationApp(std::string name, std::string dataDirPath, SimulationManager* sim)
: SimulationApp(name, dataDirPath, sim)
{
    simulationThread = NULL;
}

ConsoleSimulationApp::~ConsoleSimulationApp()
{
    delete console;
}

bool ConsoleSimulationApp::hasGraphics()
{
    return false;
}

void ConsoleSimulationApp::Init()
{
    //Initialization
    cInfo("Initializing simulation:");
    InitializeSimulation();
    cInfo("Ready for running...");
}

void ConsoleSimulationApp::Loop()
{
    while(!hasFinished())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void ConsoleSimulationApp::StartSimulation()
{
    SimulationApp::StartSimulation();
    
    ConsoleSimulationThreadData* data = new ConsoleSimulationThreadData();
    data->app = this;
    simulationThread = SDL_CreateThread(ConsoleSimulationApp::RunSimulation, "simulationThread", data);
}

void ConsoleSimulationApp::ResumeSimulation()
{
    SimulationApp::ResumeSimulation();
    
    ConsoleSimulationThreadData* data = new ConsoleSimulationThreadData();
    data->app = this;
    simulationThread = SDL_CreateThread(ConsoleSimulationApp::RunSimulation, "simulationThread", data);
}

void ConsoleSimulationApp::StopSimulation()
{
    SimulationApp::StopSimulation();
    
    int status;
    SDL_WaitThread(simulationThread, &status);
    simulationThread = NULL;
}

//Static
int ConsoleSimulationApp::RunSimulation(void* data)
{
    ConsoleSimulationThreadData* stdata = (ConsoleSimulationThreadData*)data;
    SimulationManager* sim = stdata->app->getSimulationManager();
    
    while(stdata->app->isRunning())
        sim->AdvanceSimulation();

    return 0;
}

}
