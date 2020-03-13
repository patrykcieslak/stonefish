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
//  UnderwaterTestManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright(c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__UnderwaterTestManager__
#define __Stonefish__UnderwaterTestManager__

#include <core/SimulationManager.h>

//#define PARSED_SCENARIO

class UnderwaterTestManager : public sf::SimulationManager
{
public:
    UnderwaterTestManager(sf::Scalar stepsPerSecond);
    
    void BuildScenario();
};

#endif
