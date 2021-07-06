/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  UnderwaterTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2020 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterTestApp.h"

#include <actuators/Servo.h>
#include <actuators/Thruster.h>
#include <actuators/VariableBuoyancy.h>
#include <core/Robot.h>
#include <sensors/scalar/Accelerometer.h>
#include <sensors/scalar/IMU.h>
#include <sensors/scalar/DVL.h>
#include <sensors/vision/FLS.h>
#include <sensors/vision/SSS.h>
#include <graphics/IMGUI.h>
#include <utils/SystemUtil.hpp>
#include <comms/USBL.h>
#include <core/Console.h>

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

    /*sf::Uid id;
    id.owner = 10;
    id.item = 0;

    sf::Servo* srv1 = (sf::Servo*)getSimulationManager()->getActuator("LOLO/Servo1");
    sf::Servo* srv2 = (sf::Servo*)getSimulationManager()->getActuator("LOLO/Servo2");
    sf::Scalar sp = getGUI()->DoSlider(id, 180.f, 10.f, 150.f, sf::Scalar(-1), sf::Scalar(1), srv1->getPosition(), "Servo1");
    srv1->setDesiredPosition(sp);
    srv2->setDesiredPosition(sp);

    id.item = 1;

    sf::Thruster* th1 = (sf::Thruster*)getSimulationManager()->getActuator("LOLO/ThrusterSurge1");
    sf::Thruster* th2 = (sf::Thruster*)getSimulationManager()->getActuator("LOLO/ThrusterSurge2");
    sf::Scalar sp2 = getGUI()->DoSlider(id, 180.f, 60.f, 150.f, sf::Scalar(-1), sf::Scalar(1), th1->getSetpoint(), "Surge");
    th1->setSetpoint(sp2);
    th2->setSetpoint(sp2);*/
    
    /*
    sf::Uid id;
    id.owner = 10;
    id.item = 0;

    std::vector<unsigned short> dims;
    dims.push_back(2);
    dims.push_back(6);
    //dims.push_back(6);
    sf::ScalarSensor* dvl = (sf::ScalarSensor*)getSimulationManager()->getSensor("GIRONA500/dvl");
    getGUI()->DoTimePlot(id, 200, 20, 400, 200, dvl, dims, "Water velocity");

    getSimulationManager()->getOcean()->EnableCurrents();
*/
/*
    std::vector<unsigned short> dims;
    dims.push_back(6);
    dims.push_back(7);
    dims.push_back(8);
    sf::IMU* imu = (sf::IMU*)getSimulationManager()->getSensor("GIRONA500/imu_filter");
    getGUI()->DoTimePlot(id, 200, 20, 400, 200, imu, dims, "Acceleration");
*/
    /*sf::Accelerometer* acc = (sf::Accelerometer*)getSimulationManager()->getSensor("GIRONA500/acc");
    dims.push_back(0);
    dims.push_back(1);
    dims.push_back(2);
    getGUI()->DoTimePlot(id, 200, 20, 400, 200, acc, dims, "Acceleration");*/

/*    
    sf::FLS* fls = (sf::FLS*)getSimulationManager()->getSensor("FLS");  //->getRobot("GIRONA500")->getSensor("FLS");
    sf::Scalar range = (getGUI()->DoSlider(id, 180.f, 10.f, 150.f, sf::Scalar(1.0), sf::Scalar(100.0), fls->getRangeMax(), "FLS Range[m]"));
    fls->setRangeMax(range);
    
    id.item = 1;
    sf::Scalar gain = (getGUI()->DoSlider(id, 180.f, 60.f, 150.f, sf::Scalar(0.1), sf::Scalar(10.0), fls->getGain(), "FLS Gain"));
    fls->setGain(gain);
    
    id.owner = 10;
    id.item = 2;
    
    sf::SSS* sss = (sf::SSS*)getSimulationManager()->getRobot("GIRONA500")->getSensor("SSS");
    sf::Scalar range2 = (getGUI()->DoSlider(id, 180.f, 110.f, 150.f, sf::Scalar(1.0), sf::Scalar(100.0), sss->getRangeMax(), "SSS Range[m]"));
    sss->setRangeMax(range2);
    
    id.item = 3;
    sf::Scalar gain2 = (getGUI()->DoSlider(id, 180.f, 160.f, 150.f, sf::Scalar(0.1), sf::Scalar(10.0), sss->getGain(), "SSS Gain"));
    sss->setGain(gain2);
  */  
    /*
    sf::USBL* usbl = (sf::USBL*)getSimulationManager()->getComm("GIRONA500/USBL");
    std::map<uint64_t, std::pair<sf::Scalar, sf::Vector3>>& transPos = usbl->getTransponderPositions();
    std::map<uint64_t, std::pair<sf::Scalar, sf::Vector3>>::iterator it = transPos.begin();
    sf::Vector3 pos = it->second.second;
    uint64_t id = it->first;
    
    printf("Transponder %ld: %1.3lf, %1.3lf, %1.3lf\n", id, pos.getX(), pos.getY(), pos.getZ());
    */
    
    /*
    sf::Vector3 pos;
    std::string frame;
    ((sf::AcousticModem*)getSimulationManager()->getComm("Modem"))->getPosition(pos, frame); 
    cInfo("Modem pos: %1.3lf, %1.3lf, %1.3lf frame: %s", pos.getX(), pos.getY(), pos.getZ(), frame.c_str());
    */
    
    /*
    sf::Uid id;
    id.owner = 5;
    id.item = 0;
    
    std::vector<std::string> robotNames;
    robotNames.push_back("Girona500");
    robotNames.push_back("Girona1000");
    robotNames.push_back("Girona2000");
    option = getGUI()->DoComboBox(id, 180.f, 200.f, 150.f, robotNames, option, "Trackball center");
    */
    
    //sf::VariableBuoyancy* vbs = (sf::VariableBuoyancy*)getSimulationManager()->getRobot("GIRONA500")->getActuator("VBS");
    //vbs->setFlowRate((getGUI()->DoSlider(id, 180.f, 10.f, 150.f, sf::Scalar(-0.0005), sf::Scalar(0.0005), vbs->getFlowRate(), "VBS flow rate", 4)));
    /*id.item = 1;
    
    sf::Thruster* th = (sf::Thruster*)getSimulationManager()->getActuator("GIRONA500/ThrusterHeaveStern");
    sf::Thruster* th2 = (sf::Thruster*)getSimulationManager()->getActuator("GIRONA500/ThrusterHeaveBow");
    sf::Scalar sp = (getGUI()->DoSlider(id, 180.f, 250.f, 150.f, sf::Scalar(-1), sf::Scalar(1), th->getSetpoint(), "Heave"));
    th->setSetpoint(sp);
    th2->setSetpoint(sp);*/
    
    /*
    sf::Thruster* th = (sf::Thruster*)getSimulationManager()->getRobot("GIRONA500")->getActuator("ThrusterSurgePort");
    sf::Scalar sp = (getGUI()->DoSlider(id, 180.f, 10.f, 150.f, sf::Scalar(-1), sf::Scalar(1), th->getSetpoint(), "ThrusterSurgePort"));
    th->setSetpoint(sp);*/
}
