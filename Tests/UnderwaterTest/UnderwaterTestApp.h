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
//  UnderwaterTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__UnderwaterTestApp__
#define __Stonefish__UnderwaterTestApp__

#include <core/GraphicalSimulationApp.h>
#include <graphics/OpenGLPrinter.h>
#include "UnderwaterTestManager.h"

class UnderwaterTestApp : public sf::GraphicalSimulationApp
{
public:
    UnderwaterTestApp(std::string dataDirPath, sf::RenderSettings s, sf::HelperSettings h, UnderwaterTestManager* sim);
    
    void DoHUD();
    void InitializeGUI();
    
private:
    sf::OpenGLPrinter* largePrint;
};

#endif
