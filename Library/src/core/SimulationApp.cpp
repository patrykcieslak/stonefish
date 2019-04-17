//
//  SimulationApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012-2018 Patryk Cieslak. All rights reserved.
//

#include "core/SimulationApp.h"

#include "core/Console.h"
#include "core/SimulationManager.h"

namespace sf
{

SimulationApp::SimulationApp(std::string name, std::string dataDirPath, SimulationManager* sim)
{
    SimulationApp::handle = this;
	appName = name;
    dataPath = dataDirPath;
    simulation = sim;
    finished = false;
    running = false;
    physicsTime = 0.0;
    console = NULL;
}

SimulationApp::~SimulationApp()
{
}

SimulationManager* SimulationApp::getSimulationManager()
{
    return simulation;
}

double SimulationApp::getPhysicsTime()
{
    return physicsTime;
}

bool SimulationApp::isRunning()
{
	return running;
}

bool SimulationApp::hasFinished()
{
	return finished;
}

std::string SimulationApp::getDataPath()
{
    return dataPath;
}

std::string SimulationApp::getName()
{
	return appName;
}

Console* SimulationApp::getConsole()
{
    return console;
}

void SimulationApp::InitializeSimulation()
{
    cInfo("Building scenario...");
    simulation->RestartScenario();
    cInfo("Synchronizing motion states...");
    simulation->getDynamicsWorld()->synchronizeMotionStates();
    cInfo("Simulation initialized -> using Bullet Physics %d.%d.", btGetVersion()/100, btGetVersion()%100);
}

void SimulationApp::Run(bool autostart)
{
    Init();
    if(autostart) StartSimulation();
	Loop();
	CleanUp();
}

void SimulationApp::StartSimulation()
{
    simulation->StartSimulation();
    running = true;
}

void SimulationApp::ResumeSimulation()
{
    simulation->ResumeSimulation();
    running = true;
}

void SimulationApp::StopSimulation()
{
    simulation->StopSimulation();
	running = false;
    physicsTime = 0.f;
}

void SimulationApp::Quit()
{
    finished = true;
}

void SimulationApp::CleanUp()
{
    if(simulation != NULL)
        delete simulation;
}

//Static
SimulationApp* SimulationApp::handle = NULL;

SimulationApp* SimulationApp::getApp()
{
    return SimulationApp::handle;
}

}
