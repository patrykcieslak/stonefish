//
//  AcrobotTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "AcrobotTestApp.h"

#include "OpenGLTrackball.h"
#include "DCMotor.h"
#include "NativeDialog.h"

AcrobotTestApp::AcrobotTestApp(int width, int height, AcrobotTestManager* sim) : SimulationApp("Acrobot Test", width, height, sim)
{
}

void AcrobotTestApp::MouseDown(SDL_Event* event)
{
    GLfloat xPos = (GLfloat)(event->motion.x-getWindowWidth()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    GLfloat yPos = -(GLfloat)(event->motion.y-getWindowHeight()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseDown(xPos, yPos);
}

void AcrobotTestApp::MouseUp(SDL_Event* event)
{
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseUp();
}

void AcrobotTestApp::MouseMove(SDL_Event* event)
{
    GLfloat xPos = (GLfloat)(event->motion.x-getWindowWidth()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    GLfloat yPos = -(GLfloat)(event->motion.y-getWindowHeight()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseMove(xPos, yPos);
}

void AcrobotTestApp::MouseScroll(SDL_Event* event)
{
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseScroll(event->wheel.y * -1.f);
}

void AcrobotTestApp::DoHUD()
{
    SimulationApp::DoHUD();
    
    char buffer[128];
    GLfloat white[4] = {1.f,1.f,1.f,1.f};

    ui_id label1;
    label1.owner = 1;
    label1.item = 0;
    label1.index = 0;
    
    sprintf(buffer, "Motor Torque: %1.3lf [Nm]", ((DCMotor*)getSimulationManager()->getActuator(0))->getTorque());
    getHUD()->DoLabel(label1, 90, getWindowHeight()-50, white, buffer);
    
    ui_id plot;
    plot.owner = 1;
    plot.item = 0;
    plot.index = 0;
    
    std::vector<unsigned short> dims;
    dims.push_back(0);
    dims.push_back(1);
    dims.push_back(2);
    
    if(getHUD()->DoTimePlot(plot, getWindowWidth()-310, 10, 300, 200, getSimulationManager()->getSensor(2), dims, "Arm1"))
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
                    fprintf(fp, "%1.6f\t%1.6f\n", data[i]->getTimestamp(), data[i]->getValue(0));
                fclose(fp);
     
                cInfo("Saved plot data to %s.\n", pathToFile);
            }
        }
     
        delete [] pathToFile;
        delete openDialog;
     }
    
    plot.owner = 1;
    plot.item = 1;
    plot.index = 0;
    //getHUD()->DoTimePlot(plot, getWindowWidth()-310, 220, 300, 200, getSimulationManager()->getSensor(1), "Arm2");
    
}