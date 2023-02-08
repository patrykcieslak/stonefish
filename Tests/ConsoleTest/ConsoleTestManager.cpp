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
//  ConsoleTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 18/09/2018.
//  Copyright(c) 2018-2021 Patryk Cieslak. All rights reserved.
//

#include "ConsoleTestManager.h"

#include <entities/statics/Plane.h>
#include <entities/solids/Sphere.h>
#include <core/Robot.h>
#include <utils/UnitSystem.h>
#include <utils/SystemUtil.hpp>
#include <core/ScenarioParser.h>
#include <core/Console.h>

ConsoleTestManager::ConsoleTestManager(sf::Scalar stepsPerSecond) 
    : SimulationManager(stepsPerSecond, sf::SolverType::SOLVER_SI, sf::CollisionFilteringType::COLLISION_EXCLUSIVE)
{
}

void ConsoleTestManager::BuildScenario()
{
#ifdef PARSED_SCENARIO
    sf::ScenarioParser parser(this);
    bool success = parser.Parse(sf::GetDataPath() + "console_test.scn");
    if(success)
        cInfo("Scenario description parsed successfully.");
    else
    {
        cError("Errors detected when parsing scenario description!");
        auto log = parser.getLog();
        for(size_t i=0; i<log.size(); ++i)
        {
            switch(log[i].type)
            {
                case sf::MessageType::INFO:
                    cInfo(log[i].text.c_str());
                    break;
                
                case sf::MessageType::ERROR:
                    cError(log[i].text.c_str());
                    break;

                case sf::MessageType::WARNING:
                    cWarning(log[i].text.c_str());
                    break;

                case sf::MessageType::CRITICAL:
                    cCritical(log[i].text.c_str());
                    break;
            }
        }
    }
#else
    //Create materials
    CreateMaterial("Rock", sf::UnitSystem::Density(sf::CGS, sf::MKS, 3.0), 0.8);
    SetMaterialsInteraction("Rock", "Rock", 0.9, 0.7);
    
    //Build scene	
    sf::Plane* plane = new sf::Plane("Bottom", 1000.0, "Rock");
    AddStaticEntity(plane, sf::Transform(sf::IQ(), sf::Vector3(0,0,0)));
    
    sf::BodyPhysicsSettings phy;
    phy.mode = sf::BodyPhysicsMode::SURFACE;
    phy.collisions = true;
    sf::Sphere* sph1 = new sf::Sphere("Sphere1", phy, 0.1, sf::I4(), "Rock", "");
    sf::Sphere* sph2 = new sf::Sphere("Sphere2", phy, 0.1, sf::I4(), "Rock", "");
    
    std::vector<sf::SolidEntity*> links(0);
    links.push_back(sph2);
    
    sf::Robot* robot = new sf::Robot("Robot", false);
    robot->DefineLinks(sph1, links);
    robot->DefineRevoluteJoint("Joint1", 
                               "Sphere1", 
                               "Sphere2", 
                               sf::Transform(sf::IQ(), sf::Vector3(0.5,0,0.0)), 
                               sf::Vector3(0.0,1.0,0.0), 
                               std::make_pair<sf::Scalar,sf::Scalar>(-1.0,1.0));
	robot->BuildKinematicTree();
    AddRobot(robot, sf::I4());
#endif
}

void ConsoleTestManager::SimulationStepCompleted(sf::Scalar timeStep)
{
    cInfo("Simulation time: %1.3lf", getSimulationTime());
}
