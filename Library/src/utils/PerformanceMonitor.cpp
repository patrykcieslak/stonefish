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
    maxCount = averageMaxCount;
    
    simTime = 0;
    simFinished = true;
    phyTime = std::deque<double>(0);
    phyTimeAvg = 0;
    hydroTime = std::deque<double>(0);
    hydroTimeAvg = 0;
    updateMtx = SDL_CreateMutex();
}

PerformanceMonitor::~PerformanceMonitor()
{
    SDL_DestroyMutex(updateMtx);
}

void PerformanceMonitor::SimulationStarted()
{
    SDL_LockMutex(updateMtx);
    simStart = std::chrono::high_resolution_clock::now();
    simTime = 0;
    simFinished = false;
    phyTime.clear();
    phyTimeAvg = 0;
    hydroTime.clear();
    hydroTimeAvg = 0;
    SDL_UnlockMutex(updateMtx);
}

void PerformanceMonitor::SimulationFinished()
{
    SDL_LockMutex(updateMtx);
    auto simEnd = std::chrono::high_resolution_clock::now();
    simTime = std::chrono::duration_cast<std::chrono::seconds>(simEnd - simStart).count();
    simFinished = true;
    SDL_UnlockMutex(updateMtx);
}

void PerformanceMonitor::PhysicsStarted()
{
    phyStart = std::chrono::high_resolution_clock::now();
}

void PerformanceMonitor::PhysicsFinished()
{
    Update(phyStart, phyTime, phyTimeAvg);
}

void PerformanceMonitor::HydrodynamicsStarted()
{
    hydroStart = std::chrono::high_resolution_clock::now();
}

void PerformanceMonitor::HydrodynamicsFinished()
{
    Update(hydroStart, hydroTime, hydroTimeAvg);
}

double PerformanceMonitor::getSimulationTime()
{
    SDL_LockMutex(updateMtx);
    if(!simFinished)
    {
        auto simEnd = std::chrono::high_resolution_clock::now();
        simTime = std::chrono::duration_cast<std::chrono::seconds>(simEnd - simStart).count();
    }
    double simTimeCopy = simTime;
    SDL_UnlockMutex(updateMtx);
    return simTimeCopy;
}

double PerformanceMonitor::getPhysicsTime()
{
    SDL_LockMutex(updateMtx);
    double t = phyTime.back();
    SDL_UnlockMutex(updateMtx);
    return t;
}

double PerformanceMonitor::getPhysicsTimeAverage()
{
    SDL_LockMutex(updateMtx);
    double t = phyTimeAvg;
    SDL_UnlockMutex(updateMtx);
    return t;
}

double PerformanceMonitor::getHydrodynamicsTime()
{
    SDL_LockMutex(updateMtx);
    double t = hydroTime.back();
    SDL_UnlockMutex(updateMtx);
    return t;
}

double PerformanceMonitor::getHydrodynamicsTimeAverage()
{
    SDL_LockMutex(updateMtx);
    double t = hydroTimeAvg;
    SDL_UnlockMutex(updateMtx);
    return t;
}

void PerformanceMonitor::Update(const std::chrono::high_resolution_clock::time_point& start, std::deque<double>& times, double& average)
{
    // Compute elapsed time
    auto end = std::chrono::high_resolution_clock::now();
    SDL_LockMutex(updateMtx);
    double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    // Update averaging queue
    times.push_back(elapsed);
    if(times.size() > maxCount)
        times.pop_front();
    // Update average
    average = std::accumulate(times.begin(), times.end(), 0.0) / (double)times.size();
    SDL_UnlockMutex(updateMtx);
}

}