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
//  CableTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 13/11/2025.
//  Copyright (c) 2025 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__CableTestApp__
#define __Stonefish__CableTestApp__

#include <core/GraphicalSimulationApp.h>
#include "CableTestManager.h"

class CableTestApp : public sf::GraphicalSimulationApp
{
public:
    CableTestApp(std::string dataDirPath, sf::RenderSettings s, sf::HelperSettings h, CableTestManager* sim);
    
    void DoHUD();
};

#endif
