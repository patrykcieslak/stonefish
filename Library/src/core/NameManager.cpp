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
//  Copyright (c) 2014-2026 Patryk Cieslak. All rights reserved.
//

#include "core/NameManager.h"

#include <algorithm>

namespace sf
{

NameManager::NameManager()
{
    names_ = std::vector<std::string>(0);
}

NameManager::~NameManager()
{
    names_.clear();
}

std::string NameManager::AddName(const std::string& proposedName)
{
    std::string goodName = proposedName;
    int number = 1;
    
checkname:
    auto it = std::find(names_.begin(), names_.end(), goodName);
    if (it != names_.end())
    {
        goodName = proposedName + std::to_string(number);
        ++number;
        goto checkname;
    }
    
    names_.push_back(goodName);
    return goodName;
}

void NameManager::RemoveName(const std::string& name)
{
    auto it = std::find(names_.begin(), names_.end(), name);
    if (it != names_.end())
        names_.erase(it);
}

void NameManager::ClearNames()
{
    names_.clear();
}

}
