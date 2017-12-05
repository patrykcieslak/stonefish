//
//  UnderwaterTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterTestApp.h"

#include "OpenGLTrackball.h"
#include "NativeDialog.h"
#include "Manipulator.h"
#include "UnderwaterVehicle.h"
#include "Ocean.h"
#include "FakeRotaryEncoder.h"
#include "Accelerometer.h"

UnderwaterTestApp::UnderwaterTestApp(std::string dataDirPath, int width, int height, UnderwaterTestManager* sim) 
    : SimulationApp("Underwater Test", dataDirPath, width, height, sim)
{
    decimalTime = 12.0;
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
    manip->SetDesiredJointPosition(0 , IMGUI::getInstance()->DoSlider(slider, 5.f, 100.f, 200.f, -1.0, 1.0, manip->GetDesiredJointPosition(0), "Joint1"));
    
    slider.item = 2;
    manip->SetDesiredJointPosition(1 , IMGUI::getInstance()->DoSlider(slider, 5.f, 155.f, 200.f, -1.0, 1.0, manip->GetDesiredJointPosition(1), "Joint2"));
    
    slider.item = 3;
    manip->SetDesiredJointPosition(2, IMGUI::getInstance()->DoSlider(slider, 5.f, 210.f, 200.f, -1.0, 1.0, manip->GetDesiredJointPosition(2), "Joint3"));
    
    slider.item = 4;
    manip->SetDesiredJointPosition(3, IMGUI::getInstance()->DoSlider(slider, 5.f, 265.f, 200.f, -1.0, 1.0, manip->GetDesiredJointPosition(3), "Joint4"));
    
    UnderwaterVehicle* vehicle = (UnderwaterVehicle*)getSimulationManager()->getEntity("AUV");
    slider.item = 5;
    vehicle->SetThrusterSetpoint(0, IMGUI::getInstance()->DoSlider(slider, 5.f, 350.f, 200.f, -1.0, 1.0, vehicle->GetThrusterSetpoint(0), "Sway"));
    
    slider.item = 6;
    vehicle->SetThrusterSetpoint(1, IMGUI::getInstance()->DoSlider(slider, 5.f, 405.f, 200.f, -1.0, 1.0, vehicle->GetThrusterSetpoint(1), "Surge port"));
    
    slider.item = 7;    
    vehicle->SetThrusterSetpoint(2, IMGUI::getInstance()->DoSlider(slider, 5.f, 460.f, 200.f, -1.0, 1.0, vehicle->GetThrusterSetpoint(2), "Surge starboard"));
    
    slider.item = 8;
    vehicle->SetThrusterSetpoint(3, IMGUI::getInstance()->DoSlider(slider, 5.f, 515.f, 200.f, -1.0, 1.0, vehicle->GetThrusterSetpoint(3), "Heave stern"));
    
    slider.item = 9;
    vehicle->SetThrusterSetpoint(4, IMGUI::getInstance()->DoSlider(slider, 5.f, 570.f, 200.f, -1.0, 1.0, vehicle->GetThrusterSetpoint(4), "Heave bow"));
    
    
    Ocean* ocean = getSimulationManager()->getOcean();
    
    slider.item = 10;
    ocean->setAlgeaBloomFactor(IMGUI::getInstance()->DoSlider(slider, 5.f, 625.f, 200.f, 0.0, 1.0, ocean->getAlgeaBloomFactor(), "Algea bloom"));
    
    slider.item = 11;
    ocean->setTurbidity(IMGUI::getInstance()->DoSlider(slider, 5.f, 680.f, 200.f, 0.0, 1000.0, ocean->getTurbidity(), "Turbidity"));
    
    slider.item = 13;
    double newTime = IMGUI::getInstance()->DoSlider(slider, 5.f, 735.f, 200.f, 0.0, 24.0, decimalTime, "Time");
	if(newTime != decimalTime)
	{
		decimalTime = newTime;
		std::time_t rawTime;
		std::time(&rawTime);
		std::tm* hUtc;
		hUtc = std::gmtime(&rawTime);
		hUtc->tm_hour = trunc(decimalTime);
		hUtc->tm_min = ((int)trunc(decimalTime*60.0)) % 60;
		hUtc->tm_sec = 0;
    	OpenGLAtmosphere::getInstance()->SetSunPosition(3, 40.0, *hUtc);
	}
    
    //FakeRotaryEncoder* enc = (FakeRotaryEncoder*)getSimulationManager()->getSensor("Encoder");
    Accelerometer* acc = (Accelerometer*)getSimulationManager()->getSensor("Acc");
    
    slider.item = 12;
    std::vector<unsigned short> dims;
    dims.push_back(2);
    IMGUI::getInstance()->DoTimePlot(slider, getWindowWidth()-310, 10, 300, 200, acc, dims, "Test", new btScalar[2]{-20.f,20.f});
    
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
