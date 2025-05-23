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
//  Copyright (c) 2018-2025 Patryk Cieslak. All rights reserved.
//

#include "core/ConsoleSimulationApp.h"

#include <chrono>
#include <thread>
#include <omp.h>
#include "core/SimulationManager.h"
#include "utils/SystemUtil.hpp"

namespace sf
{

ConsoleSimulationApp::ConsoleSimulationApp(std::string title, std::string dataDirPath, SimulationManager* sim)
: SimulationApp(title, dataDirPath, sim)
{
    simulationThread = nullptr;
}

ConsoleSimulationApp::~ConsoleSimulationApp()
{
    delete console_;
}

bool ConsoleSimulationApp::hasGraphics()
{
    return false;
}

void ConsoleSimulationApp::Init()
{
    SimulationApp::Init();
    cInfo("Initializing simulation:");
    InitializeSimulation();
    cInfo("Ready for running...");

    state_ = SimulationState::STOPPED;
}

void ConsoleSimulationApp::LoopInternal()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void ConsoleSimulationApp::StartSimulation()
{
    SimulationApp::StartSimulation();
    
    if (autostep_)
    {
        ConsoleSimulationThreadData* data = new ConsoleSimulationThreadData{*this};
        simulationThread = SDL_CreateThread(ConsoleSimulationApp::RunSimulation, "simulationThread", data);
    }
}

void ConsoleSimulationApp::ResumeSimulation()
{
    SimulationApp::ResumeSimulation();
    
    if (autostep_)
    {
        ConsoleSimulationThreadData* data = new ConsoleSimulationThreadData{*this};
        simulationThread = SDL_CreateThread(ConsoleSimulationApp::RunSimulation, "simulationThread", data);
    }
}

void ConsoleSimulationApp::StopSimulation()
{
    SimulationApp::StopSimulation();
    
    if (autostep_ && simulationThread != nullptr)
    {
        int status;
        SDL_WaitThread(simulationThread, &status);
        simulationThread = nullptr;
    }
}

//Static
int ConsoleSimulationApp::RunSimulation(void* data)
{
    ConsoleSimulationApp& simApp = static_cast<ConsoleSimulationThreadData*>(data)->app;
    SimulationManager* simManager = simApp.getSimulationManager();
    simManager->setCallSimulationStepCompleted(simApp.timeStep_ == Scalar(0));

    int maxThreads = std::max(omp_get_max_threads()/2, 1);
    omp_set_num_threads(maxThreads);
    
    while(simApp.getState() == SimulationState::RUNNING)
    {
        simApp.StepSimulation();
    }

    return 0;
}

}
