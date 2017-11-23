//
//  FallingTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "FallingTestApp.h"

#include "Manipulator.h"
#include "NativeDialog.h"
#include "Accelerometer.h"

FallingTestApp::FallingTestApp(std::string dataDirPath, int width, int height, FallingTestManager* sim) 
    : SimulationApp("Falling Test", dataDirPath, width, height, sim)
{
    checked = false;
}

void FallingTestApp::DoHUD()
{
    SimulationApp::DoHUD();
    
	ui_id slider;
    slider.owner = 1;
    slider.item = 0;
	slider.index = 0;
	getSimulationManager()->setStepsPerSecond(IMGUI::getInstance()->DoSlider(slider, 5.f, 5.f, 120.f, 100.0, 2000.0, getSimulationManager()->getStepsPerSecond(), "Steps/s"));
    
	slider.item = 1;
	std::vector<unsigned short> dims;
    dims.push_back(2);
	//Accelerometer* acc = (Accelerometer*)getSimulationManager()->getSensor("Acc");
    //IMGUI::getInstance()->DoTimePlot(slider, getWindowWidth()-310, 10, 300, 200, acc, dims, "Acceleration", new btScalar[2]{0.0, 1000.0});
    
    //Left side
    /*ui_id button;
    button.owner = 1;
    button.item = 1;
    button.index = 0;
    IMGUI::getInstance()->DoButton(button, 5.f, 59.f, 120.f, 30.f, "Restart");
	
    ui_id check;
    check.owner = 1;
    check.item = 2;
    check.index = 0;
    checked = IMGUI::getInstance()->DoCheckBox(check, 5.f, 95.f, 120.f, checked, "Test");
    
    IMGUI::getInstance()->DoProgressBar(5.f, 125.f, 120.f, 0.3f, "Progress");
    */
	
	
	/*Manipulator* manipA = (Manipulator*)getSimulationManager()->getEntity("ArmA");
    ui_id slider;
    
	slider.owner = 1;
    slider.index = 0;
    
	slider.item = 1;
	manipA->setDesiredJointPosition(0, IMGUI::getInstance()->DoSlider(slider, 5.f, 55.f, 100.0, -10.f, 10.f, manipA->getDesiredJointPosition(0), "Joint 0"));
    */
	
    /*
	 * Look l = solid->getLook();
    
	 * sliderMat.item = 2;
	l.data[1] = (GLfloat)IMGUI::getInstance()->DoSlider(sliderMat, 5.f, 105.f, 100.0, 5.f, 5.f, 20.f, 0.0, 1.0, l.data[1], "Roughness");
    
    sliderMat.item = 3;
    l.data[2] = (GLfloat)IMGUI::getInstance()->DoSlider(sliderMat, 5.f, 155.f, 100.0, 5.f, 5.f, 20.f, 0.0, 3.0, l.data[2], "IOR");
    
	*/
    //Right side
    /*ui_id plot;
    plot.owner = 1;
    plot.item = 0;
    plot.index = 0;
    std::vector<unsigned short> dims;
    dims.push_back(2);
	dims.push_back(0);
    
    if(IMGUI::getInstance()->DoTimePlot(plot, getWindowWidth()-310, getWindowHeight() - 240, 300, 200, (SimpleSensor*)getSimulationManager()->getSensor(0), dims, "Height"))
    {
        NativeDialog* openDialog = new NativeDialog(DialogType_Save, "Save plot data...", "txt");
        openDialog->Show();
        
        char* pathToFile;
        if(openDialog->GetInput(&pathToFile) == DialogResult_OK)
        {
            const std::deque<Sample*>& data = ((SimpleSensor*)getSimulationManager()->getSensor(0))->getHistory();
            if(data.size() > 0)
            {
                FILE* fp;
                fp = fopen(pathToFile, "wt");
                
                for(int i=0; i<data.size(); i++)
                    fprintf(fp, "%1.6lf\t%1.6lf\n", data[i]->getTimestamp(), data[i]->getValue(2));
                fclose(fp);
                
                cInfo("Saved plot data to %s.\n", pathToFile);
            }
        }
        
        delete [] pathToFile;
        delete openDialog;
    }*/
}
