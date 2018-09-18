//
//  SimulationApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012-2017 Patryk Cieslak. All rights reserved.
//

#include "SimulationApp.h"
#include "SystemUtil.hpp"
#include <chrono>
#include <thread>

SimulationApp::SimulationApp(std::string name, std::string dataDirPath, SimulationManager* sim)
{
    SimulationApp::handle = this;
	appName = name;
    dataPath = dataDirPath;
    finished = false;
    running = false;
    simulation = sim;
	physicsTime = 0.0;
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

void SimulationApp::InitializeSimulation()
{
    cInfo("Building scenario...");
    simulation->RestartScenario();
    cInfo("Synchronizing motion states...");
    simulation->getDynamicsWorld()->synchronizeMotionStates();
    cInfo("Simulation initialized -> using Bullet Physics %d.%d.", btGetVersion()/100, btGetVersion()%100);
}

void SimulationApp::Run()
{
	Init();
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