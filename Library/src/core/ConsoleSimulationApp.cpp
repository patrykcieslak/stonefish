//
//  ConsoleSimulationApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "core/ConsoleSimulationApp.h"

#include <chrono>
#include <thread>
#include "core/Console.h"
#include "core/SimulationManager.h"
#include "utils/SystemUtil.hpp"

namespace sf
{

ConsoleSimulationApp::ConsoleSimulationApp(std::string name, std::string dataDirPath, SimulationManager* sim)
: SimulationApp(name, dataDirPath, sim)
{
	simulationThread = NULL;
    console = new Console();
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
	cInfo("Initializing simulation:");
    InitializeSimulation();
    cInfo("Ready for running...");
}

void ConsoleSimulationApp::Loop()
{
	while(!hasFinished());
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
