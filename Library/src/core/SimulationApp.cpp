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

SimulationApp::SimulationApp(std::string title, std::string dataDirPath, SimulationManager* sim)
    : console_{new Console()}, startTime_{0}, autostep_{true}, timeStep_{Scalar(0)}, state_{SimulationState::NOT_READY},
      simManager_{sim}, title_{title}, dataPath_{dataDirPath}, physicsTime_{0.0}
{
    SimulationApp::handle = this;

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
    return simManager_;
}

double SimulationApp::getPhysicsTime()
{
    return physicsTime_;
}

std::string SimulationApp::getDataPath()
{
    return dataPath_;
}

std::string SimulationApp::getName()
{
	return title_;
}

Console* SimulationApp::getConsole()
{
    return console_;
}

void SimulationApp::Init()
{
}

void SimulationApp::InitializeSimulation()
{
    cInfo("Building scenario...");
    simManager_->RestartScenario();
    cInfo("Synchronizing motion states...");
    simManager_->getDynamicsWorld()->synchronizeMotionStates();
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
    startTime_ = GetTimeInMicroseconds();
    while(state_ != SimulationState::FINISHED)
        LoopInternal();
}

void SimulationApp::StartSimulation()
{
    simManager_->StartSimulation();
    state_ = SimulationState::RUNNING;
}

void SimulationApp::ResumeSimulation()
{
    simManager_->ResumeSimulation();
    state_ = SimulationState::RUNNING;
}

void SimulationApp::StopSimulation()
{
    simManager_->StopSimulation();
	state_ = SimulationState::STOPPED;
    physicsTime_ = 0.f;
}

void SimulationApp::StepSimulation()
{
    if (timeStep_ == Scalar(0)) // Real time simulation
    {
        simManager_->AdvanceSimulation();
    }
    else // Fixed step simulation
    {   
        simManager_->StepSimulation(timeStep_);
        simManager_->SimulationStepCompleted(timeStep_);
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
