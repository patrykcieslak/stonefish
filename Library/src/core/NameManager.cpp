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
//  NameManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "core/NameManager.h"

namespace sf
{

NameManager::NameManager()
{
    names = std::vector<std::string>(0);
}

NameManager::~NameManager()
{
    names.clear();
}

std::string NameManager::AddName(std::string proposedName)
{
    std::string goodName = proposedName;
    int number = 1;
    
checkname:
    for(unsigned int i = 0; i < names.size(); i++)
    {
        if(goodName == names[i])
        {
            goodName = proposedName + std::to_string(number);
            number++;
            goto checkname;
        }
    }
    
    names.push_back(goodName);
    return goodName;
}

void NameManager::RemoveName(std::string name)
{
    std::vector<std::string>::iterator it;
    for(it = names.begin(); it < names.end(); it++)
        if(*it == name)
        {
            names.erase(it);
            break;
        }
}

void NameManager::ClearNames()
{
    names.clear();
}

}
