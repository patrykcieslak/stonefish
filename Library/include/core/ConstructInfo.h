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
//  ConstructInfo.h
//  Stonefish
//
//  Created by Patryk Cieslak on 20/07/26.
//  Copyright (c) 2026 Patryk Cieslak. All rights reserved.
//

#pragma once

#include "StonefishCommon.h"
#include <unordered_map>
#include <variant>

namespace sf
{

enum class ConstructInfoValueType {BOOL, INT, SCALAR, VECTOR3, TRANSFORM, STRING};

struct ConstructInfoValue
{
    ConstructInfoValueType valueType;
    std::variant<bool, int, Scalar, Vector3, Transform, std::string> value;  
    bool optional;
    bool valid;

    ConstructInfoValue() : 
        valueType(ConstructInfoValueType::SCALAR),
        value(Scalar(0.)),
        optional(true),
        valid(false)
    {}

    ConstructInfoValue(ConstructInfoValueType type, bool isOptional) :
        valueType(type),
        value(Scalar(0.)),
        optional(isOptional),
        valid(false)
    {}
};

struct ConstructInfoNode
{
    std::unordered_map<std::string, ConstructInfoNode> childNodes;
    std::unordered_map<std::string, ConstructInfoValue> attributes;
    bool optional;
};

struct ConstructInfo
{
    std::unordered_map<std::string, ConstructInfoNode> nodes;
};

}