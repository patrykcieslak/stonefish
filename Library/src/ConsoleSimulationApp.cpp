//
//  ConsoleSimulationApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "ConsoleSimulationApp.h"
#include "SystemUtil.hpp"
#include <chrono>
#include <thread>

ConsoleSimulationApp::ConsoleSimulationApp(std::string name, std::string dataDirPath, SimulationManager* sim)
: SimulationApp(name, dataDirPath, sim)
{
	simulationThread = NULL;
}

ConsoleSimulationApp::~ConsoleSimulationApp()
{
}

bool ConsoleSimulationApp::hasGraphics()
{
	return false;
}

void ConsoleSimulationApp::Init()
{
	cInfo("Initializing simulation:");
    InitializeSimulation();
	StartSimulation();
    cInfo("Running...");
}

void ConsoleSimulationApp::Loop()
{
    while(!hasFinished())
    {
#ifdef DEBUG
		btScalar simTime = getSimulationManager()->getSimulationTime();
		cInfo("Simulation time: %1.3lf s", simTime);
#endif
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
    
    while(stdata->app->isRunning())
        sim->AdvanceSimulation();
	
	return 0;
}