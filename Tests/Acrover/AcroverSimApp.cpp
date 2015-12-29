//
//  AcroverSimApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "AcroverSimApp.h"

#include "OpenGLTrackball.h"
#include "NativeDialog.h"
#include "DCMotor.h"
#include "MISOStateSpaceController.h"
#include "AcroverTiltController.h"

AcroverSimApp::AcroverSimApp(int width, int height, AcroverSimManager* sim) : SimulationApp("Acrover Simulation", width, height, sim)
{
    turning = btScalar(0.);
    speed = btScalar(0.);
}

void AcroverSimApp::ProcessInputs()
{
    /*if(joystickAxes != NULL)
    {
        int16_t deadband = 1000;
        
        if(abs(joystickAxes[1]) > deadband)
        {
            if(joystickAxes[1] < 0)
                speed = btScalar(joystickAxes[1]+deadband)/btScalar(INT16_MAX - deadband);
            else
                speed = btScalar(joystickAxes[1]-deadband)/btScalar(INT16_MAX - deadband);
        }
        else
            speed = btScalar(0.);
        
        MISOStateSpaceController* longitudinal = (MISOStateSpaceController*)getSimulationManager()->getController("Longitudinal");
        longitudinal->setReferenceValue(2, speed * 5.0);
        
        if(abs(joystickAxes[2]) > deadband)
        {
            if(joystickAxes[2] < 0)
                turning = btScalar(joystickAxes[2]+deadband)/btScalar(INT16_MAX - deadband);
            else
                turning = btScalar(joystickAxes[2]-deadband)/btScalar(INT16_MAX - deadband);
        }
        else
            turning = btScalar(0.);
        
        AcroverTiltController* lateral = (AcroverTiltController*)getSimulationManager()->getController("Lateral");
        lateral->setDesiredTilt(turning * 0.02);
    }*/
}

void AcroverSimApp::MouseDown(SDL_Event* event)
{
    GLfloat xPos = (GLfloat)(event->motion.x-getWindowWidth()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    GLfloat yPos = -(GLfloat)(event->motion.y-getWindowHeight()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseDown(xPos, yPos);
}

void AcroverSimApp::MouseUp(SDL_Event* event)
{
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseUp();
}

void AcroverSimApp::MouseMove(SDL_Event* event)
{
    GLfloat xPos = (GLfloat)(event->motion.x-getWindowWidth()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    GLfloat yPos = -(GLfloat)(event->motion.y-getWindowHeight()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseMove(xPos, yPos);
}

void AcroverSimApp::MouseScroll(SDL_Event* event)
{
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseScroll(event->wheel.y * -1.f);
}

void AcroverSimApp::DoHUD()
{
    SimulationApp::DoHUD();
    
    /*ui_id slider;
    slider.owner = 1;
    slider.item = 1;
    slider.index = 0;
    
    DCMotor* motorCW = (DCMotor*)getSimulationManager()->getActuator("DCXWheel");
    IMGUI::getInstance()->DoSlider(slider, 5, 70, 100, 5, 5, 20, -1.0, 1.0, speed, "Speed");
    
    slider.item = 2;
    
    DCMotor* motorCL = (DCMotor*)getSimulationManager()->getActuator("DCXLever");
    IMGUI::getInstance()->DoSlider(slider, 5, 120, 100, 5, 5, 20, -1.0, 1.0, turning, "Turning");
    */
    ui_id plot;
    plot.owner = 1;
    plot.item = 0;
    plot.index = 0;
    
    std::vector<unsigned short> dims;
    dims.push_back(0);
    dims.push_back(1);
    dims.push_back(2);
    
    if(IMGUI::getInstance()->DoTimePlot(plot, getWindowWidth()-310, 10, 300, 200, getSimulationManager()->getSensor("AHRS"), dims, "AHRS"))
    {
        StopSimulation();
        
        NativeDialog* openDialog = new NativeDialog(DialogType_Save, "Save plot data...", "txt");
        openDialog->Show();
     
        char* pathToFile;
        if(openDialog->GetInput(&pathToFile) == DialogResult_OK)
        {
            getSimulationManager()->getSensor("AHRS")->SaveMeasurementsToTextFile(pathToFile);
        }
     
        delete [] pathToFile;
        delete openDialog;
        
        ResumeSimulation();
    }
    
    dims.clear();
    dims.push_back(0);
    dims.push_back(1);
    
    plot.item = 3;
    if(IMGUI::getInstance()->DoTimePlot(plot, getWindowWidth()-310, 220, 300, 200, getSimulationManager()->getSensor("EncoderPendulum"), dims, "Pendulum Encoder"))
    {
        StopSimulation();
        
        NativeDialog* openDialog = new NativeDialog(DialogType_Save, "Save plot data...", "txt");
        openDialog->Show();
    
        char* pathToFile;
        if(openDialog->GetInput(&pathToFile) == DialogResult_OK)
        {
            getSimulationManager()->getSensor("EncoderPendulum")->SaveMeasurementsToTextFile(pathToFile);
        }
    
        delete [] pathToFile;
        delete openDialog;
        
        ResumeSimulation();
    }
    
    plot.item = 4;
    if(IMGUI::getInstance()->DoTimePlot(plot, 20, 300, 300, 200, getSimulationManager()->getSensor("EncoderWheel"), dims, "Wheel Encoder"))
    {
        StopSimulation();
        
        NativeDialog* openDialog = new NativeDialog(DialogType_Save, "Save plot data...", "txt");
        openDialog->Show();
        
        char* pathToFile;
        if(openDialog->GetInput(&pathToFile) == DialogResult_OK)
        {
            getSimulationManager()->getSensor("EncoderWheel")->SaveMeasurementsToTextFile(pathToFile);
        }
        
        delete [] pathToFile;
        delete openDialog;
        
        ResumeSimulation();
    }

    ui_id button;
    button.owner = 1;
    button.item = 4;
    button.index = 0;
    
    if(IMGUI::getInstance()->DoButton(button, 5, 180, 110, 30, "Save Trajectory"))
    {
        StopSimulation();
        
        NativeDialog* openDialog = new NativeDialog(DialogType_Save, "Save trajectory data...", "oct");
        openDialog->Show();
        
        char* pathToFile;
        if(openDialog->GetInput(&pathToFile) == DialogResult_OK)
        {
            getSimulationManager()->getSensor("Trajectory")->SaveMeasurementsToTextFile(pathToFile);
        }
        
        delete [] pathToFile;
        delete openDialog;
        
        ResumeSimulation();
    }
    
    button.item = 5;
    if(IMGUI::getInstance()->DoButton(button, 5, 215, 110, 30, "Save Contact"))
    {
        StopSimulation();
        
        NativeDialog* openDialog = new NativeDialog(DialogType_Save, "Save contact data...", "oct");
        openDialog->Show();
        
        char* pathToFile;
        if(openDialog->GetInput(&pathToFile) == DialogResult_OK)
        {
            getSimulationManager()->getContact(0)->SaveContactDataToOctaveFile(pathToFile);
        }
        
        delete [] pathToFile;
        delete openDialog;
        
        ResumeSimulation();
    }

}