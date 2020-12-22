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
//  ConsoleTest
//
//  Created by Patryk Cieslak on 12/09/2018.
//  Copyright(c) 2018-2019 Patryk Cieslak. All rights reserved.
//

#include "ConsoleTestApp.h"
#include "ConsoleTestManager.h"

int main(int argc, const char * argv[])
{
    ConsoleTestManager* simulationManager = new ConsoleTestManager(500.0);
    ConsoleTestApp app(std::string(DATA_DIR_PATH), simulationManager);
    app.Run(true);
    
    return 0;
}