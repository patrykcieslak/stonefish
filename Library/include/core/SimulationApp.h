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
//  Copyright (c) 2012-2025 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SimulationApp__
#define __Stonefish_SimulationApp__

#include "StonefishCommon.h"
#include "core/Console.h"

//Console output aliases
#define cInfo(format, ...)     sf::SimulationApp::getApp()->getConsole()->Print(sf::MessageType::INFO, format, ##__VA_ARGS__)
#define cWarning(format, ...)  sf::SimulationApp::getApp()->getConsole()->Print(sf::MessageType::WARNING, format, ##__VA_ARGS__)
#define cError(format, ...)    sf::SimulationApp::getApp()->getConsole()->Print(sf::MessageType::ERROR, format, ##__VA_ARGS__)
#define cCritical(format, ...) {sf::SimulationApp::getApp()->getConsole()->Print(sf::MessageType::CRITICAL, format, ##__VA_ARGS__);abort();}

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
         \param name a name for the application
         \param dataDirPath a path to the directory containing simulation data
         \param sim a pointer to the simulation manager
         */
        SimulationApp(std::string name, std::string dataDirPath, SimulationManager* sim);
        
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
        
        //! A method returning simulation state.
        SimulationState getState() const;

        //! A method returning a pointer to the simulation manager.
        SimulationManager* getSimulationManager();
        
        //! A method returning the physics computation time.
        double getPhysicsTime();
          
        //! A method returning the path to the directory containing simulation data.
        std::string getDataPath();
        
        //! A method returning the name of the application.
        std::string getName();
        
        //! A method returning a pointer to the console associated with the application.
        Console* getConsole();

        //! A method informing if the application is graphical.
        virtual bool hasGraphics() = 0;
        
        //! A static method returning the pointer to the currently running application.
        static SimulationApp* getApp();
        
    protected:
        void Loop();

        virtual void Init();
        virtual void LoopInternal() = 0;
        virtual void CleanUp();
        virtual void Quit();
        
        virtual void InitializeSimulation();
        
        Console* console;
        uint64_t startTime;
        bool autostep_;
        Scalar timeStep_;
        SimulationState state_;

    private:
        SimulationManager* simulation;
        std::string appName;
        std::string dataPath;
        double physicsTime;
        
        static SimulationApp* handle;
    };
}

#endif
