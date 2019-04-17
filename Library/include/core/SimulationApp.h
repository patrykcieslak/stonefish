//
//  SimulationApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SimulationApp__
#define __Stonefish_SimulationApp__

#include "StonefishCommon.h"

namespace sf
{
    class Console;
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
        
    protected:
        virtual void Init() = 0;
        virtual void Loop() = 0;
        virtual void CleanUp();
        virtual void Quit();
        
        virtual void InitializeSimulation();
        virtual void StartSimulation();
        virtual void ResumeSimulation();
        virtual void StopSimulation();
        
        Console* console;
        
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
