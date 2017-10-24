//
//  UnderwaterTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterTestApp.h"

#include "OpenGLTrackball.h"
#include "NativeDialog.h"
#include "Manipulator.h"
#include "Thruster.h"

UnderwaterTestApp::UnderwaterTestApp(std::string dataDirPath, std::string shaderDirPath, int width, int height, UnderwaterTestManager* sim) 
    : SimulationApp("Underwater Test", dataDirPath, shaderDirPath, width, height, sim)
{
}

void UnderwaterTestApp::DoHUD()
{
    SimulationApp::DoHUD();
    
    ui_id slider;
    slider.owner = 0;
    slider.item = 0;
    slider.index = 0;
    getSimulationManager()->setStepsPerSecond(IMGUI::getInstance()->DoSlider(slider, 5.f, 5.f, 120.f, 100.0, 2000.0, getSimulationManager()->getStepsPerSecond(), "Steps/s"));
    
    Manipulator* manip = (Manipulator*)getSimulationManager()->getEntity("Arm");
    slider.item = 1;
    manip->setDesiredJointPosition(0 , IMGUI::getInstance()->DoSlider(slider, 5.f, 100.f, 200.f, -1000.0, 1000.0, manip->getDesiredJointPosition(0), "Joint1"));
    
    slider.item = 2;
    manip->setDesiredJointPosition(1 , IMGUI::getInstance()->DoSlider(slider, 5.f, 155.f, 200.f, -1000.0, 1000.0, manip->getDesiredJointPosition(1), "Joint2"));
    
    slider.item = 3;
    manip->setDesiredJointPosition(2, IMGUI::getInstance()->DoSlider(slider, 5.f, 210.f, 200.f, -1000.0, 1000.0, manip->getDesiredJointPosition(2), "Joint3"));
    
    slider.item = 4;
    manip->setDesiredJointPosition(3, IMGUI::getInstance()->DoSlider(slider, 5.f, 265.f, 200.f, -1000.0, 1000.0, manip->getDesiredJointPosition(3), "Joint4"));
    
    slider.item = 5;
    Thruster* th = (Thruster*)getSimulationManager()->getActuator("ThrusterSway");
    th->setSetpoint(IMGUI::getInstance()->DoSlider(slider, 5.f, 350.f, 200.f, -1.0, 1.0, th->getSetpoint(), "Sway"));
    
    slider.item = 6;
    th = (Thruster*)getSimulationManager()->getActuator("ThrusterSurgePort");
    th->setSetpoint(IMGUI::getInstance()->DoSlider(slider, 5.f, 405.f, 200.f, -1.0, 1.0, th->getSetpoint(), "Surge port"));
    
    slider.item = 7;
    th = (Thruster*)getSimulationManager()->getActuator("ThrusterSurgeStarboard");
    th->setSetpoint(IMGUI::getInstance()->DoSlider(slider, 5.f, 460.f, 200.f, -1.0, 1.0, th->getSetpoint(), "Surge starboard"));
    
    slider.item = 8;
    th = (Thruster*)getSimulationManager()->getActuator("ThrusterHeaveStern");
    th->setSetpoint(IMGUI::getInstance()->DoSlider(slider, 5.f, 515.f, 200.f, -1.0, 1.0, th->getSetpoint(), "Heave stern"));
    
    slider.item = 9;
    th = (Thruster*)getSimulationManager()->getActuator("ThrusterHeaveBow");
    th->setSetpoint(IMGUI::getInstance()->DoSlider(slider, 5.f, 570.f, 200.f, -1.0, 1.0, th->getSetpoint(), "Heave bow"));
    
    //slider.item = 2;
    //manip->setDesiredJointPosition(0 , IMGUI::getInstance()->DoSlider(slider, 5.f, 60.f, 200.f, -1000.0, 1000.0, manip->getDesiredJointPosition(0), "Joint1"));
    
    
    /*
    ui_id plot;
    plot.owner = 1;
    plot.item = 0;
    plot.index = 0;
    if(getHUD()->DoTimePlot(plot, getWindowWidth()-310, 10, 300, 200, getSimulationManager()->getSensor(0), "Acceleration"))
    {
        NativeDialog* openDialog = new NativeDialog(DialogType_Save, "Save plot data...", "txt");
        openDialog->Show();
        
        char* pathToFile;
        if(openDialog->GetInput(&pathToFile) == DialogResult_OK)
        {
            const std::deque<std::unique_ptr<Sample>>& data = getSimulationManager()->getSensor(0)->getHistory();
            if(data.size() > 0)
            {
                FILE* fp;
                fp = fopen(pathToFile, "wt");
                for(int i=0; i<data.size(); i++)
                    fprintf(fp, "%1.6f\n", data[i]->getValue(0));
                fclose(fp);
            
                printf("Saved plot data to %s.\n", pathToFile);
            }
        }
        
        delete [] pathToFile;
        delete openDialog;
    }*/
}
