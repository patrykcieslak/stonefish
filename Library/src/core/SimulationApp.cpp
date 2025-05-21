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
//  Copyright (c) 2012-2025 Patryk Cieslak. All rights reserved.
//

#include "core/SimulationApp.h"

#include "core/SimulationManager.h"
#include "utils/SystemUtil.hpp"

namespace sf
{

SimulationApp::SimulationApp(std::string name, std::string dataDirPath, SimulationManager* sim)
{
    SimulationApp::handle = this;
	state_ = SimulationState::NOT_READY;
    appName = name;
    dataPath = dataDirPath;
    simulation = sim;
    physicsTime = 0.0;
    timeStep_ = Scalar(0);
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
        SimulationApp::handle = nullptr;
}

SimulationState SimulationApp::getState() const
{
    return state_;
}

SimulationManager* SimulationApp::getSimulationManager()
{
    return simulation;
}

double SimulationApp::getPhysicsTime()
{
    return physicsTime;
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

void SimulationApp::Init()
{
}

void SimulationApp::InitializeSimulation()
{
    cInfo("Building scenario...");
    simulation->RestartScenario();
    cInfo("Synchronizing motion states...");
    simulation->getDynamicsWorld()->synchronizeMotionStates();
    cInfo("Simulation initialized -> using Bullet Physics %d.%d.", btGetVersion()/100, btGetVersion()%100);
}

void SimulationApp::Run(bool autostart, bool autostep, Scalar timeStep)
{
    autostep_ = autostep;
    timeStep_ = timeStep < Scalar(0) ? Scalar(0) : timeStep;

    Init();
    if(autostart) StartSimulation();
	Loop();
	CleanUp();
}

void SimulationApp::Loop()
{
    startTime = GetTimeInMicroseconds();
    while(state_ != SimulationState::FINISHED)
        LoopInternal();
}

void SimulationApp::StartSimulation()
{
    simulation->StartSimulation();
    state_ = SimulationState::RUNNING;
}

void SimulationApp::ResumeSimulation()
{
    simulation->ResumeSimulation();
    state_ = SimulationState::RUNNING;
}

void SimulationApp::StopSimulation()
{
    simulation->StopSimulation();
	state_ = SimulationState::STOPPED;
    physicsTime = 0.f;
}

void SimulationApp::StepSimulation()
{
    if (timeStep_ == Scalar(0)) // Real time simulation
    {
        simulation->AdvanceSimulation();
    }
    else // Fixed step simulation
    {   
        simulation->StepSimulation(timeStep_);
        simulation->SimulationStepCompleted(timeStep_);
    }
}

void SimulationApp::Quit()
{
    state_ = SimulationState::FINISHED;
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
