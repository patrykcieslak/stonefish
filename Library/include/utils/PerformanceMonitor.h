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
//  PerformanceMonitor.h
//  Stonefish
//
//  Created by Patryk Cieslak on 02/09/2022.
//  Copyright (c) 2022 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_PerformanceMonitor__
#define __Stonefish_PerformanceMonitor__

#include <SDL2/SDL_mutex.h>
#include <chrono>
#include <deque>
#include <vector>

namespace sf
{
    class PerformanceMonitor
    {
    public:
        PerformanceMonitor(size_t averageMaxCount);
        ~PerformanceMonitor();
        
        void SimulationStarted();
        void SimulationFinished();
        void PhysicsStarted();
        void PhysicsFinished();
        void HydrodynamicsStarted();
        void HydrodynamicsFinished();

        // In seconds.
        double getSimulationTime();
        
        // In microseconds.
        double getPhysicsTime();
        double getPhysicsTimeAverage();
        template<typename T> std::vector<T> getPhysicsTimeHistory(size_t len) { return getHistory<T>(phyTime, len); };

        double getHydrodynamicsTime();
        double getHydrodynamicsTimeAverage();
        template<typename T> std::vector<T> getHydrodynamicsTimeHistory(size_t len) { return getHistory<T>(hydroTime, len); };

    private:
        void Update(const std::chrono::high_resolution_clock::time_point& start, std::deque<double>& times, double& average);
        template<typename T> std::vector<T> getHistory(std::deque<double>& data, size_t len)
        { 
            std::vector<T> dataOut; 
            dataOut.resize(data.size() < len ? data.size() : len);
            SDL_LockMutex(updateMtx);
            size_t offset = data.size() - dataOut.size();
            for(size_t i=data.size()-dataOut.size(); i<data.size(); ++i) 
                dataOut[i-offset] = (T)(data[i]);
            SDL_UnlockMutex(updateMtx);   
            return dataOut;
        };

        size_t maxCount;
        std::chrono::high_resolution_clock::time_point simStart;
        std::chrono::high_resolution_clock::time_point phyStart;
        std::chrono::high_resolution_clock::time_point hydroStart;
        double simTime;
        bool simFinished;
        std::deque<double> phyTime;
        std::deque<double> hydroTime;
        double phyTimeAvg;
        double hydroTimeAvg;
        SDL_mutex* updateMtx;
    };
}

#endif