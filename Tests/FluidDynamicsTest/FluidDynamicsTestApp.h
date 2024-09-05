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
//  FluidDynamicsTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 01/07/2024.
//  Copyright (c) 2024 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__FluidDynamicsTestApp__
#define __Stonefish__FluidDynamicsTestApp__

#include <core/GraphicalSimulationApp.h>
#include "FluidDynamicsTestManager.h"

class FluidDynamicsTestApp : public sf::GraphicalSimulationApp
{
public:
    FluidDynamicsTestApp(std::string dataDirPath, sf::RenderSettings s, sf::HelperSettings h, FluidDynamicsTestManager* sim);
    void DoHUD();
};

#endif
