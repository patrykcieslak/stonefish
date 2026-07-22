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
//  ParserTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 20/07/2026.
//  Copyright (c) 2026 Patryk Cieslak. All rights reserved.
//

#include "ParserTestManager.h"

#include <core/ScenarioParser.h>
#include <utils/SystemUtil.hpp>

ParserTestManager::ParserTestManager(sf::Scalar stepsPerSecond) 
    : SimulationManager(stepsPerSecond, sf::Solver::SI, sf::CollisionFilter::EXCLUSIVE)
{
}

void ParserTestManager::BuildScenario()
{
    sf::ScenarioParser parser(this);
    bool success = parser.Parse(sf::GetDataPath() + "parser_test.scn");
    parser.SaveLog("parser_test.log");
    if(!success)
        cCritical("Scenario parser: Parsing failed!");    
}
