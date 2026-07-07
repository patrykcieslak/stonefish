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
//  PerformanceMonitor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 02/09/2022.
//  Copyright (c) 2022 Patryk Cieslak. All rights reserved.
//

#include "utils/PerformanceMonitor.h"
#include <numeric>

namespace sf
{

PerformanceMonitor::PerformanceMonitor(size_t averageMaxCount)
{
    maxCount_ = averageMaxCount;
    
    simTime_ = 0;
    simFinished_ = true;
    phyTime_ = std::deque<double>(0);
    phyTimeAvg_ = 0;
    hydroTime_ = std::deque<double>(0);
    hydroTimeAvg_ = 0;
    updateMtx_ = SDL_CreateMutex();
}

PerformanceMonitor::~PerformanceMonitor()
{
    SDL_DestroyMutex(updateMtx_);
}

void PerformanceMonitor::SimulationStarted()
{
    SDL_LockMutex(updateMtx_);
    simStart_ = std::chrono::high_resolution_clock::now();
    simTime_ = 0;
    simFinished_ = false;
    phyTime_.clear();
    phyTimeAvg_ = 0;
    hydroTime_.clear();
    hydroTimeAvg_ = 0;
    SDL_UnlockMutex(updateMtx_);
}

void PerformanceMonitor::SimulationFinished()
{
    SDL_LockMutex(updateMtx_);
    auto simEnd = std::chrono::high_resolution_clock::now();
    simTime_ = std::chrono::duration_cast<std::chrono::seconds>(simEnd - simStart_).count();
    simFinished_ = true;
    SDL_UnlockMutex(updateMtx_);
}

void PerformanceMonitor::PhysicsStarted()
{
    phyStart_ = std::chrono::high_resolution_clock::now();
}

void PerformanceMonitor::PhysicsFinished()
{
    Update(phyStart_, phyTime_, phyTimeAvg_);
}

void PerformanceMonitor::HydrodynamicsStarted()
{
    hydroStart_ = std::chrono::high_resolution_clock::now();
}

void PerformanceMonitor::HydrodynamicsFinished()
{
    Update(hydroStart_, hydroTime_, hydroTimeAvg_);
}

double PerformanceMonitor::getSimulationTime()
{
    SDL_LockMutex(updateMtx_);
    if(!simFinished_)
    {
        auto simEnd = std::chrono::high_resolution_clock::now();
        simTime_ = std::chrono::duration_cast<std::chrono::seconds>(simEnd - simStart_).count();
    }
    double simTimeCopy = simTime_;
    SDL_UnlockMutex(updateMtx_);
    return simTimeCopy;
}

double PerformanceMonitor::getPhysicsTime()
{
    SDL_LockMutex(updateMtx_);
    double t = phyTime_.back();
    SDL_UnlockMutex(updateMtx_);
    return t;
}

double PerformanceMonitor::getPhysicsTimeAverage()
{
    SDL_LockMutex(updateMtx_);
    double t = phyTimeAvg_;
    SDL_UnlockMutex(updateMtx_);
    return t;
}

double PerformanceMonitor::getHydrodynamicsTime()
{
    SDL_LockMutex(updateMtx_);
    double t = hydroTime_.back();
    SDL_UnlockMutex(updateMtx_);
    return t;
}

double PerformanceMonitor::getHydrodynamicsTimeAverage()
{
    SDL_LockMutex(updateMtx_);
    double t = hydroTimeAvg_;
    SDL_UnlockMutex(updateMtx_);
    return t;
}

void PerformanceMonitor::Update(const std::chrono::high_resolution_clock::time_point& start, std::deque<double>& times, double& average)
{
    // Compute elapsed time
    auto end = std::chrono::high_resolution_clock::now();
    SDL_LockMutex(updateMtx_);
    double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    // Update averaging queue
    times.push_back(elapsed);
    if(times.size() > maxCount_)
        times.pop_front();
    // Update average
    average = std::accumulate(times.begin(), times.end(), 0.0) / (double)times.size();
    SDL_UnlockMutex(updateMtx_);
}

}