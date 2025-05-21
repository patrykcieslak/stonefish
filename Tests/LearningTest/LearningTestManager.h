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
//  LearningTestManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 20/05/2025.
//  Copyright (c) 2025 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__LearningTestManager__
#define __Stonefish__LearningTestManager__

#include <core/SimulationManager.h>

#define USE_FEATHERSTONE_ROBOT

class LearningTestManager : public sf::SimulationManager 
{
public:
    LearningTestManager(sf::Scalar stepsPerSecond);
    
    void BuildScenario() override;
};

#endif