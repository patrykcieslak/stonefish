//
//  ConsoleSimulationApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ConsoleSimulationApp__
#define __Stonefish_ConsoleSimulationApp__

#include <SDL2/SDL_thread.h>
#include "core/SimulationApp.h"

namespace sf
{

    class Console;
    
/*!
    A class that defines a console application interface.
 */
class ConsoleSimulationApp : public SimulationApp
{
public:
    ConsoleSimulationApp(std::string name, std::string dataDirPath, SimulationManager* sim);
    virtual ~ConsoleSimulationApp();
    
	bool hasGraphics();
	
protected:
	virtual void Loop();
	void StartSimulation();
    void ResumeSimulation();
    void StopSimulation();
    
private:
    void Init();
    
    SDL_Thread* simulationThread;
    
	static int RunSimulation(void* data);
};

typedef struct
{
    SimulationApp* app;
}
ConsoleSimulationThreadData;

}
    
#endif
