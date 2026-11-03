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
//  PluginManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 28/10/26.
//  Copyright (c) 2026 Patryk Cieslak. All rights reserved.
//

#include "core/PluginManager.h"

#include <dlfcn.h>

namespace sf
{

PluginManager::~PluginManager()
{
    for (auto h : pluginHandles_)
        dlclose(h.second);
}

void* PluginManager::LoadPlugin(std::string_view path)
{
    void* handle = GetPluginHandle(path);
    
    if (!handle) // Load plugin as it doesn't exist yet
    {
        handle = dlopen(std::string(path).c_str(), RTLD_LAZY);
        if (!handle)
            return nullptr;

        pluginHandles_.insert({std::string(path), handle});
    }
    return handle;
}

void* PluginManager::GetPluginHandle(std::string_view path)
{
    auto it = pluginHandles_.find(std::string(path));
    if (it != pluginHandles_.end())
        return it->second;
    else
        return nullptr;
}

}