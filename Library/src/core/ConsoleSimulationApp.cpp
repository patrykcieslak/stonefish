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
//  Copyright (c) 2018-2022 Patryk Cieslak. All rights reserved.
//

#include "core/ConsoleSimulationApp.h"

#include <chrono>
#include <thread>
#include <omp.h>
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
    SimulationApp::Init();
    cInfo("Initializing simulation:");
    InitializeSimulation();
    cInfo("Ready for running...");
}

void ConsoleSimulationApp::KeyDown(SDL_Event *event)
{
    GLfloat moveStep = 0.1f;
    if(event->key.keysym.mod & KMOD_SHIFT)
        moveStep = 1.f;

    switch (event->key.keysym.sym)
    {
        case SDLK_ESCAPE:
            Quit();
            break;
            
        case SDLK_SPACE:
             if(!getSimulationManager()->isSimulationFresh())
             {
                 StopSimulation();
                 getSimulationManager()->RestartScenario();
             }
             StartSimulation();
             break;
        default:
            break;
    }
}

void ConsoleSimulationApp::LoopInternal()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    SDL_Event event;
    SDL_FlushEvents(SDL_FINGERDOWN, SDL_MULTIGESTURE);
            
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
                
            case SDL_KEYDOWN:
            {
                KeyDown(&event);
                break;
            }
                 
            case SDL_QUIT:
            {
                if(isRunning())
                    StopSimulation();
                    
                Quit();
            }   
                break;
        }
    }
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

    int maxThreads = std::max(omp_get_max_threads()/2, 1);
    omp_set_num_threads(maxThreads);
    
    while(stdata->app->isRunning())
        sim->AdvanceSimulation();

    return 0;
}
void ConsoleSimulationApp::StopSimulationWrapper() {
        StopSimulation();
}

void ConsoleSimulationApp::ResumeSimulationWrapper() {
        ResumeSimulation();
}

void ConsoleSimulationApp::StartSimulationWrapper() {
        StartSimulation();
}
}
