//
//  XMLSimulationApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/10/12.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_XMLSimulationApp__
#define __Stonefish_XMLSimulationApp__

#include "SimulationApp.h"

class XMLSimulationApp : public SimulationApp
{
public:    
    XMLSimulationApp(std::string name, std::string xmlPath, std::string dataDirPath, std::string shaderDirPath, int windowWidth, int windowHeight, SimulationManager* sim);
    
private:
    
};

#endif