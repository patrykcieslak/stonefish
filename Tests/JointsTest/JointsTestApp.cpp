//
//  JointsTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "JointsTestApp.h"

#include "OpenGLTrackball.h"

JointsTestApp::JointsTestApp(std::string dataDirPath, std::string shaderDirPath, int width, int height, JointsTestManager* sim) 
    : SimulationApp("Joints Test", dataDirPath, shaderDirPath, width, height, sim)
{
}