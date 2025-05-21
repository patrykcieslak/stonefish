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
//  main.cpp
//  LearningTest
//
//  Created by Patryk Cieslak on 20/05/2025.
//  Copyright (c) 2025 Patryk Cieslak. All rights reserved.
//

#include <core/GraphicalSimulationApp.h>
#include <sensors/scalar/RotaryEncoder.h>
#include <actuators/Motor.h>
#include <sensors/Sample.h>

#include "LearningTestManager.h"

struct LearningThreadData
{
    sf::SimulationApp& sim;
};

int learning(void* data)
{
    sf::SimulationApp& simApp = static_cast<LearningThreadData*>(data)->sim;
    sf::SimulationManager* simManager = simApp.getSimulationManager();

    // Wait for app to be ready
    while(simApp.getState() == sf::SimulationState::NOT_READY)
    {
        SDL_Delay(10);
    }

    // Start the simulation (includes building the scenario)
    simApp.StartSimulation();
    
    // Repeatedly step the simulation
    while(simApp.getState() != sf::SimulationState::FINISHED)
    {
        // Step simulation
        simApp.StepSimulation();

        // Get the observations
        sf::Scalar angle1 = static_cast<sf::RotaryEncoder*>(simManager->getSensor("Encoder1"))->getLastSample().getValue(0);
        sf::Scalar angle2 = static_cast<sf::RotaryEncoder*>(simManager->getSensor("Encoder2"))->getLastSample().getValue(0);

        // Compute command
        sf::Scalar command = btCos(angle1 * angle2) * 15;

        // Apply actuator commands
        static_cast<sf::Motor*>(simManager->getActuator("Motor"))->setIntensity(command);
    }
    
    return 0;
}

int main(int argc, const char * argv[])
{
    // Define the simulation settings
    sf::RenderSettings s;
    s.windowW = 1200;
    s.windowH = 900;
    s.aa = sf::RenderQuality::HIGH;
    s.shadows = sf::RenderQuality::HIGH;
    s.ao = sf::RenderQuality::HIGH;
    s.atmosphere = sf::RenderQuality::HIGH;
    s.ocean = sf::RenderQuality::DISABLED;
    
    sf::HelperSettings h;
    h.showFluidDynamics = false;
    h.showCoordSys = false;
    h.showBulletDebugInfo = false;
    h.showSensors = false;
    h.showActuators = false;
    h.showForces = false;
    
    // Create the simulation manager and application
    // Here you define the sampling frequency of the simulation;
    // higher frequency means more accurate simulation but taking more time to compute...
    LearningTestManager* simulationManager = new LearningTestManager(1000.0);
    sf::GraphicalSimulationApp app("LearningTest", std::string(DATA_DIR_PATH), s, h, simulationManager);
    
    // Start the learning thread
    LearningThreadData data {app};
    SDL_Thread* learningThread = SDL_CreateThread(learning, "learningThread", &data);

    // Start the main application loop (no automatic start and no automatic step!)
    // Here you define the time step that will be used to advance the simulation.
    app.Run(false, false, sf::Scalar(0.1));

    // Clean-up
    int status {0};
    SDL_WaitThread(learningThread, &status);
    return status;
}

