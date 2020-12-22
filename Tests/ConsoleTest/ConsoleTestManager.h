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
//  ConsoleTestManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 18/09/2018.
//  Copyright(c) 2018-2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__ConsoleTestManager__
#define __Stonefish__ConsoleTestManager__

#include <core/SimulationManager.h>

#define PARSED_SCENARIO

class ConsoleTestManager : public sf::SimulationManager
{
public:
    ConsoleTestManager(sf::Scalar stepsPerSecond);
    
    void BuildScenario();
    void SimulationStepCompleted(sf::Scalar timeStep);
};

#endif
