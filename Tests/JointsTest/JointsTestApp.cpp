//
//  JointsTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "JointsTestApp.h"

#include <graphics/OpenGLTrackball.h>

JointsTestApp::JointsTestApp(std::string dataDirPath, int width, int height, JointsTestManager* sim) 
    : GraphicalSimulationApp("Joints Test", dataDirPath, width, height, sim)
{
}