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
//  SimulationApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012-2026 Patryk Cieslak. All rights reserved.
//

#pragma once

#include "StonefishCommon.h"
#include "core/Console.h"
#include "utils/ThreadPool.hpp"

//Console output aliases
#define cInfo(format, ...)     sf::SimulationApp::GetApp()->GetConsole()->Print(sf::MessageType::INFO, format, ##__VA_ARGS__)
#define cWarning(format, ...)  sf::SimulationApp::GetApp()->GetConsole()->Print(sf::MessageType::WARNING, format, ##__VA_ARGS__)
#define cError(format, ...)    sf::SimulationApp::GetApp()->GetConsole()->Print(sf::MessageType::ERROR, format, ##__VA_ARGS__)
#define cCritical(format, ...) {sf::SimulationApp::GetApp()->GetConsole()->Print(sf::MessageType::CRITICAL, format, ##__VA_ARGS__);abort();}

namespace sf
{
    enum class SimulationState 
    {
        NOT_READY,
        STOPPED,
        RUNNING,
        FINISHED,
    };

    class SimulationManager;
    
    //! An abstract class that defines an application interface hosting a simulation manager.
    class SimulationApp
    {
    public:
        //! A constructor.
        /*!
         \param title a title for the application
         \param dataDirPath a path to the directory containing simulation data
         \param sim a pointer to the simulation manager
         */
        SimulationApp(std::string_view title, std::string_view dataDirPath, std::unique_ptr<SimulationManager> sim);
        
        //! A destructor.
        virtual ~SimulationApp();
        
        //! A method implementing the simulation sequence.
        /*!
         \param autostart optional flag determining if the simulation should automatically start running
         \param autostep optional flag determining if the simulation should automatically step
         \param timeStep optional time step that will be used for each simulation update instead of real time (0 means real time)
         */
        void Run(bool autostart = true, bool autostep = true, Scalar timeStep = Scalar(0));
        
        // ! A method that starts the simulation on demand.
        virtual void StartSimulation();
        
        //! A method that stops the simulation on demand.
        virtual void StopSimulation();

        //! A method that resumes the simulation on demand.
        virtual void ResumeSimulation();

        //! A method that performs a single simulation step and necessary updates.
        virtual void StepSimulation();

        //! A method that stores the plugin handles.
        void AddPluginHandle(std::string_view name, void* handle);
        
        //! A method setting the maximum allowed parallel threads for physcis computation.
        /*!
         \param n number of threads
         */
        void SetMaxPhysicsThreads(unsigned int n);
        
        //! A method returning simulation state.
        SimulationState GetState() const;

        //! A method returning a pointer to the simulation manager.
        SimulationManager* GetSimulationManager();
        
        //! A method returning the physics computation time.
        double GetPhysicsTime();
          
        //! A method returning the path to the directory containing simulation data.
        const std::string& GetDataPath() const;
        
        //! A method returning the name of the application.
        const std::string& GetName() const;
        
        //! A method returning a pointer to the console associated with the application.
        Console* GetConsole();

        //! A method returning the maximum allowed parallel threads for physics computation.
        unsigned int GetMaxPhysicsThreads() const;

        //! A method returning the physics thread pool.
        ThreadPool* GetPhysicsThreadPool();

        //! A method returning the plugin handle.
        void* GetPluginHandle(std::string_view name);

        //! A method informing if the application is graphical.
        virtual bool HasGraphics() = 0;

        //! A static method returning the pointer to the currently running application.
        static SimulationApp* GetApp();
        
    protected:
        void Loop();

        virtual void Init();
        virtual void LoopInternal() = 0;
        virtual void CleanUp();
        virtual void Quit();
        
        virtual void InitializeSimulation();
        
        std::unique_ptr<Console> console_;
        uint64_t startTime_;
        bool autostep_;
        Scalar timeStep_;
        SimulationState state_;
        unsigned int maxPhysicsThreads_;
        std::unique_ptr<ThreadPool> physicsThreadPool_;

    private:
        std::unique_ptr<SimulationManager> simManager_;
        std::string title_;
        std::string dataPath_;
        double physicsTime_;
        std::unordered_map<std::string, void*> pluginHandles_;
        
        static SimulationApp* handle;
    };
}
