//
//  SlidingTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "SlidingTestApp.h"

SlidingTestApp::SlidingTestApp(std::string dataDirPath, RenderSettings s, SlidingTestManager* sim) : GraphicalSimulationApp("Sliding Test", dataDirPath, s, sim)
{
}

void SlidingTestApp::DoHUD()
{
    GraphicalSimulationApp::DoHUD();
    
    ui_id plot;
    plot.owner = 1;
    plot.item = 0;
    plot.index = 0;
  
    std::vector<unsigned short> dims;
    dims.push_back(2);
    getGUI()->DoTimePlot(plot, getWindowWidth()-310, 10, 300, 200, (ScalarSensor*)getSimulationManager()->getSensor(0), dims, "Height");
}
