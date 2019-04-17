//
//  UnderwaterTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterTestApp.h"

#include <actuators/Thruster.h>
#include <core/Robot.h>
#include <graphics/IMGUI.h>
#include <utils/SystemUtil.hpp>

UnderwaterTestApp::UnderwaterTestApp(std::string dataDirPath, sf::RenderSettings s, sf::HelperSettings h, UnderwaterTestManager* sim)
    : GraphicalSimulationApp("Underwater Test", dataDirPath, s, h, sim)
{
}

void UnderwaterTestApp::InitializeGUI()
{
    largePrint = new sf::OpenGLPrinter(sf::GetShaderPath() + std::string(STANDARD_FONT_NAME), 64.0);
}

void UnderwaterTestApp::DoHUD()
{
    GraphicalSimulationApp::DoHUD();
    
    largePrint->Print("Test", glm::vec4(1.f,1.f,1.f,1.f), 10.0, 10.0, 64.0);
    
    sf::ui_id id;
    id.owner = 5;
    id.index = 0;
    id.item = 0;
    
    sf::Thruster* th = (sf::Thruster*)getSimulationManager()->getRobot("GIRONA500")->getActuator("ThrusterHeaveStern");
    sf::Thruster* th2 = (sf::Thruster*)getSimulationManager()->getRobot("GIRONA500")->getActuator("ThrusterHeaveBow");
    sf::Scalar sp = (getGUI()->DoSlider(id, 180.f, 5.f, 150.f, sf::Scalar(-1), sf::Scalar(1), th->getSetpoint(), "ThrusterHeaveStern"));
    th->setSetpoint(sp);
    th2->setSetpoint(sp);
}
