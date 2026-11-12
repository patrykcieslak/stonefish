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
//  PluginManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 28/10/26.
//  Copyright (c) 2026 Patryk Cieslak. All rights reserved.
//

#pragma once

#include "StonefishCommon.h"
#include <unordered_map>

namespace sf
{
    class PluginManager
    {
    public:
        //! A constructor
        PluginManager() = default;

        //! A destructor.
        virtual ~PluginManager();

        //! A method used to load a plugin library (checks if library already loaded).
        /*!
         \param path file path to the plugin library
         \return plugin handle
        */
        void* LoadPlugin(std::string_view path);

        //! A method returning the plugin handle.
        /*!
         \param path file path to the plugin library
         \return plugin handle
        */
        void* GetPluginHandle(std::string_view path);

    private:
        std::unordered_map<std::string, void*> pluginHandles_;
    };
}
