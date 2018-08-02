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
#include "ForceTorque.h"
#include "Torque.h"

AcrobotTestApp::AcrobotTestApp(std::string dataDirPath, int width, int height, AcrobotTestManager* sim) : SimulationApp("Acrobot Test", dataDirPath, width, height, sim)
{
}

void AcrobotTestApp::DoHUD()
{
    SimulationApp::DoHUD();
    
    ui_id slider1;
    slider1.owner = 0;
    slider1.item = 3;
    slider1.index = 0;
    getSimulationManager()->setStepsPerSecond(IMGUI::getInstance()->DoSlider(slider1, 5.f, 5.f, 120.f, 100.0, 2000.0, getSimulationManager()->getStepsPerSecond(), "Steps/s"));
    
    char buffer[128];
    GLfloat white[4] = {1.f,1.f,1.f,1.f};

    ui_id label1;
    label1.owner = 1;
    label1.item = 0;
    label1.index = 0;
    
    //sprintf(buffer, "Motor Torque: %1.3lf [Nm]", ((DCMotor*)getSimulationManager()->getActuator(0))->getTorque());
    //getHUD()->DoLabel(label1, 90, getWindowHeight()-50, white, buffer);
    
    ui_id plot;
    plot.owner = 1;
    plot.item = 0;
    plot.index = 0;
    
    //IMGUI::getInstance()->DoXYPlot(plot, getWindowWidth()-310, getWindowHeight() - 240, 300, 200, getSimulationManager()->getSensor(1), 0, getSimulationManager()->getSensor(1), 1, "Arm1");
    
    std::vector<unsigned short> dims;
    dims.push_back(0);
    dims.push_back(1);
    
    btVector3 force, torque;
    
    FeatherstoneEntity* fe = (FeatherstoneEntity*)getSimulationManager()->getEntity("Manipulator1");
    /*fe->getJointFeedback(1, force, torque);
    bt8Vector3 baseF = fe->getMultiBody()->getLinkTorque(0);
    
    std::cout << "FT1: " << force.x() << ", " << force.y() << ", " << force.z() << ", " << torque.x() << ", " << torque.y() << ", " << torque.z() << std::endl;
    */
    ForceTorque* ft = (ForceTorque*)getSimulationManager()->getSensor("FT1");
    std::cout << "FT1: " << ft->getLastSample().data[0] << ", " << ft->getLastSample().data[1] << ", " << ft->getLastSample().data[2] << ", "
              << ft->getLastSample().data[3] << ", " << ft->getLastSample().data[4] << ", " << ft->getLastSample().data[5] << std::endl;
 
    ft = (ForceTorque*)getSimulationManager()->getSensor("FT2");
    std::cout << "FT2: " << ft->getLastSample().data[0] << ", " << ft->getLastSample().data[1] << ", " << ft->getLastSample().data[2] << ", "
              << ft->getLastSample().data[3] << ", " << ft->getLastSample().data[4] << ", " << ft->getLastSample().data[5] << std::endl;
 
    ft = (ForceTorque*)getSimulationManager()->getSensor("FT3");
    std::cout << "FT3: " << ft->getLastSample().data[0] << ", " << ft->getLastSample().data[1] << ", " << ft->getLastSample().data[2] << ", "
              << ft->getLastSample().data[3] << ", " << ft->getLastSample().data[4] << ", " << ft->getLastSample().data[5] << std::endl;
 
    ft = (ForceTorque*)getSimulationManager()->getSensor("FT4");
    std::cout << "FT4: " << ft->getLastSample().data[0] << ", " << ft->getLastSample().data[1] << ", " << ft->getLastSample().data[2] << ", "
              << ft->getLastSample().data[3] << ", " << ft->getLastSample().data[4] << ", " << ft->getLastSample().data[5] << std::endl;
 
    ft = (ForceTorque*)getSimulationManager()->getSensor("FT5");
    std::cout << "FT5: " << ft->getLastSample().data[0] << ", " << ft->getLastSample().data[1] << ", " << ft->getLastSample().data[2] << ", "
              << ft->getLastSample().data[3] << ", " << ft->getLastSample().data[4] << ", " << ft->getLastSample().data[5] << std::endl;
 
    std::cout << std::endl;
   // Torque* tau = (Torque*)getSimulationManager()->getSensor("Torque");
    //std::cout << "Tau: " << tau->getLastSample().data[0] << std::endl;
 
    /*if(IMGUI::getInstance()->DoTimePlot(plot, getWindowWidth()-310, 10, 300, 200, (SimpleSensor*)getSimulationManager()->getSensor("Encoder1"), dims, "Arm1"))
    {
        StopSimulation();
        
        NativeDialog* openDialog = new NativeDialog(DialogType_Save, "Save plot data...", "txt");
        openDialog->Show();
     
        char* pathToFile;
        if(openDialog->GetInput(&pathToFile) == DialogResult_OK)
        {
			SimpleSensor* sensor = (SimpleSensor*)getSimulationManager()->getSensor("Encoder1"); 
			sensor->SaveMeasurementsToTextFile(pathToFile);
        }
     
        delete [] pathToFile;
        delete openDialog;
        
        ResumeSimulation();
     }
	 
	 FeatherstoneEntity* fe = (FeatherstoneEntity*)getSimulationManager()->getEntity("FE");
	 btVector3 f, tau;
	 fe->getJointFeedback(2, f, tau);
	 std::cout << "Force=(" << f.x() << "," << f.y() << "," << f.z() << ") Torque=(" << tau.x() << "," << tau.y() << "," << tau.z() << ")" << std::endl;	 */
}