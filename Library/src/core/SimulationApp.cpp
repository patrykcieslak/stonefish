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
//  SimulationApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012-2020 Patryk Cieslak. All rights reserved.
//

#include "core/SimulationApp.h"

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
    console = new Console();
    //Version info
    if(STONEFISH_VER_PATCH != 0)
        cInfo("Welcome to Stonefish %d.%d.%d", STONEFISH_VER_MAJOR, STONEFISH_VER_MINOR, STONEFISH_VER_PATCH);
    else
        cInfo("Welcome to Stonefish %d.%d.", STONEFISH_VER_MAJOR, STONEFISH_VER_MINOR);
}

SimulationApp::~SimulationApp()
{
    if(SimulationApp::handle == this)
        SimulationApp::handle = NULL;
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
}

//Static
SimulationApp* SimulationApp::handle = NULL;

SimulationApp* SimulationApp::getApp()
{
    return SimulationApp::handle;
}

}
