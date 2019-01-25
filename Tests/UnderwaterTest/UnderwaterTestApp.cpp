//
//  UnderwaterTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterTestApp.h"

#include <Stonefish/actuators/Thruster.h>
#include <Stonefish/core/Robot.h>
#include <Stonefish/graphics/IMGUI.h>

UnderwaterTestApp::UnderwaterTestApp(std::string dataDirPath, sf::RenderSettings s, sf::HelperSettings h, UnderwaterTestManager* sim)
    : GraphicalSimulationApp("Underwater Test", dataDirPath, s, h, sim)
{
}

void UnderwaterTestApp::DoHUD()
{
    GraphicalSimulationApp::DoHUD();
    
    sf::ui_id id;
    id.owner = 5;
    id.index = 0;
    id.item = 0;
    
    sf::Thruster* th = (sf::Thruster*)getSimulationManager()->getRobot("GIRONA500")->getActuator("ThrusterSway");
    th->setSetpoint(getGUI()->DoSlider(id, 180.f, 5.f, 150.f, sf::Scalar(-1), sf::Scalar(1), th->getSetpoint(), "Sway"));
}
