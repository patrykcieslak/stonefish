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
        SimulationApp(std::string name, std::string dataDirPath, SimulationManager* sim);
        virtual ~SimulationApp();
        
        void Run(bool autostart = true);
        
        virtual bool hasGraphics() = 0;
        SimulationManager* getSimulationManager();
        double getPhysicsTime();
        bool isRunning();
        bool hasFinished();
        std::string getDataPath();
        std::string getName();
        Console* getConsole();
        
        static SimulationApp* getApp();
        
    protected:
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
