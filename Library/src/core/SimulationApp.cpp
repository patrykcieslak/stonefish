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
//  Copyright (c) 2012-2026 Patryk Cieslak. All rights reserved.
//

#include "core/SimulationApp.h"

#include "core/SimulationManager.h"
#include "utils/SystemUtil.hpp"
#include <dlfcn.h>

namespace sf
{

SimulationApp::SimulationApp(std::string_view title, std::string_view dataDirPath, std::unique_ptr<SimulationManager> sim)
    : console_{std::make_unique<Console>()}, startTime_{0}, autostep_{true}, timeStep_{Scalar(0)}, state_{SimulationState::NOT_READY},
      simManager_{std::move(sim)}, title_{title}, dataPath_{dataDirPath}, physicsTime_{0.0}
{
    SimulationApp::handle = this;

    //Version info
    if(STONEFISH_VER_PATCH != 0)
        cInfo("Welcome to Stonefish %d.%d.%d", STONEFISH_VER_MAJOR, STONEFISH_VER_MINOR, STONEFISH_VER_PATCH);
    else
        cInfo("Welcome to Stonefish %d.%d.", STONEFISH_VER_MAJOR, STONEFISH_VER_MINOR);

    //Get available threads
    SetMaxPhysicsThreads(GetPhysicalCores());
}

SimulationApp::~SimulationApp()
{
    for (auto h : pluginHandles_)
        dlclose(h.second);

    if(SimulationApp::handle == this)
        SimulationApp::handle = nullptr;
}

void SimulationApp::SetMaxPhysicsThreads(unsigned int n)
{
    maxPhysicsThreads_ = n > 0 ? n : 1;
}

unsigned int SimulationApp::GetMaxPhysicsThreads() const
{
    return maxPhysicsThreads_;
}

SimulationState SimulationApp::GetState() const
{
    return state_;
}

SimulationManager* SimulationApp::GetSimulationManager()
{
    return simManager_.get();
}

double SimulationApp::GetPhysicsTime()
{
    return physicsTime_;
}

const std::string& SimulationApp::GetDataPath() const
{
    return dataPath_;
}

const std::string& SimulationApp::GetName() const
{
	return title_;
}

Console* SimulationApp::GetConsole()
{
    return console_.get();
}

ThreadPool* SimulationApp::GetPhysicsThreadPool()
{
    return physicsThreadPool_.get();
}

void* SimulationApp::GetPluginHandle(std::string_view name)
{
    auto it = pluginHandles_.find(std::string(name));
    if (it != pluginHandles_.end())
        return it->second;
    else
        return nullptr;
}

void SimulationApp::Init()
{
}

void SimulationApp::InitializeSimulation()
{
    cInfo("Building scenario...");
    simManager_->RestartScenario();
    cInfo("Synchronizing motion states...");
    simManager_->GetDynamicsWorld()->synchronizeMotionStates();
    cInfo("Simulation initialized -> using Bullet Physics %d.%d.", btGetVersion()/100, btGetVersion()%100);
}

void SimulationApp::Run(bool autostart, bool autostep, Scalar timeStep)
{
    autostep_ = autostep;
    timeStep_ = timeStep < Scalar(0) ? Scalar(0) : timeStep;

    Init(); // Initialize the simulator and build scenario
    if(autostart) StartSimulation(); // Start simulation updates
	Loop(); // Loop until terminated
	CleanUp(); // Clean up all the allocated resources
}

void SimulationApp::Loop()
{
    startTime_ = GetTimeInMicroseconds();
    while(state_ != SimulationState::FINISHED)
        LoopInternal();
}

void SimulationApp::StartSimulation()
{
    if (GetMaxPhysicsThreads() > 1)
    {
        physicsThreadPool_ = std::make_unique<ThreadPool>(GetMaxPhysicsThreads()); // Prepare threads for running physics
        cInfo("Multithreading physics using %d threads.", GetMaxPhysicsThreads());
    }
    
    simManager_->StartSimulation();
    state_ = SimulationState::RUNNING;
}

void SimulationApp::ResumeSimulation()
{
    if (GetMaxPhysicsThreads() > 1)
    {
        physicsThreadPool_ = std::make_unique<ThreadPool>(GetMaxPhysicsThreads()); // Prepare threads for running physics
        cInfo("Multithreading physics using %d threads.", GetMaxPhysicsThreads());
    }

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

void SimulationApp::AddPluginHandle(std::string_view name, void* handle)
{
    pluginHandles_.insert({std::string(name), handle});
}

//Static
SimulationApp* SimulationApp::handle = NULL;

SimulationApp* SimulationApp::GetApp()
{
    return SimulationApp::handle;
}

}
