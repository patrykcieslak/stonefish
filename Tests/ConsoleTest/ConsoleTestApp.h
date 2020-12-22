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
//  ConsoleTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2018-2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__ConsoleTestApp__
#define __Stonefish__ConsoleTestApp__

#include <core/ConsoleSimulationApp.h>
#include "ConsoleTestManager.h"

class ConsoleTestApp : public sf::ConsoleSimulationApp
{
public:
    ConsoleTestApp(std::string dataDirPath, ConsoleTestManager* sim);
};

#endif
