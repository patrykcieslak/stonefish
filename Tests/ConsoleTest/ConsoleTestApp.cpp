//
//  ConsoleTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "ConsoleTestApp.h"

ConsoleTestApp::ConsoleTestApp(std::string dataDirPath, ConsoleTestManager* sim) 
    : ConsoleSimulationApp("Console Test", dataDirPath, sim)
{
}

void ConsoleTestApp::Loop()
{
	cInfo("Press ENTER to start simulation.");
	
	while(!hasFinished())
	{
		if(!isRunning() && std::cin.get())
			StartSimulation();
	}
}