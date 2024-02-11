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
//  Copyright (c) 2012-2022 Patryk Cieslak. All rights reserved.
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
         \param autostart a flag determining if the simulation should automatically start running
         */
        void Run(bool autostart = true);
        
        //! A method informing if the application is graphical.
        virtual bool hasGraphics() = 0;
        
        //! A method returning a pointer to the simulation manager.
        SimulationManager* getSimulationManager();
        
        //! A method returning the physics computation time.
        double getPhysicsTime();
        
        //! A method informing if the simulation is running.
        bool isRunning();

        //! A method informing if the simulation has finished.
        bool hasFinished();
          
        //! A method returning the path to the directory containing simulation data.
        std::string getDataPath();
        
        //! A method returning the name of the application.
        std::string getName();
        
        //! A method returning a pointer to the console associated with the application.
        Console* getConsole();
        
        //! A static method returning the pointer to the currently running application.
        static SimulationApp* getApp();
        
        void QuitWrapper();
        
    protected:
        void Loop();

        virtual void Init();
        virtual void LoopInternal() = 0;
        virtual void CleanUp();
        virtual void Quit();
        
        virtual void InitializeSimulation();
        virtual void StartSimulation();
        virtual void ResumeSimulation();
        virtual void StopSimulation();
        
        Console* console;
        uint64_t startTime;
        
    private:
        SimulationManager* simulation;
        std::string appName;
        std::string dataPath;
        bool finished;
        bool running;
        double physicsTime;
        
        static SimulationApp* handle;
    };
}

#endif
