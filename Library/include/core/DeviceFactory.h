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
//  DeviceFactory.h
//  Stonefish
//
//  Created by Patryk Cieslak on 20/07/26.
//  Copyright (c) 2026 Patryk Cieslak. All rights reserved.
//

#pragma once

#include "sensors/Sensor.h"
#include "actuators/Actuator.h"
#include "comms/Comm.h"
#include <functional>
#include <unordered_map>

// Macros
#define REGISTER_SENSOR(TypeName, ClassName) \
    struct ClassName##Register \
    { \
        ClassName##Register() \
        { \
            SensorFactory::Instance().Register(TypeName, { &ClassName::getConstructInfo, &ClassName::Construct} ); \
        } \
    } ClassName##RegisterInstance;

#define REGISTER_ACTUATOR(TypeName, ClassName) \
    struct ClassName##Register \
    { \
        ClassName##Register() \
        { \
            ActuatorFactory::Instance().Register(TypeName, { &ClassName::getConstructInfo, &ClassName::Construct} ); \
        } \
    } ClassName##RegisterInstance;

#define REGISTER_COMM(TypeName, ClassName) \
    struct ClassName##Register \
    { \
        ClassName##Register() \
        { \
            CommFactory::Instance().Register(TypeName, { &ClassName::getConstructInfo, &ClassName::Construct} ); \
        } \
    } ClassName##RegisterInstance;

// Factories
namespace sf
{

struct SensorFactoryEntry
{
    std::function<ConstructInfo()> getConstructInfo;
    std::function<std::unique_ptr<Sensor>(const std::string& uniqueName, Scalar frequency, ConstructInfo& info)> construct; 
};

class SensorFactory
{
public:
    static SensorFactory& Instance() { static SensorFactory f; return f; }

    void Register(const std::string& typeName, SensorFactoryEntry e) { registry_[typeName] = std::move(e); }

    const SensorFactoryEntry* Find(const std::string& typeName) const
    {
        auto it = registry_.find(typeName);
        return it != registry_.end() ? &it->second : nullptr;
    }

private:
    std::unordered_map<std::string, SensorFactoryEntry> registry_;
};

struct ActuatorFactoryEntry
{
    std::function<ConstructInfo()> getConstructInfo;
    std::function<std::unique_ptr<Actuator>(const std::string& uniqueName, ConstructInfo& info)> construct; 
};

class ActuatorFactory
{
public:
    static ActuatorFactory& Instance() { static ActuatorFactory f; return f; }

    void Register(const std::string& typeName, ActuatorFactoryEntry e) { registry_[typeName] = std::move(e); }

    const ActuatorFactoryEntry* Find(const std::string& typeName) const
    {
        auto it = registry_.find(typeName);
        return it != registry_.end() ? &it->second : nullptr;
    }

private:
    std::unordered_map<std::string, ActuatorFactoryEntry> registry_;
};

struct CommFactoryEntry
{
    std::function<ConstructInfo()> getConstructInfo;
    std::function<std::unique_ptr<Comm>(const std::string& uniqueName, uint64_t deviceId, ConstructInfo& info)> construct; 
};

class CommFactory
{
public:
    static CommFactory& Instance() { static CommFactory f; return f; }

    void Register(const std::string& typeName, CommFactoryEntry e) { registry_[typeName] = std::move(e); }

    const CommFactoryEntry* Find(const std::string& typeName) const
    {
        auto it = registry_.find(typeName);
        return it != registry_.end() ? &it->second : nullptr;
    }

private:
    std::unordered_map<std::string, CommFactoryEntry> registry_;
};

}
