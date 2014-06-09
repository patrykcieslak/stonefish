//
//  FallingTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "FallingTestApp.h"

#include "OpenGLTrackball.h"
#include "NativeDialog.h"

FallingTestApp::FallingTestApp(int width, int height, FallingTestManager* sim) : SimulationApp("Falling Test", width, height, sim)
{
    setSimulationSpeed(1.0);
    checked = false;
    radioOption = 0;
}

void FallingTestApp::MouseDown(SDL_Event* event)
{
    GLfloat xPos = (GLfloat)(event->motion.x-getWindowWidth()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    GLfloat yPos = -(GLfloat)(event->motion.y-getWindowHeight()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseDown(xPos, yPos);
}

void FallingTestApp::MouseUp(SDL_Event* event)
{
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseUp();
}

void FallingTestApp::MouseMove(SDL_Event* event)
{
    GLfloat xPos = (GLfloat)(event->motion.x-getWindowWidth()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    GLfloat yPos = -(GLfloat)(event->motion.y-getWindowHeight()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseMove(xPos, yPos);
}

void FallingTestApp::MouseScroll(SDL_Event* event)
{
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseScroll(event->wheel.y * -1.f);
}

void FallingTestApp::DoHUD()
{
    SimulationApp::DoHUD();
    
    //Left side
    ui_id button;
    button.owner = 1;
    button.item = 1;
    button.index = 0;
    IMGUI::getInstance()->DoButton(button, 5.f, 59.f, 110.f, 30.f, "Restart");
    
    ui_id check;
    check.owner = 1;
    check.item = 2;
    check.index = 0;
    checked = IMGUI::getInstance()->DoCheckBox(check, 5.f, 95.f, checked, "Test");
    
    ui_id radio;
    radio.owner = 1;
    radio.item = 3;
    radio.index = 0;
    std::vector<std::string> items;
    items.push_back("Item1");
    items.push_back("Item12");
    items.push_back("Item123");
    radioOption = IMGUI::getInstance()->DoRadioGroup(radio, 5.f, 125.f, radioOption, items, "Title");
    
    IMGUI::getInstance()->DoProgressBar(5.f, 220.f, 100.f, 10.f, 0.3, "Test");
    
    IMGUI::getInstance()->DoGauge(5.f, 265.f, 100.f, 10.0, new btScalar[2]{0.0,100.0}, new btScalar[2]{10.0, 80.0}, "Test");
    
    //Right side
    ui_id plot;
    plot.owner = 1;
    plot.item = 0;
    plot.index = 0;
    std::vector<unsigned short> dims;
    dims.push_back(2);
    
    
    if(IMGUI::getInstance()->DoTimePlot(plot, getWindowWidth()-310, getWindowHeight() - 240, 300, 200, getSimulationManager()->getSensor(0), dims, "Height"))
    {
        NativeDialog* openDialog = new NativeDialog(DialogType_Save, "Save plot data...", "txt");
        openDialog->Show();
        
        char* pathToFile;
        if(openDialog->GetInput(&pathToFile) == DialogResult_OK)
        {
            const std::deque<Sample*>& data = getSimulationManager()->getSensor(0)->getHistory();
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
    }
}
