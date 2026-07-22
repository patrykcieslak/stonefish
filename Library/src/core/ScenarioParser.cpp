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
//  ScenarioParser.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 17/07/19.
//  Copyright (c) 2019-2026 Patryk Cieslak. All rights reserved.
//

#include "core/ScenarioParser.h"
#include "core/DeviceFactory.h"
#include "core/SimulationManager.h"
#include "core/NED.h"
#include "core/FeatherstoneRobot.h"
#include "core/GeneralRobot.h"
#include "entities/statics/Obstacle.h"
#include "entities/statics/Plane.h"
#include "entities/statics/Terrain.h"
#include "entities/AnimatedEntity.h"
#include "entities/animation/ManualTrajectory.h"
#include "entities/animation/PWLTrajectory.h"
#include "entities/animation/CRTrajectory.h"
#include "entities/animation/BSTrajectory.h"
#include "entities/solids/Box.h"
#include "entities/solids/Cylinder.h"
#include "entities/solids/Sphere.h"
#include "entities/solids/Torus.h"
#include "entities/solids/Wing.h"
#include "entities/solids/Polyhedron.h"
#include "entities/solids/Compound.h"
#include "entities/forcefields/Uniform.h"
#include "entities/forcefields/Jet.h"
#include "entities/FeatherstoneEntity.h"
#include "entities/CableEntity.h"
#include "sensors/scalar/LinkSensor.h"
#include "sensors/scalar/JointSensor.h"
#include "sensors/VisionSensor.h"
#include "sensors/Contact.h"
#include "actuators/LinkActuator.h"
#include "actuators/JointActuator.h"
#include "actuators/Light.h"
#include "comms/AcousticModem.h"
#include "comms/SimpleUSBL.h"
#include "comms/RealUSBL.h"
#include "comms/OpticalModem.h"
#include "joints/FixedJoint.h"
#include "graphics/OpenGLDataStructs.h"
#include "utils/SystemUtil.hpp"
#include "tinyexpr.h"
#include <sstream>

namespace sf
{

ScenarioParser::ScenarioParser(SimulationManager* sm) : log(false), sm_(sm)
{
    graphical_ = SimulationApp::getApp()->hasGraphics();
}

std::vector<ConsoleMessage> ScenarioParser::getLog()
{
    return log.getLines();
}

SimulationManager* ScenarioParser::getSimulationManager()
{
    return sm_;
}

bool ScenarioParser::Parse(std::string filename)
{
    cInfo("Scenario parser: Loading scenario from '%s'.", filename.c_str());
    log.Print(MessageType::INFO, "Scenario file: %s", filename.c_str());
    
    //Open file
    XMLError result = doc_.LoadFile(filename.c_str());
    if(result != XML_SUCCESS)
    {
        switch(result)
        {
            case XMLError::XML_ERROR_FILE_NOT_FOUND:
            {
                cInfo("Scenario parser: File not found!");
                log.Print(MessageType::ERROR, "File not found!");
            }
                break;

            default:
            {
                cInfo("Scenario parser: Syntax error in file!");
                log.Print(MessageType::ERROR, "Syntax error in file!");
            }
                break;
        }
        return false;
    }
    
    //Find root node
    XMLNode* root = doc_.FirstChildElement("scenario");
    if(root == nullptr)
    {
        log.Print(MessageType::ERROR, "Root node not found!");
        return false;
    }
    
    if(!PreProcess(root))
    {
        log.Print(MessageType::ERROR, "Pre-processing failed!");
        return false;
    }

    //Include other scenario files
    if(!IncludeFiles(root))
    {
        log.Print(MessageType::ERROR, "Including files failed!");
        return false;
    }

    //Load solver settings
    XMLElement* element = root->FirstChildElement("solver");
    if(element != nullptr)
        ParseSolver(element);
    
    //Load environment settings
    element = root->FirstChildElement("environment");
    if(element == nullptr)
    {
        log.Print(MessageType::ERROR, "Environment settings not defined!");
        return false;
    }
    if(!ParseEnvironment(element))
    {
        log.Print(MessageType::ERROR, "Environment settings not properly defined!");
        return false;
    }
        
    //Load materials
    element = root->FirstChildElement("materials");
    if(element == nullptr)
    {
        log.Print(MessageType::ERROR, "Materials not defined!");
        return false;
    }
    if(!ParseMaterials(element))
    {
        log.Print(MessageType::ERROR, "Materials not properly defined!");
        return false;
    }
    
    //Load looks (optional)
    if(isGraphicalSim())
    {
        element = root->FirstChildElement("looks");
        if(element == nullptr)
            log.Print(MessageType::WARNING, "Looks not defined -> using standard look.");

        while(element != nullptr)
        {
            if(!ParseLooks(element))
            {
                log.Print(MessageType::ERROR, "Looks not properly defined!");
                return false;
            }
            element = element->NextSiblingElement("looks");
        }
    }
        
    //Load static objects (optional)
    element = root->FirstChildElement("static");
    while(element != nullptr)
    {
        if(!ParseStatic(element))
        {
            log.Print(MessageType::ERROR, "Static object not properly defined!");
            return false;
        }
        element = element->NextSiblingElement("static");
    }

    //Load animated bodys (optional)
    element = root->FirstChildElement("animated");
    while(element != nullptr)
    {
        if(!ParseAnimated(element))
        {
            log.Print(MessageType::ERROR, "Animated object not properly defined!");
            return false;
        }
        element = element->NextSiblingElement("animated");
    }
    
    //Load dynamic objects (optional)
    element = root->FirstChildElement("dynamic");
    while(element != nullptr)
    {
        if(!ParseDynamic(element))
        {
            log.Print(MessageType::ERROR, "Dynamic object not properly defined!");
            return false;
        }
        element = element->NextSiblingElement("dynamic");
    }

    //Load robots (optional)
    element = root->FirstChildElement("robot");
    while(element != nullptr)
    {
        if(!ParseRobot(element))
        {
            log.Print(MessageType::ERROR, "Robot not properly defined!");
            return false;
        }
        element = element->NextSiblingElement("robot");
    }

    //Load cables (optional)
    element = root->FirstChildElement("cable");
    while(element != nullptr)
    {
        if(!ParseCable(element))
        {
            log.Print(MessageType::ERROR, "Cable not properly defined!");
            return false;
        }
        element = element->NextSiblingElement("cable");
    }

    //Load glue (optional)
    element = root->FirstChildElement("glue");
    while(element != nullptr)
    {
        if(!ParseGlue(element))
        {
            log.Print(MessageType::ERROR, "Glue not properly defined!");
            return false;
        }
        element = element->NextSiblingElement("glue");
    }
    
    //Load standalone vision sensors (optional)
    element = root->FirstChildElement("sensor");
    while(element != nullptr)
    {
        if(!ParseSensor(element))
        {
            log.Print(MessageType::ERROR, "Sensor not properly defined!");
            return false;
        }
        element = element->NextSiblingElement("sensor");
    }

    //Load standalone lights (optional)
    element = root->FirstChildElement("light");
    while(element != nullptr)
    {
        std::unique_ptr<Light> l = ParseLight(element, "");
        if(l == nullptr)
        {
            log.Print(MessageType::ERROR, "Light not properly defined!");
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item;
            if((item = element->FirstChildElement("world_transform")) == nullptr || !ParseTransform(item, origin))
            {
                log.Print(MessageType::ERROR, "Light not properly defined!");
                return false;
            }
            l->AttachToWorld(origin);
            sm_->AddActuator(std::move(l));
        }
        element = element->NextSiblingElement("light");
    }

    //Load standalone communication devices (beacons, optional)
    element = root->FirstChildElement("comm");
    while(element != nullptr)
    {
        std::unique_ptr<Comm> comm = ParseComm(element, "");
        if(comm == nullptr)
        {
            log.Print(MessageType::ERROR, "Communication device not properly defined!");
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item;
            if((item = element->FirstChildElement("world_transform")) == nullptr || !ParseTransform(item, origin))
            {
                log.Print(MessageType::ERROR, "Communication device not properly defined!");
                return false;
            }
            comm->AttachToWorld(origin);
            sm_->AddComm(std::move(comm));
        }
        element = element->NextSiblingElement("comm");
    }
        
    //Load contacts (optional)
    element = root->FirstChildElement("contact");
    while(element != nullptr)
    {
        if(!ParseContact(element))
        {
            log.Print(MessageType::ERROR, "Contact not properly defined!");
            return false;
        }
        element = element->NextSiblingElement("contact");
    }
    
    log.Print(MessageType::INFO, "Parsing finished normally.");
    return true;
}

bool ScenarioParser::SaveLog(std::string filename)
{
    if(log.SaveToFile(filename))
    {
        cInfo("Scenario parser: Log saved to file '%s'.", filename.c_str());
        return true;
    }    
    else
    {
        cError("Scenario parser: Not possible to save log to file '%s'!", filename.c_str());
        return false;
    }
}

bool ScenarioParser::PreProcess(XMLNode* root, const std::map<std::string, std::string>& args)
{
    //Replace arguments passed to the files
    if(args.size() > 0 && !ReplaceArguments(root, args))
        return false;

    //Parse and evaluate mathematical expressions
    if(!EvaluateMath(root))
        return false;

    return true;
}

bool ScenarioParser::ReplaceArguments(XMLNode* node, const std::map<std::string, std::string>& args)
{
    XMLElement* element = node->ToElement();
    if(element != nullptr)
    {
        for(const XMLAttribute* attr = element->FirstAttribute(); attr != nullptr; attr = attr->Next())
        {
            std::string value = std::string(attr->Value());

            //Replace $(arg ....) with the actual value
            std::string replacedValue = "";
            size_t currentPos = 0;
            size_t startPos, endPos;

            //Fing "arg" keywords
            while((startPos = value.find("$(arg ", currentPos)) != std::string::npos && (endPos = value.find(")", startPos+2)) != std::string::npos)
            {
                //Append prefix
                replacedValue += value.substr(currentPos, startPos - currentPos);
                
                //Check if argument defined and replace value
                std::string argName = value.substr(startPos+6, endPos-startPos-6);              
                try
                {
                    replacedValue += args.at(argName);
                }
                catch(const std::out_of_range& e)
                {
                    log.Print(MessageType::ERROR, "Argument '%s' does not exist!", argName.c_str());
                    return false;
                }

                currentPos = endPos + 1;
            }
            //Append postfix
            replacedValue += value.substr(currentPos, value.size() - currentPos);

            if(replacedValue != value)
                element->SetAttribute(attr->Name(), replacedValue.c_str());
        }
    }

    for(XMLNode* child = node->FirstChild(); child != nullptr; child = child->NextSibling())
    {
        if(!ReplaceArguments(child, args))
            return false;
    }

    return true;
}

bool ScenarioParser::EvaluateMath(XMLNode* node)
{
    XMLElement* element = node->ToElement();
    if(element != nullptr)
    {
        for(const XMLAttribute* attr = element->FirstAttribute(); attr != nullptr; attr = attr->Next())
        {
            std::string value = std::string(attr->Value());

            //Evaluate expressions between ${ and }
            std::string replacedValue = "";
            size_t currentPos = 0;
            size_t startPos, endPos;

            //Fing expression start and end
            while((startPos = value.find("${", currentPos)) != std::string::npos && (endPos = value.find("}", startPos+2)) != std::string::npos)
            {
                //Append prefix
                replacedValue += value.substr(currentPos, startPos - currentPos);
                
                //Parse and evaluate expression 
                std::string expr = value.substr(startPos+2, endPos-startPos-2);              
                int err;
                double result = te_interp(expr.c_str(), &err);
                if(err != 0)
                {
                    log.Print(MessageType::ERROR, "Evaluating expression '%s' failed. Parse error at position %d.", expr.c_str(), err);
                    return false;
                }

                //Replace text with result
                replacedValue += std::to_string(result);

                currentPos = endPos + 1;
            }
            //Append postfix
            replacedValue += value.substr(currentPos, value.size() - currentPos);

            if(replacedValue != value)
                element->SetAttribute(attr->Name(), replacedValue.c_str());
        }
    }

    for(XMLNode* child = node->FirstChild(); child != nullptr; child = child->NextSibling())
    {
        if(!EvaluateMath(child))
            return false;
    }

    return true;
}

bool ScenarioParser::IncludeFiles(XMLNode* node)
{
    XMLElement* element = node->FirstChildElement("include");
    while(element != nullptr)
    {
        //Get file path
        const char* path = nullptr;
        if(element->QueryStringAttribute("file", &path) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Include not properly defined!");
            return false;
        }
        
        //Read optional arguments
        std::map<std::string, std::string> args;	
		XMLElement* argElement = element->FirstChildElement("arg");
		while(argElement != nullptr)
        {
			const char* name = argElement->Attribute("name");
			const char* value = argElement->Attribute("value");
			
			if(value == nullptr || name == nullptr)
			{
			    log.Print(MessageType::ERROR, "Include file argument not properly defined!");
				return false;
			}

			args.insert(std::make_pair(std::string(name), std::string(value)));
			argElement = argElement->NextSiblingElement("arg");
		}

        //Load file
        std::string includedPath = GetFullPath(std::string(path));
        XMLDocument includedDoc;
        XMLError result = includedDoc.LoadFile(includedPath.c_str());
        if(result != XML_SUCCESS)
        {
            switch(result)
            {
                case XMLError::XML_ERROR_FILE_NOT_FOUND:
                {
                    cInfo("Scenario parser: Included file not found!");
                    log.Print(MessageType::ERROR, "Included file '%s' not found!", includedPath.c_str());
                }
                    break;

                default:
                {
                    cInfo("Scenario parser: Syntax error in included file!");
                    log.Print(MessageType::ERROR, "Syntax error in included file '%s'!", includedPath.c_str());
                }
                    break;
            }
            return false;
        }
        cInfo("Scenario parser: Including file '%s'", includedPath.c_str());
        log.Print(MessageType::INFO, "Including file '%s'", includedPath.c_str());
        
        node->DeleteChild(element); //Delete "include" element
        
        XMLNode* includedRoot = includedDoc.FirstChildElement("scenario");
        if(includedRoot == nullptr)
        {
            log.Print(MessageType::ERROR, "Root node not found in included file '%s'!", includedPath.c_str());
            return false;
        }
        
        if(!PreProcess(includedRoot, args))
        {
            log.Print(MessageType::ERROR, "Pre-processing of included file '%s' failed!", includedPath.c_str());
            return false;
        }
        
        for(const XMLNode* child = includedRoot->FirstChild(); child != nullptr; child = child->NextSibling())
        {
            if(!CopyNode(node, child))
            {
                log.Print(MessageType::ERROR, "Could not copy included XML elements!");
                return false;
            }
        }
        element = node->FirstChildElement("include");
    }
    return true;
}

bool ScenarioParser::ParseSolver(XMLElement* element)
{
    XMLElement* item;
    Scalar erp, stopErp;
    sm_->getJointErp(erp, stopErp);
    Scalar erp2 = sm_->getDynamicsWorld()->getSolverInfo().m_erp2;
    Scalar globalDamping = sm_->getDynamicsWorld()->getSolverInfo().m_damping;
    Scalar globalFriction = sm_->getDynamicsWorld()->getSolverInfo().m_friction;
    Scalar linSleep, angSleep;
    sm_->getSleepingThresholds(linSleep, angSleep);

    if((item = element->FirstChildElement("erp")) != nullptr)
        item->QueryAttribute("value", &erp);
    if((item = element->FirstChildElement("stop_erp")) != nullptr)
        item->QueryAttribute("value", &stopErp);
    if((item = element->FirstChildElement("erp2")) != nullptr)
        item->QueryAttribute("value", &erp2);
    if((item = element->FirstChildElement("global_damping")) != nullptr)
        item->QueryAttribute("value", &globalDamping);
    if((item = element->FirstChildElement("global_friction")) != nullptr)
        item->QueryAttribute("value", &globalFriction);
    if((item = element->FirstChildElement("sleeping_thresholds")) != nullptr)
    {
        item->QueryAttribute("linear", &linSleep);
        item->QueryAttribute("angular", &angSleep);
    }
    sm_->setSolverParams(erp, stopErp, erp2, globalDamping, globalFriction, linSleep, angSleep);
    
    unsigned int presc;
    if ((item = element->FirstChildElement("fluid_dynamics")) != nullptr
        && item->QueryAttribute("prescaler", &presc) == XML_SUCCESS)
            sm_->setFluidDynamicsPrescaler(presc);

    unsigned int maxPhysicsThreads;
    if ((item = element->FirstChildElement("multithreading")) != nullptr
        && item->QueryAttribute("max_physics_threads", &maxPhysicsThreads) == XML_SUCCESS)
            SimulationApp::getApp()->setMaxPhysicsThreads(maxPhysicsThreads);

    return true;
}

bool ScenarioParser::ParseEnvironment(XMLElement* element)
{
    XMLElement* item;

    //Setup NED home
    XMLElement* ned = element->FirstChildElement("ned");
    if(ned == nullptr) //Obligatory
    {
        log.Print(MessageType::ERROR, "NED definition missing!");
        return false;
    }
    
    Scalar lat, lon;
    if(ned->QueryAttribute("latitude", &lat) != XML_SUCCESS
       || ned->QueryAttribute("longitude", &lon) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "NED definition incorrect!");
        return false;
    }
    sm_->getNED()->Init(lat, lon, Scalar(0));
    
    //Setup ocean
    XMLElement* ocean = element->FirstChildElement("ocean");
    if(ocean != nullptr)
    {
        log.Print(MessageType::INFO, "Ocean simulation enabled.");
        //Basic setup
        Scalar wavesHeight(0);
        Scalar waterDensity(1000);
        Scalar waterTemperature(15.0);
        Scalar jerlov(0.2);

        if((item = ocean->FirstChildElement("waves")) != nullptr
            && item->QueryAttribute("height", &wavesHeight) == XML_SUCCESS
            && wavesHeight > Scalar(0))
            log.Print(MessageType::INFO, "Using ocean surface with geometrical waves.");
        else
            log.Print(MessageType::INFO, "Using flat ocean surface.");
        
        if((item = ocean->FirstChildElement("water")) != nullptr)
        {
            item->QueryAttribute("density", &waterDensity);
            item->QueryAttribute("jerlov", &jerlov);
            item->QueryAttribute("temperature", &waterTemperature);
        }
        
        std::string waterName = sm_->getMaterialManager()->CreateFluid("Water", waterDensity, 1.308e-3, 1.55); 
        sm_->EnableOcean(wavesHeight, sm_->getMaterialManager()->getFluid(waterName));
        sm_->getOcean()->setWaterType(jerlov);
        sm_->getOcean()->SetConditions(waterTemperature);
        
        //Particles
        bool particles = true;
        if((item = ocean->FirstChildElement("particles")) != nullptr)
        {
            item->QueryAttribute("enabled", &particles);
        }
        sm_->getOcean()->setParticles(particles);

        //Currents
        if((item = ocean->FirstChildElement("current")) != nullptr)
        {
            Ocean* ocn = sm_->getOcean();
            do
            {
                std::unique_ptr<VelocityField> current = ParseVelocityField(item);
                if(current != nullptr)
                    ocn->AddVelocityField(std::move(current));
            }
            while((item = item->NextSiblingElement("current")) != nullptr);
        }
    }

    //Setup atmosphere
    XMLElement* atmosphere = element->FirstChildElement("atmosphere");
    if(atmosphere != nullptr)
    {
        //Basic setup
        if((item = atmosphere->FirstChildElement("sun")) != nullptr)
        {
            Scalar az, elev;
            if(item->QueryAttribute("azimuth", &az) != XML_SUCCESS
               || item->QueryAttribute("elevation", &elev) != XML_SUCCESS)
            {
                log.Print(MessageType::WARNING, "Sun position definition incorrect - using defualts.");
            }
            else
                sm_->getAtmosphere()->SetSunPosition(az, elev);
        }

        //Winds
        if((item = atmosphere->FirstChildElement("wind")) != nullptr)
        {
            Atmosphere* atm = sm_->getAtmosphere();
            do
            {
                std::unique_ptr<VelocityField> wind = ParseVelocityField(item);
                if(wind != nullptr)
                    atm->AddVelocityField(std::move(wind));
            }
            while((item = item->NextSiblingElement("wind")) != nullptr);
        }

        //Conditions
        if((item = atmosphere->FirstChildElement("conditions")) != nullptr)
        {
            Scalar temp = 20.0;
            Scalar press = 101300.0;
            Scalar hum = 0.5;
            item->QueryAttribute("temperature", &temp);
            item->QueryAttribute("pressure", &press);
            item->QueryAttribute("humidity", &hum);
            sm_->getAtmosphere()->SetConditions(temp, press, hum);
        }
    }
    return true;
}

bool ScenarioParser::ParseMaterials(XMLElement* element)
{
    //Get first material
    XMLElement* mat = element->FirstChildElement("material");
    
    if(mat == nullptr)
    {
        log.Print(MessageType::ERROR, "No material definitions found!");
        return false; //There has to be at least one material defined!
    }
        
    //Iterate through all materials
    while(mat != nullptr)
    {
        const char* name = nullptr;
        Scalar density, restitution;
        Scalar magnetic(0);
        if(mat->QueryStringAttribute("name", &name) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Material name missing!");
            return false;
        }
        std::string materialName(name);
        if(mat->QueryAttribute("density", &density) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Density of material '%s' missing!", materialName.c_str());
            return false;
        }
        if(mat->QueryAttribute("restitution", &restitution) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Restitution of material '%s' missing!", materialName.c_str());
            return false;
        }
        mat->QueryAttribute("magnetic", &magnetic);
        sm_->getMaterialManager()->CreateMaterial(materialName, density, restitution, magnetic);
        mat = mat->NextSiblingElement("material");
    }
    
    //Read friction table
    XMLElement* table = element->FirstChildElement("friction_table");
    if(table == nullptr) //Optional
    {
        log.Print(MessageType::WARNING, "Material friction table not defined - using defaults.");
    }
    else
    {
        XMLElement* friction = table->FirstChildElement("friction");
        
        while(friction != nullptr)
        {
            const char* name1 = nullptr;
            const char* name2 = nullptr;
            Scalar fstatic, fdynamic;
            if(friction->QueryStringAttribute("material1", &name1) != XML_SUCCESS
               || friction->QueryStringAttribute("material2", &name2) != XML_SUCCESS
               || friction->QueryAttribute("static", &fstatic) != XML_SUCCESS
               || friction->QueryAttribute("dynamic", &fdynamic) != XML_SUCCESS)
            {
                log.Print(MessageType::WARNING, "Material friction coefficients not properly defined - using defaults.");
            }
            else
            {
                if(!sm_->getMaterialManager()->SetMaterialsInteraction(std::string(name1), std::string(name2), fstatic, fdynamic))
                    log.Print(MessageType::WARNING, "Setting friction coefficients failed - using defaults.");
            }
            friction = friction->NextSiblingElement("friction");
        }
    }
    return true;
}
        
bool ScenarioParser::ParseLooks(XMLElement* element)
{
    //Get first look
    XMLElement* look = element->FirstChildElement("look");
    
    if(look == nullptr)
    {
        log.Print(MessageType::ERROR, "No look definitions found!");
        return false; //There has to be at least one look defined!
    }
    
    //Iterate through all looks
    while(look != nullptr)
    {
        const char* name = nullptr;
        Color color = Color::Gray(1.f);
        Scalar roughness;
        Scalar metalness;
        Scalar reflectivity;
        std::pair<Scalar, Scalar> tempRange;
        const char* texture = nullptr;
        std::string textureStr = "";
        const char* normalMap = nullptr;
        std::string normalMapStr = "";
        const char* tempMap = nullptr;
        std::string tempMapStr = "";
        
        if(look->QueryStringAttribute("name", &name) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Look name missing!");
            return false;
        }
        std::string lookName(name);

        if(!ParseColor(look, color))
        {
            log.Print(MessageType::ERROR, "Color of look '%s' missing!", lookName.c_str());
            return false;    
        }
        if(look->QueryAttribute("roughness", &roughness) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Roughness of look '%s' missing!", lookName.c_str());
            return false;
        }
        if(look->QueryAttribute("metalness", &metalness) != XML_SUCCESS)
            metalness = Scalar(0);
        if(look->QueryAttribute("reflectivity", &reflectivity) != XML_SUCCESS)
            reflectivity = Scalar(0);
        if(look->QueryStringAttribute("texture", &texture) == XML_SUCCESS)
            textureStr = GetFullPath(std::string(texture));
        if(look->QueryStringAttribute("normal_map", &normalMap) == XML_SUCCESS)
            normalMapStr = GetFullPath(std::string(normalMap));
        
        if(look->QueryAttribute("temperature", &tempRange.first) == XML_SUCCESS)
        {
            tempRange.second = tempRange.first;
            sm_->CreateLook(lookName, color, roughness, metalness, reflectivity, textureStr, normalMapStr, "", tempRange);
        }
        else if(look->QueryStringAttribute("temperature_map", &tempMap) == XML_SUCCESS
                && look->QueryAttribute("temperature_min", &tempRange.first) == XML_SUCCESS
                && look->QueryAttribute("temperature_max", &tempRange.second) == XML_SUCCESS)
        {
            tempMapStr = GetFullPath(std::string(tempMap));
            sm_->CreateLook(lookName, color, roughness, metalness, reflectivity, textureStr, normalMapStr, tempMapStr, tempRange);
        }
        else
        sm_->CreateLook(lookName, color, roughness, metalness, reflectivity, textureStr, normalMapStr);
        look = look->NextSiblingElement("look");
    }
    
    return true;
}

std::unique_ptr<VelocityField> ScenarioParser::ParseVelocityField(XMLElement* element)
{
    //Get type of current
    const char* vfType;
    if(element->QueryStringAttribute("type", &vfType) != XML_SUCCESS)
    {
        log.Print(MessageType::WARNING, "Velocity field type missing - skipping.");
        return nullptr;
    }
    std::string vfTypeStr(vfType);

    if(vfTypeStr == "uniform")
    {
        XMLElement* item;
        const char* vel;
        Vector3 v;
            
        if((item = element->FirstChildElement("velocity")) == nullptr
            || item->QueryStringAttribute("xyz", &vel) != XML_SUCCESS
            || !ParseVector(vel, v))
        {
            log.Print(MessageType::WARNING, "Velocity definition of uniform velocity field missing - skipping.");
            return nullptr;        
        }
        return std::make_unique<Uniform>(v);
    }
    else if(vfTypeStr == "jet")
    {
        XMLElement* item;
        const char* center;
        const char* vel;
        Vector3 c;
        Vector3 v;
        Scalar radius;
                   
        if((item = element->FirstChildElement("center")) == nullptr
            || item->QueryStringAttribute("xyz", &center) != XML_SUCCESS
            || !ParseVector(center, c))
        {
            log.Print(MessageType::WARNING, "Center definition of jet velocity field missing - skipping.");
            return nullptr;
        }
        if((item = element->FirstChildElement("outlet")) == nullptr
            || item->QueryAttribute("radius", &radius) != XML_SUCCESS)
        {
            log.Print(MessageType::WARNING, "Outlet radius definition of jet velocity field missing - skipping.");
            return nullptr;               
        }
        if((item = element->FirstChildElement("velocity")) == nullptr
            || item->QueryStringAttribute("xyz", &vel) != XML_SUCCESS
            || !ParseVector(vel, v))
        {
            log.Print(MessageType::WARNING, "Velocity definition of jet velocity field missing - skipping.");
            return nullptr;
        }
        Vector3 dir = v.normalized();
        return std::make_unique<Jet>(c, dir, radius, v.norm());
    }
    else
    {
        log.Print(MessageType::WARNING, "Velocity field type not supported - skipping.");
        return nullptr;
    }
}

bool ScenarioParser::ParseStatic(XMLElement* element)
{
    //---- Basic ----
    const char* name = nullptr;
    const char* type = nullptr;
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Name of static body missing!");
        return false;
    }
    std::string objectName(name);
    if(element->QueryStringAttribute("type", &type) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Type of static body '%s' missing!", objectName.c_str());
        return false;
    }
    std::string typestr(type);
        
    //---- Common ----
    XMLElement* item;
    const char* mat = nullptr;
    const char* look = nullptr;
    unsigned int uvMode = 0;
    float uvScale = 1.f;
    Transform trans;
    
    //Material
    if((item = element->FirstChildElement("material")) == nullptr
        || item->QueryStringAttribute("name", &mat) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Material of static body '%s' not properly defined!", objectName.c_str());
        return false;
    }
    //Look
    if((item = element->FirstChildElement("look")) == nullptr
       || item->QueryStringAttribute("name", &look) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Look of static body '%s' not properly defined!", objectName.c_str());
        return false;
    }
    else
    {
        item->QueryAttribute("uv_mode", &uvMode); //Optional
        item->QueryAttribute("uv_scale", &uvScale); //Optional
    }
    //Transform
    if((item = element->FirstChildElement("world_transform")) == nullptr || !ParseTransform(item, trans))
    {
        log.Print(MessageType::ERROR, "Initial pose of static body '%s', in the world frame, missing!", objectName.c_str());
        return false;
    }
  
    //---- Object specific ----
    std::unique_ptr<StaticEntity> object {};

    if(typestr == "box")
    {
        const char* dims = nullptr;
        Vector3 dim;
        if((item = element->FirstChildElement("dimensions")) == nullptr
           || item->QueryStringAttribute("xyz", &dims) != XML_SUCCESS
           || !ParseVector(dims, dim))
        {
            log.Print(MessageType::ERROR, "Dimensions of static body '%s' not properly defined!", objectName.c_str());
            return false;
        }
        Transform origin = I4();
        if((item = element->FirstChildElement("origin")) != nullptr) 
            ParseTransform(item, origin);
        object = std::make_unique<Obstacle>(objectName, dim, origin, std::string(mat), std::string(look), uvMode);
    }
    else if(typestr == "cylinder")
    {
        Scalar radius, height;
        if((item = element->FirstChildElement("dimensions")) == nullptr
            || item->QueryAttribute("radius", &radius) != XML_SUCCESS
            || item->QueryAttribute("height", &height) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Dimensions of static body '%s' not properly defined!", objectName.c_str());
            return false;
        }       
        Transform origin = I4();
        if((item = element->FirstChildElement("origin")) != nullptr) 
            ParseTransform(item, origin);
        object = std::make_unique<Obstacle>(objectName, radius, height, origin, std::string(mat), std::string(look));
    }
    else if(typestr == "sphere")
    {
        Scalar radius;
        if((item = element->FirstChildElement("dimensions")) == nullptr
            || item->QueryAttribute("radius", &radius) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Dimensions of static body '%s' not properly defined!", objectName.c_str());
            return false;
        }
        Transform origin = I4();
        if((item = element->FirstChildElement("origin")) != nullptr) 
            ParseTransform(item, origin);
        object = std::make_unique<Obstacle>(objectName, radius, origin, std::string(mat), std::string(look));
    }
    else if(typestr == "model")
    {
        const char* phyMesh = nullptr;
        Scalar phyScale(1);
        Transform phyOrigin;
        bool convex = false;
        
        if((item = element->FirstChildElement("physical")) == nullptr)
        {
            log.Print(MessageType::ERROR, "Physical mesh of static body '%s' not defined!", objectName.c_str());
            return false;
        }
        if((item = item->FirstChildElement("mesh")) == nullptr
            || item->QueryStringAttribute("filename", &phyMesh) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Physical mesh of static body '%s' not properly defined!", objectName.c_str());
            return false;
        }
        item->QueryAttribute("scale", &phyScale);
        item->QueryAttribute("convex", &convex); 
        if((item = item->NextSiblingElement("origin")) == nullptr || !ParseTransform(item, phyOrigin))
        {
            log.Print(MessageType::ERROR, "Physical mesh of static body '%s' not properly defined!", objectName.c_str());
            return false;
        }
        
        if((item = element->FirstChildElement("visual")) != nullptr)
        {
            const char* graMesh = nullptr;
            Scalar graScale(1);
            Transform graOrigin;
            
            if((item = item->FirstChildElement("mesh")) == nullptr
               || item->QueryStringAttribute("filename", &graMesh) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Visual mesh of static body '%s' not properly defined!", objectName.c_str());
                return false;
            }
            item->QueryAttribute("scale", &graScale);
            if((item = item->NextSiblingElement("origin")) == nullptr || !ParseTransform(item, graOrigin))
            {
                log.Print(MessageType::ERROR, "Visual mesh of static body '%s' not properly defined!", objectName.c_str());
                return false;
            }
            object = std::make_unique<Obstacle>(objectName, GetFullPath(std::string(graMesh)), graScale, graOrigin, GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, convex, std::string(mat), std::string(look));
        }
        else
        {
            object = std::make_unique<Obstacle>(objectName, GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, convex, std::string(mat), std::string(look));
        }
    }
    else if(typestr == "plane")
    {
        object = std::make_unique<Plane>(objectName, Scalar(10000), std::string(mat), std::string(look), uvScale);
    } 
    else if(typestr == "terrain")
    {
        const char* heightmap = nullptr;
        Scalar scaleX, scaleY, height;
            
        if((item = element->FirstChildElement("height_map")) == nullptr
           || item->QueryStringAttribute("filename", &heightmap) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Heightmap of terrain '%s' not properly defined!", objectName.c_str());
            return false;
        }
        if((item = element->FirstChildElement("dimensions")) == nullptr
            || item->QueryAttribute("scalex", &scaleX) != XML_SUCCESS
            || item->QueryAttribute("scaley", &scaleY) != XML_SUCCESS
            || item->QueryAttribute("height", &height) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Dimensions of terrain '%s' not properly defined!", objectName.c_str());
            return false;
        }   
        object = std::make_unique<Terrain>(objectName, GetFullPath(std::string(heightmap)), scaleX, scaleY, height, std::string(mat), std::string(look), uvScale);
    }
    else
    {
        log.Print(MessageType::ERROR, "Unknown type of static body '%s'!", objectName.c_str());
        return false;
    }

    //---- Vision sensors ----
    item = element->FirstChildElement("sensor");
    while(item != nullptr)
    {
        if(!ParseSensor(item, static_cast<Entity*>(object.get())))
        {
            log.Print(MessageType::ERROR, "Sensor of static body '%s' not properly defined!", objectName.c_str());
            return false;
        }
        item = item->NextSiblingElement("sensor");
    }

    //---- Lights ----
    item = element->FirstChildElement("light");
    while(item != nullptr)
    {
        std::unique_ptr<Light> l = ParseLight(item, object->getName());
        if(l == nullptr)
        {
            log.Print(MessageType::ERROR, "Light of static body '%s' not properly defined!", objectName.c_str());
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                log.Print(MessageType::ERROR, "Light of static body '%s' not properly defined!", objectName.c_str());
                return false;
            }
            l->AttachToStatic(object.get(), origin);
            sm_->AddActuator(std::move(l));
        }
        item = item->NextSiblingElement("light");
    }
    
    //---- Communication devices ----
    item = element->FirstChildElement("comm");
    while(item != nullptr)
    {
        std::unique_ptr<Comm> comm = ParseComm(item, object->getName());
        if(comm == nullptr)
        {
            log.Print(MessageType::ERROR, "Communication device of static body '%s' not properly defined!", objectName.c_str());
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                log.Print(MessageType::ERROR, "Communication device of static body '%s' not properly defined!", objectName.c_str());
                return false;
            }
            comm->AttachToStatic(object.get(), origin);
            sm_->AddComm(std::move(comm));
        }
        item = item->NextSiblingElement("comm");
    }

    //---- Add to world ----
    sm_->AddStaticEntity(std::move(object), trans);

    return true;
}

bool ScenarioParser::ParseAnimated(XMLElement* element)
{
    //---- Basic ----
    const char* name = nullptr;
    const char* type = nullptr;
    bool collides = false;
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Name of animated body missing!");
        return false;
    }
    std::string objectName(name);

    if(element->QueryStringAttribute("type", &type) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Type of animated body '%s' missing!", objectName.c_str());
        return false;
    }
    std::string typestr(type);
    
    element->QueryAttribute("collisions", &collides); //Optional
    
    //---- Common ----
    XMLElement* item;
    const char* mat = nullptr;
    const char* look = nullptr;
    unsigned int uvMode = 0;
    float uvScale = 1.f;
    Transform trans;
    
    if(typestr != "empty")
    {
        //Material
        if((item = element->FirstChildElement("material")) == nullptr
           || item->QueryStringAttribute("name", &mat) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Material definition for animated body '%s' missing!", objectName.c_str());
            return false;
        }
        //Look
        if((item = element->FirstChildElement("look")) == nullptr
            || item->QueryStringAttribute("name", &look) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Look definition for animated body '%s' missing", objectName.c_str());
            return false;
        }
        else
        {
            item->QueryAttribute("uv_mode", &uvMode); //Optional
            item->QueryAttribute("uv_scale", &uvScale); //Optional
        }
    }
        
    //---- Trajectory ----
    std::unique_ptr<Trajectory> tr;
    if((item = element->FirstChildElement("trajectory")) != nullptr)
    {
        const char* trType = nullptr;
        if(item->QueryStringAttribute("type", &trType) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Trajectory type not defined for animated body '%s'!", objectName.c_str());
            return false;
        }
        std::string trTypeStr(trType);

        const char* playMode = nullptr;
        if(item->QueryStringAttribute("playback", &playMode) != XML_SUCCESS && trTypeStr != "manual")
        {
            log.Print(MessageType::ERROR, "Trajectory playback mode not defined for animated body '%s'!", objectName.c_str());
            return false;
        }
        
        std::string playModeStr = playMode == nullptr ? "onetime" : std::string(playMode);
        PlaybackMode pm;
        if(playModeStr == "onetime")
            pm = PlaybackMode::ONETIME;
        else if(playModeStr == "repeat")
            pm = PlaybackMode::REPEAT;
        else if(playModeStr == "boomerang")
            pm = PlaybackMode::BOOMERANG;
        else
        {
            log.Print(MessageType::ERROR, "Incorrect trajectory playback mode for animated body '%s'!", objectName.c_str());
            return false;
        }
        
        if(trTypeStr == "manual")
        {
            tr = std::make_unique<ManualTrajectory>();
            XMLElement* key = item->FirstChildElement("keypoint");
            Transform T;
            Scalar t;    
            if(key != nullptr && key->QueryAttribute("time", &t) == XML_SUCCESS
                && ParseTransform(key, T) && t == 0.0)
            {
                static_cast<ManualTrajectory*>(tr.get())->setTransform(T);
            }
        }
        else if(trTypeStr == "pwl" || trTypeStr == "spline" || trTypeStr == "catmull-rom")
        {
            if(trTypeStr == "pwl")
                tr = std::make_unique<PWLTrajectory>(pm);
            else if(trTypeStr == "spline")
                tr = std::make_unique<BSTrajectory>(pm);
            else
                tr = std::make_unique<CRTrajectory>(pm);
            
            PWLTrajectory* pwl = static_cast<PWLTrajectory*>(tr.get()); //Spline has the same mechanism of adding points
            
            XMLElement* key = item->FirstChildElement("keypoint");
            while(key != nullptr)
            {
                Transform T;
                Scalar t;
                if(key->QueryAttribute("time", &t) != XML_SUCCESS || !ParseTransform(key, T))
                {
                    log.Print(MessageType::ERROR, "Trajectory keypoint not properly defined for animated body '%s'!", objectName.c_str());
                    return false;
                }
                pwl->AddKeyPoint(t, T);
                key = key->NextSiblingElement("keypoint");
            }
        }
        else
        {
            log.Print(MessageType::ERROR, "Unknown trajectory type for animated body '%s'!", objectName.c_str());
            return false;        
        }
    }
    else
    {
        log.Print(MessageType::ERROR, "No trajectory defined for animated body '%s'!", objectName.c_str());
        return false;
    }

    //---- Object specific ----
    std::unique_ptr<AnimatedEntity> object {};
        
    if(typestr == "empty")
    {
        object = std::make_unique<AnimatedEntity>(objectName, std::move(tr));
    }
    else if(typestr == "box")
    {
        const char* dims = nullptr;
        Vector3 dim;
        Transform origin;

        if((item = element->FirstChildElement("dimensions")) == nullptr
           || item->QueryStringAttribute("xyz", &dims) != XML_SUCCESS
           || !ParseVector(dims, dim))
        {
            log.Print(MessageType::ERROR, "Dimensions of animated body '%s' not properly defined!", objectName.c_str());
            return false;
        }
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
        {
            log.Print(MessageType::ERROR, "Origin frame of animated body '%s' not properly defined!", objectName.c_str());
            return false;        
        }
        object = std::make_unique<AnimatedEntity>(objectName, std::move(tr), dim, origin, std::string(mat), std::string(look), collides);
    }
    else if(typestr == "cylinder")
    {
        Scalar radius;
        Scalar height;
        Transform origin;
            
        if((item = element->FirstChildElement("dimensions")) == nullptr
            || item->QueryAttribute("radius", &radius) != XML_SUCCESS
            || item->QueryAttribute("height", &height) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Dimensions of animated body '%s' not properly defined!", objectName.c_str());
            return false;
        }
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
        {
            log.Print(MessageType::ERROR, "Origin frame of animated body '%s' not properly defined!", objectName.c_str());
            return false;
        }            
        object = std::make_unique<AnimatedEntity>(objectName, std::move(tr), radius, height, origin, std::string(mat), std::string(look), collides);
    }
    else if(typestr == "sphere")
    {
        Scalar radius;
        Transform origin;
            
        if((item = element->FirstChildElement("dimensions")) == nullptr
            || item->QueryAttribute("radius", &radius) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Dimensions of animated body '%s' not properly defined!", objectName.c_str());
            return false;
        }
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
        {
            log.Print(MessageType::ERROR, "Origin frame of animated body '%s' not properly defined!", objectName.c_str());
            return false;
        }
        object = std::make_unique<AnimatedEntity>(objectName, std::move(tr), radius, origin, std::string(mat), std::string(look), collides);
    }
    else if(typestr == "model")
    {
        const char* phyMesh = nullptr;
        Scalar phyScale;
        Transform phyOrigin;
        
        if((item = element->FirstChildElement("physical")) == nullptr)
        {
            log.Print(MessageType::ERROR, "Physical mesh of animated body '%s' not defined!", objectName.c_str());
            return false;
        }
        if((item = item->FirstChildElement("mesh")) == nullptr
            || item->QueryStringAttribute("filename", &phyMesh) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Physical mesh of animated body '%s' not properly defined!", objectName.c_str());
            return false;
        }
        if(item->QueryAttribute("scale", &phyScale) != XML_SUCCESS)
            phyScale = Scalar(1);
        if((item = item->NextSiblingElement("origin")) == nullptr || !ParseTransform(item, phyOrigin))
        {
            log.Print(MessageType::ERROR, "Physical mesh of animated body '%s' not properly defined!", objectName.c_str());
            return false;
        }
        
        if((item = element->FirstChildElement("visual")) != nullptr)
        {
            const char* graMesh = nullptr;
            Scalar graScale;
            Transform graOrigin;
            
            if((item = item->FirstChildElement("mesh")) == nullptr
               || item->QueryStringAttribute("filename", &graMesh) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Visual mesh of animated body '%s' not properly defined!", objectName.c_str());
                return false;
            }
            if(item->QueryAttribute("scale", &graScale) != XML_SUCCESS)
                graScale = Scalar(1);
            if((item = item->NextSiblingElement("origin")) == nullptr || !ParseTransform(item, graOrigin))
            {
                log.Print(MessageType::ERROR, "Visual mesh of animated body '%s' not properly defined!", objectName.c_str());
                return false;
            }
            object = std::make_unique<AnimatedEntity>(objectName, std::move(tr), GetFullPath(std::string(graMesh)), graScale, graOrigin, GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, std::string(mat), std::string(look), collides);
        }
        else
        {
            object = std::make_unique<AnimatedEntity>(objectName, std::move(tr), GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, std::string(mat), std::string(look), collides);
        }
    }
    else
    {
        log.Print(MessageType::ERROR, "Incorrect animated body type!");
        return false;
    }
        
    //---- Sensors ----
    item = element->FirstChildElement("sensor");
    while(item != nullptr)
    {
        if(!ParseSensor(item, static_cast<Entity*>(object.get())))
        {
            log.Print(MessageType::ERROR, "Sensor of animated body '%s' not properly defined!", objectName.c_str());
            return false;
        }
        item = item->NextSiblingElement("sensor");
    }

    //---- Lights ----
    item = element->FirstChildElement("light");
    while(item != nullptr)
    {
        std::unique_ptr<Light> l = ParseLight(item, object->getName());
        if(l == nullptr)
        {
            log.Print(MessageType::ERROR, "Light of animated body '%s' not properly defined!", objectName.c_str());
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                log.Print(MessageType::ERROR, "Light of animated body '%s' not properly defined!", objectName.c_str());
                return false;
            }
            l->AttachToAnimated(object.get(), origin);
            sm_->AddActuator(std::move(l));
        }
        item = item->NextSiblingElement("light");
    }

    //---- Communication devices ----
    item = element->FirstChildElement("comm");
    while(item != nullptr)
    {
        std::unique_ptr<Comm> comm = ParseComm(item, object->getName());
        if(comm == nullptr)
        {
            log.Print(MessageType::ERROR, "Communication device of animated body '%s' not properly defined!", objectName.c_str());
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                log.Print(MessageType::ERROR, "Communication device of animated body '%s' not properly defined!", objectName.c_str());
                return false;
            }
            comm->AttachToSolid(object.get(), origin);
            sm_->AddComm(std::move(comm));
        }
        item = item->NextSiblingElement("comm");
    }

    //---- Add to world ----
    sm_->AddAnimatedEntity(std::move(object));

    return true;
}

bool ScenarioParser::ParseDynamic(XMLElement* element)
{
    //---- Solid ----
    std::unique_ptr<SolidEntity> solid {};
    if((solid = ParseSolid(element)) == nullptr)
        return false;
    
    XMLElement* item;
    Transform trans;
    if((item = element->FirstChildElement("world_transform")) == nullptr || !ParseTransform(item, trans))
    {
        log.Print(MessageType::ERROR, "Initial pose of dynamic object '%s', in the world frame, missing!", solid->getName().c_str());
        return false;
    }

    //---- Sensors -----
    item = element->FirstChildElement("sensor");
    while(item != nullptr)
    {
        if(!ParseSensor(item, static_cast<Entity*>(solid.get())))
        {
            log.Print(MessageType::ERROR, "Sensor of dynamic body '%s' not properly defined!", solid->getName().c_str());
            return false;
        }
        item = item->NextSiblingElement("sensor");
    }

    //---- Lights ----
    item = element->FirstChildElement("light");
    while(item != nullptr)
    {
        std::unique_ptr<Light> l = ParseLight(item, solid->getName());
        if(l == nullptr)
        {
            log.Print(MessageType::ERROR, "Light of dynamic body '%s' not properly defined!", solid->getName().c_str());
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                log.Print(MessageType::ERROR, "Lght of dynamic body '%s' not properly defined!", solid->getName().c_str());
                return false;
            }
            l->AttachToSolid(solid.get(), origin);
            sm_->AddActuator(std::move(l));
        }
        item = item->NextSiblingElement("light");
    }

    //---- Communication devices ----
    item = element->FirstChildElement("comm");
    while(item != nullptr)
    {
        std::unique_ptr<Comm> comm = ParseComm(item, solid->getName());
        if(comm == nullptr)
        {
            log.Print(MessageType::ERROR, "Communication device of dynamic body '%s' not properly defined!", solid->getName().c_str());
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                log.Print(MessageType::ERROR, "Communication device of dynamic body '%s' not properly defined!", solid->getName().c_str());
                return false;
            }
            comm->AttachToSolid(solid.get(), origin);
            sm_->AddComm(std::move(comm));
        }
        item = item->NextSiblingElement("comm");
    }

    //---- Add to world ----
    sm_->AddSolidEntity(std::move(solid), trans);

    return true;
}

bool ScenarioParser::ParseCable(XMLElement* element)
{
    //---- Basic ----
    const char* name = nullptr;
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Name of cable missing!");
        return false;
    }
    std::string cableName(name);

    const char* phyType = nullptr;
    PhysicsSettings phy;
    if(element->QueryStringAttribute("physics", &phyType) == XML_SUCCESS)
    {
        std::string phyTypeStr(phyType);
        if(phyTypeStr == "disabled")
            phy.mode = PhysicsMode::DISABLED;
        else if(phyTypeStr == "surface")
            phy.mode = PhysicsMode::SURFACE;
        else if(phyTypeStr == "floating")
            phy.mode = PhysicsMode::FLOATING;
        else if(phyTypeStr == "submerged")
            phy.mode = PhysicsMode::SUBMERGED;
        else if(phyTypeStr == "aerodynamic")
            phy.mode = PhysicsMode::AERODYNAMIC;
        else 
        {
            log.Print(MessageType::ERROR, "Incorrect physics type for cable '%s'!", cableName.c_str());
            return false;
        }
    }
    element->QueryAttribute("buoyant", &phy.buoyancy);
    element->QueryAttribute("collisions", &phy.collisions);

    //---- Cable specific ----
    XMLElement* item;
    const char* mat = nullptr;
    const char* look = nullptr;
    float uvScale = 1.f;
    
    //Material
    Scalar stretchFactor {0};
    if((item = element->FirstChildElement("material")) == nullptr
       || item->QueryStringAttribute("name", &mat) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Material of cable '%s' not properly defined!", cableName.c_str());
        return false;
    }
    item->QueryAttribute("stretch_factor", &stretchFactor); //Optional

    //Look
    if((item = element->FirstChildElement("look")) == nullptr
       || item->QueryStringAttribute("name", &look) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Look of cable '%s' not properly defined!", cableName.c_str());
        return false;
    }
    item->QueryAttribute("uv_scale", &uvScale); //Optional

    //Dimensions
    Scalar diameter;
    unsigned int numSegments;
    if((item = element->FirstChildElement("geometry")) == nullptr
        || item->QueryAttribute("diameter", &diameter) != XML_SUCCESS
        || item->QueryAttribute("number_of_segments", &numSegments) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Geometry of cable '%s' not properly defined!", cableName.c_str());
        return false;
    }
    
    //Ends
    const char* firstPosition = nullptr;
    const char* firstAnchorType = nullptr;
    if((item = element->FirstChildElement("first_end")) == nullptr
        || item->QueryStringAttribute("position", &firstPosition) != XML_SUCCESS
        || item->QueryStringAttribute("anchor", &firstAnchorType) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Definition of the first end of cable '%s' missing!", cableName.c_str());
        return false;
    }
    
    Vector3 first;
    if(!ParseVector(firstPosition, first))
    {
        log.Print(MessageType::ERROR, "Position of the first end of cable '%s' not properly defined!", cableName.c_str());
        return false;
    }

    const char* secondPosition = nullptr;
    const char* secondAnchorType = nullptr;
    if((item = element->FirstChildElement("second_end")) == nullptr
        || item->QueryStringAttribute("position", &secondPosition) != XML_SUCCESS
        || item->QueryStringAttribute("anchor", &secondAnchorType) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Definition of the second end of cable '%s' missing!", cableName.c_str());
        return false;
    }
    
    Vector3 second;
    if(!ParseVector(secondPosition, second))
    {
        log.Print(MessageType::ERROR, "Position of the second end of cable '%s' not properly defined!", cableName.c_str());
        return false;
    }

    //---- Add to world ----
    std::unique_ptr<CableEntity> cable = std::make_unique<CableEntity>(cableName, phy, first, second, numSegments, diameter, std::string(mat), std::string(look), stretchFactor, uvScale);
    CableEntity* cablePtr = cable.get();
    sm_->AddEntity(std::move(cable));

    //---- Anchors ----
    auto parseAnchor = [&](XMLElement* anchorItem, const std::string& anchorTypeStr, sf::CableEnds anchorEnd) -> bool
    {
        if (anchorTypeStr == "world")
        {
            cablePtr->AttachToWorld(anchorEnd);
            return true;
        }
        else if (anchorTypeStr == "dynamic")
        {
            XMLElement* childItem;
            const char* bodyName;
            if ((childItem = anchorItem->FirstChildElement("body")) != nullptr
                && childItem->QueryStringAttribute("name", &bodyName) == XML_SUCCESS)
            {        
                SolidEntity* body = dynamic_cast<SolidEntity*>(sm_->getEntity(std::string(bodyName)));
                if (body != nullptr)
                {
                    cablePtr->AttachToSolid(anchorEnd, body);
                    return true;
                }
                else
                {
                    log.Print(MessageType::ERROR, "Dynamic body '%s', to which the first end of cable '%s' is anchored, not found!", bodyName, cableName.c_str());
                    return false;
                }
            }
            else 
            {
                log.Print(MessageType::ERROR, "Dynamic body, to which the first end of cable '%s' is anchored, not properly defined!", cableName.c_str());
                return false;
            }
        }
        else if (anchorTypeStr == "free")
        {
            return true;
        }
        else
        {
            log.Print(MessageType::ERROR, "Incorrect anchor type for cable '%s'!", cableName.c_str());
            return false;
        }
    };
    
    parseAnchor(element->FirstChildElement("first_end"), std::string(firstAnchorType), sf::CableEnds::FIRST);
    parseAnchor(element->FirstChildElement("second_end"), std::string(secondAnchorType), sf::CableEnds::SECOND);

    return true;
}

std::unique_ptr<SolidEntity> ScenarioParser::ParseSolid(XMLElement* element, std::string ns, bool compoundPart)
{
    //---- Basic ----
    const char* name = nullptr;
    const char* type = nullptr;
    const char* phyType = nullptr;
    PhysicsSettings phy;
    
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Rigid body name missing!");
        return nullptr;
    }
    std::string solidName = ns != "" ? ns + "/" + std::string(name) : std::string(name);

    if(element->QueryStringAttribute("type", &type) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Type of rigid body '%s' missing!", solidName.c_str());
        return nullptr;
    }
    if(element->QueryStringAttribute("physics", &phyType) == XML_SUCCESS)
    {
        std::string phyTypeStr(phyType);
        if(phyTypeStr == "disabled")
            phy.mode = PhysicsMode::DISABLED;
        else if(phyTypeStr == "surface")
            phy.mode = PhysicsMode::SURFACE;
        else if(phyTypeStr == "floating")
            phy.mode = PhysicsMode::FLOATING;
        else if(phyTypeStr == "submerged")
            phy.mode = PhysicsMode::SUBMERGED;
        else if(phyTypeStr == "aerodynamic")
            phy.mode = PhysicsMode::AERODYNAMIC;
        else 
        {
            log.Print(MessageType::ERROR, "Incorrect physics type for rigid body '%s'!", solidName.c_str());
            return nullptr;
        }
    }
    element->QueryAttribute("buoyant", &phy.buoyancy);
    element->QueryAttribute("collisions", &phy.collisions);
    
    std::unique_ptr<SolidEntity> solid {};
    XMLElement* item;
    std::string typeStr(type);
    
    if(typeStr == "compound")
    {
        //First external part
        std::unique_ptr<SolidEntity> part {};
        std::unique_ptr<Compound> comp {};
        Transform partOrigin;
      
        if((item = element->FirstChildElement("external_part")) == nullptr
            || (part = ParseSolid(item, solidName, true)) == nullptr)
        {
            log.Print(MessageType::ERROR, "No properly defined external part of compound rigid body '%s' found!", solidName.c_str());
            return nullptr;
        }
        XMLElement* item2;
        if((item2 = item->FirstChildElement("compound_transform")) == nullptr || !ParseTransform(item2, partOrigin))
        {
            log.Print(MessageType::ERROR, "Incorrect definition of external part's '%s' origin frame, for rigid body '%s'!", part->getName().c_str(), solidName.c_str());
            return nullptr;
        }
        comp = std::make_unique<Compound>(solidName, phy, std::move(part), partOrigin);
        
        //Iterate through all external parts
        item = item->NextSiblingElement("external_part");
        while(item != nullptr)
        {
            if((part = ParseSolid(item, solidName, true)) == nullptr)
            {
                log.Print(MessageType::ERROR, "Incorrect definition of external part of rigid body '%s'!", solidName.c_str());
                return nullptr;
            }
            if((item2 = item->FirstChildElement("compound_transform")) == nullptr || !ParseTransform(item2, partOrigin))
            {
                log.Print(MessageType::ERROR, "Incorrect definition of external part's '%s' origin frame, for rigid body '%s'!", part->getName().c_str(), solidName.c_str());
                return nullptr;
            }
                
            comp->AddExternalPart(std::move(part), partOrigin);
            item = item->NextSiblingElement("external_part");
        }
        
        //Iterate through all internal parts
        item = element->FirstChildElement("internal_part");
        while(item != nullptr)
        {
            if((part = ParseSolid(item, solidName, true)) == nullptr)
            {
                log.Print(MessageType::ERROR, "Incorrect definition of internal part of rigid body '%s'!", solidName.c_str());
                return nullptr;
            }
            if((item2 = item->FirstChildElement("compound_transform")) == nullptr || !ParseTransform(item2, partOrigin))
            {
                log.Print(MessageType::ERROR, "Incorrect definition of internal part's '%s' origin frame, for rigid body '%s'!", part->getName().c_str(), solidName.c_str());
                return nullptr;
            }
            bool alwaysVisible = false;
            item->QueryBoolAttribute("always_visible", &alwaysVisible);
                
            comp->AddInternalPart(std::move(part), partOrigin, alwaysVisible);
            item = item->NextSiblingElement("internal_part");
        }
        
        solid = std::move(comp);
    }
    else
    {
        //---- Common ----
        const char* mat = nullptr;
        const char* look = nullptr;
        const char* inertia = nullptr;
        Transform origin;
        Transform cg;
        Scalar mass;
        Scalar ix, iy, iz;
        Vector3 I;
        Vector3 Cf(-1,-1,-1);
        Vector3 Cd(-1,-1,-1);    
        bool cgok;
        unsigned int uvMode = 0;
        float uvScale = 1.f;
        
        //Material
        if((item = element->FirstChildElement("material")) == nullptr
            || item->QueryStringAttribute("name", &mat) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Material of rigid body '%s' not properly defined!", solidName.c_str());
            return nullptr;
        }
        //Look
        if((item = element->FirstChildElement("look")) == nullptr
            || item->QueryStringAttribute("name", &look) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Look of rigid body '%s' not properly defined!", solidName.c_str());
            return nullptr;
        }
        else
        {
            item->QueryAttribute("uv_mode", &uvMode); //Optional
            item->QueryAttribute("uv_scale", &uvScale); //Optional
        }
        //Dynamic parameters  
        if((item = element->FirstChildElement("mass")) == nullptr || item->QueryAttribute("value", &mass) != XML_SUCCESS)
            mass = Scalar(-1);
        if((item = element->FirstChildElement("inertia")) == nullptr 
            || item->QueryStringAttribute("xyz", &inertia) != XML_SUCCESS 
            || sscanf(inertia, "%lf %lf %lf", &ix, &iy, &iz) != 3)
            I = V0();
        else
            I = Vector3(ix, iy, iz);
        cgok = (item = element->FirstChildElement("cg")) != nullptr && ParseTransform(item, cg);
        
        //Hydrodynamic parameters
        if((item = element->FirstChildElement("hydrodynamics")) != nullptr)
        {   
            const char* xyz = nullptr;
            if(item->QueryStringAttribute("viscous_drag", &xyz) == XML_SUCCESS)
                ParseVector(xyz, Cf);
            if(item->QueryStringAttribute("quadratic_drag", &xyz) == XML_SUCCESS)
                ParseVector(xyz, Cd);  
        } 

        //Origin    
        if(typeStr != "model")
        {
            if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            {
                log.Print(MessageType::ERROR, "Definition of origin frame of rigid body '%s' missing!", solidName.c_str());
                return nullptr;
            }
        }
        //---- Specific ----
        if(typeStr == "box")
        {
            const char* dims = nullptr;
            Vector3 dim;
            Scalar thickness;
            
            if((item = element->FirstChildElement("dimensions")) == nullptr
                || item->QueryStringAttribute("xyz", &dims) != XML_SUCCESS
                || !ParseVector(dims, dim))
            {
                log.Print(MessageType::ERROR, "Dimensions of rigid body '%s' not properly defined!", solidName.c_str());
                return nullptr;
            }    
            if(item->QueryAttribute("thickness", &thickness) != XML_SUCCESS)
                thickness = Scalar(-1);
            solid = std::make_unique<Box>(solidName, phy, dim, origin, std::string(mat), std::string(look), thickness, uvMode);
        }
        else if(typeStr == "cylinder")
        {
            Scalar radius, height, thickness;
            if((item = element->FirstChildElement("dimensions")) == nullptr
                || item->QueryAttribute("radius", &radius) != XML_SUCCESS
                || item->QueryAttribute("height", &height) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Dimensions of rigid body '%s' not properly defined!", solidName.c_str());
                return nullptr;
            }
            if(item->QueryAttribute("thickness", &thickness) != XML_SUCCESS)
                thickness = Scalar(-1);
            solid = std::make_unique<Cylinder>(solidName, phy, radius, height, origin, std::string(mat), std::string(look), thickness);
        }
        else if(typeStr == "sphere")
        {
            Scalar radius, thickness;
            if((item = element->FirstChildElement("dimensions")) == nullptr
                || item->QueryAttribute("radius", &radius) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Dimensions of rigid body '%s' not properly defined!", solidName.c_str());
                return nullptr;
            }
            if(item->QueryAttribute("thickness", &thickness) != XML_SUCCESS)
                thickness = Scalar(-1);
            solid = std::make_unique<Sphere>(solidName, phy, radius, origin, std::string(mat), std::string(look), thickness);
        }
        else if(typeStr == "torus")
        {
            Scalar radiusMaj, radiusMin, thickness;
            if((item = element->FirstChildElement("dimensions")) == nullptr
                || item->QueryAttribute("major_radius", &radiusMaj) != XML_SUCCESS
                || item->QueryAttribute("minor_radius", &radiusMin) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Dimensions of rigid body '%s' not properly defined!", solidName.c_str());
                return nullptr;
            }
            if(item->QueryAttribute("thickness", &thickness) != XML_SUCCESS)
                thickness = Scalar(-1);
            solid = std::make_unique<Torus>(solidName, phy, radiusMaj, radiusMin, origin, std::string(mat), std::string(look), thickness);
        }
        else if(typeStr == "wing")
        {
            Scalar baseChord, tipChord, length, thickness;
            const char* naca = nullptr;
            if((item = element->FirstChildElement("dimensions")) == nullptr
               || item->QueryAttribute("base_chord", &baseChord) != XML_SUCCESS
               || item->QueryAttribute("tip_chord", &tipChord) != XML_SUCCESS
               || item->QueryAttribute("length", &length) != XML_SUCCESS
               || item->QueryStringAttribute("naca", &naca) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Dimensions of rigid body '%s' not properly defined!", solidName.c_str());
                return nullptr;
            }
            if(item->QueryAttribute("thickness", &thickness) != XML_SUCCESS)
                thickness = Scalar(-1);

            std::string nacaStr(naca);
            if(nacaStr.size() != 4)
            {
                log.Print(MessageType::ERROR, "Incorrect NACA code for wind '%s'!", solidName.c_str());
                return nullptr;
            }
            solid = std::make_unique<Wing>(solidName, phy, baseChord, tipChord, nacaStr, length, origin, std::string(mat), std::string(look), thickness);
        }
        else if(typeStr == "model")
        {
            const char* phyMesh = nullptr;
            Scalar phyScale(1);
            Transform phyOrigin;
            Scalar thickness(-1);

            if((item = element->FirstChildElement("physical")) == nullptr)
            {
                log.Print(MessageType::ERROR, "Physical mesh of rigid body '%s' not defined!", solidName.c_str());
                return nullptr;
            }
            XMLElement* item2;
            if((item2 = item->FirstChildElement("mesh")) == nullptr
                || item2->QueryStringAttribute("filename", &phyMesh) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Physical mesh of rigid body '%s' not properly defined!", solidName.c_str());
                return nullptr;
            }
            item2->QueryAttribute("scale", &phyScale);
            if((item2 = item->FirstChildElement("thickness")) != nullptr)
                item2->QueryAttribute("value", &thickness);
            if((item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, phyOrigin))
            {
                log.Print(MessageType::ERROR, "Physical mesh of rigid body '%s' not properly defined!", solidName.c_str());
                return nullptr;
            }
            
            if((item = element->FirstChildElement("visual")) != nullptr)
            {
                const char* graMesh = nullptr;
                Scalar graScale(1);
                Transform graOrigin;
                
                if((item = item->FirstChildElement("mesh")) == nullptr
                || item->QueryStringAttribute("filename", &graMesh) != XML_SUCCESS)
                {
                    log.Print(MessageType::ERROR, "Visual mesh of rigid body '%s' not properly defined!", solidName.c_str());
                    return nullptr;
                }
                item->QueryAttribute("scale", &graScale);
                if((item = item->NextSiblingElement("origin")) == nullptr || !ParseTransform(item, graOrigin))
                {
                    log.Print(MessageType::ERROR, "Visual mesh of rigid body '%s' not properly defined!", solidName.c_str());
                    return nullptr;
                }          
                solid = std::make_unique<Polyhedron>(solidName, phy, GetFullPath(std::string(graMesh)), graScale, graOrigin, GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, std::string(mat), std::string(look), thickness);
            }
            else
            {
                solid = std::make_unique<Polyhedron>(solidName, phy, GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, std::string(mat), std::string(look), thickness);
            }
        }
        else
        {
            log.Print(MessageType::ERROR, "Unknown type of rigid body '%s'!", solidName.c_str());
            return nullptr;
        }
         
        //Modify automatically calculated dynamical properties
        if(mass > Scalar(0) && btFuzzyZero(I.length2()) && !cgok)
            solid->ScalePhysicalPropertiesToArbitraryMass(mass);
        else if(mass > Scalar(0) || !btFuzzyZero(I.length2()) || cgok)
        {
            Scalar newMass = mass > Scalar(0) ? mass : solid->getMass();
            Vector3 newI = !btFuzzyZero(I.length2()) ? I : solid->getInertia();
            Transform newCg = cgok ? cg : solid->getCG2OTransform().inverse();  
            solid->SetArbitraryPhysicalProperties(newMass, newI, newCg);
        }
        solid->SetHydrodynamicCoefficients(Cd, Cf);
    }

    //Contact properties (soft contact)
    if(!compoundPart)
    {
        Scalar contactK;
        Scalar contactD;
        
        if((item = element->FirstChildElement("contact")) == nullptr 
            || item->QueryAttribute("stiffness", &contactK) != XML_SUCCESS
            || item->QueryAttribute("damping", &contactD) != XML_SUCCESS)
            contactK = contactD = Scalar(-1);

        if(contactK > Scalar(0) && contactD >= Scalar(0))
        {
            log.Print(MessageType::INFO, "Using soft contact for rigid body '%s'.", solidName.c_str());
            solid->SetContactProperties(true, contactK, contactD);
        }
    }
    return solid;
}

bool ScenarioParser::ParseRobot(XMLElement* element)
{
    //---- Basic ----
    XMLElement* item;
    const char* name = nullptr;
    const char* algo = nullptr;
    bool fixed;
    bool selfCollisions;
    Transform trans;
    std::string algorithm = "featherstone";

    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Robot name missing!");
        return false;
    }
    std::string robotName(name);
    if(element->QueryAttribute("fixed", &fixed) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Base type of robot '%s' not specified!", robotName.c_str());
        return false;
    }
    if(element->QueryAttribute("self_collisions", &selfCollisions) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Self-collision flag for robot '%s' not specified!", robotName.c_str());
        return false;
    }
    if(element->QueryStringAttribute("algorithm", &algo) == XML_SUCCESS)
    {
        algorithm = std::string(algo);
        if(algorithm != "featherstone" && algorithm != "general")
        {
            log.Print(MessageType::ERROR, "Wrong algorithm specified for robot '%s' not specified!", robotName.c_str());
            return false;
        }
    }
    if((item = element->FirstChildElement("world_transform")) == nullptr || !ParseTransform(item, trans))
    {
        log.Print(MessageType::ERROR, "Initial pose of robot '%s', in the world frame, missing!", robotName.c_str());
        return false;
    }

    std::unique_ptr<Robot> robot {};
    if(algorithm == "featherstone")
        robot = std::make_unique<FeatherstoneRobot>(robotName, fixed);
    else if(algorithm == "general")
        robot = std::make_unique<GeneralRobot>(robotName, fixed);

    //---- Links ----
    //Base link
    std::unique_ptr<SolidEntity> baseLink {};
    
    if((item = element->FirstChildElement("base_link")) == nullptr)
    {
        log.Print(MessageType::ERROR, "Base link of robot '%s' missing!", robotName.c_str());
        return false;
    }
    if((baseLink = ParseLink(item, robot.get())) == nullptr)
    {
        log.Print(MessageType::ERROR, "Base link of robot '%s' not properly defined!", robotName.c_str());
        return false;
    }
    
    //Other links
    std::vector<std::unique_ptr<SolidEntity>> links;
    
    item = element->FirstChildElement("link");
    while(item != nullptr)
    {
        std::unique_ptr<SolidEntity> link {};
        if ((link = ParseLink(item, robot.get())) != nullptr)
        {
            links.push_back(std::move(link));
        }
        else
        {
            log.Print(MessageType::ERROR, "Link of robot '%s' not properly defined!", robot->getName().c_str());
            return false;
        }        
        item = item->NextSiblingElement("link");
    }
    
    robot->DefineLinks(std::move(baseLink), std::move(links), selfCollisions);
    
    //---- Joints ----
    item = element->FirstChildElement("joint");
    while(item != nullptr)
    {
        if(!ParseJoint(item, robot.get()))
        {
            log.Print(MessageType::ERROR, "Joint of robot '%s' not properly defined!", robotName.c_str());
            return false;
        }
        item = item->NextSiblingElement("joint");
    }
    
    robot->BuildKinematicStructure();
    
    //---- Sensors ----
    item = element->FirstChildElement("sensor");
    while(item != nullptr)
    {
        if(!ParseSensor(item, robot.get()))
        {
            log.Print(MessageType::ERROR, "Sensor of robot '%s' not properly defined!", robotName.c_str());
            return false;
        }
        item = item->NextSiblingElement("sensor");
    }
    
    //---- Actuators ----
    item = element->FirstChildElement("actuator");
    while(item != nullptr)
    {
        if(!ParseActuator(item, robot.get()))
        {
            log.Print(MessageType::ERROR, "Actuator of robot '%s' not properly defined!", robotName.c_str());
            return false;
        }
        item = item->NextSiblingElement("actuator");
    }

    //---- Lights ----
    item = element->FirstChildElement("light");
    while(item != nullptr)
    {
        std::unique_ptr<Light> l = ParseLight(item, robot->getName());
        if(l == nullptr)
        {
            log.Print(MessageType::ERROR, "Light of robot '%s' not properly defined!", robotName.c_str());
            return false;
        }
        else
        {
            const char* linkName = nullptr;
            Transform origin;
            XMLElement* item2;
            if( ((item2 = item->FirstChildElement("link")) == nullptr || item2->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
                || ((item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin)) )
            {
                log.Print(MessageType::ERROR, "Light of robot '%s' not properly defined!", robotName.c_str());
                return false;
            }
            robot->AddLinkActuator(std::move(l), robot->getName() + "/" + std::string(linkName), origin);
        }
        item = item->NextSiblingElement("light");
    }
    
    //---- Communication devices ----
    item = element->FirstChildElement("comm");
    while(item != nullptr)
    {
        std::unique_ptr<Comm> comm = ParseComm(item, robot->getName());
        if(comm == nullptr)
        {
            log.Print(MessageType::ERROR, "Communication device of robot '%s' not properly defined!", robotName.c_str());
            return false;
        }
        else
        {
            const char* linkName = nullptr;
            Transform origin;
            XMLElement* item2;
            if( ((item2 = item->FirstChildElement("link")) == nullptr || item2->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
                || ((item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin)) )
            {
                log.Print(MessageType::ERROR, "Communication device of robot '%s' not properly defined!", robotName.c_str());
                return false;
            }
            robot->AddComm(std::move(comm), robot->getName() + "/" + std::string(linkName), origin);
        }
        item = item->NextSiblingElement("comm");
    }
    
    sm_->AddRobot(std::move(robot), trans);
    return true;
}

std::unique_ptr<SolidEntity> ScenarioParser::ParseLink(XMLElement* element, Robot* robot)
{
    return ParseSolid(element, robot->getName());
}
        
bool ScenarioParser::ParseJoint(XMLElement* element, Robot* robot)
{
    const char* name = nullptr;
    const char* type = nullptr;
    const char* parent = nullptr;
    const char* child = nullptr; 
    Transform origin;
    
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Joint name missing (robot '%s')!", robot->getName().c_str());   
        return false;
    }
    std::string jointName = robot->getName() + "/" + std::string(name);

    if(element->QueryStringAttribute("type", &type) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Type of joint '%s' missing!", jointName.c_str());   
        return false;
    }
    std::string typeStr(type);
    
    XMLElement* item;
    if((item = element->FirstChildElement("parent")) == nullptr)
    {
        log.Print(MessageType::ERROR, "Parent definition for joint '%s' missing!", jointName.c_str());
        return false;
    }
    if(item->QueryStringAttribute("name", &parent) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Parent name for joint '%s' missing!", jointName.c_str());   
        return false;
    }
    if((item = element->FirstChildElement("child")) == nullptr)
    {
        log.Print(MessageType::ERROR, "Child definition for joint '%s' missing!", jointName.c_str());
        return false;
    }
    if(item->QueryStringAttribute("name", &child) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Child name for joint '%s' missing!", jointName.c_str());   
        return false;    
    }
    if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
    {
        log.Print(MessageType::ERROR, "Origin frame of joint '%s' missing!", jointName.c_str());
        return false;
    }
    
    std::string parentName = robot->getName() + "/" + std::string(parent);
    std::string childName = robot->getName() + "/" + std::string(child);

    if(typeStr == "fixed")
    {
        robot->DefineFixedJoint(jointName, parentName, childName, origin);
    }
    else if(typeStr == "prismatic" || typeStr == "revolute")
    {
        const char* vec = nullptr;
        Vector3 axis;
        Scalar posMin(1);
        Scalar posMax(-1);
        Scalar damping(-1);
        
        if((item = element->FirstChildElement("axis")) == nullptr
            || item->QueryStringAttribute("xyz", &vec) != XML_SUCCESS
            || !ParseVector(vec, axis))
        {
            log.Print(MessageType::ERROR, "Axis of joint '%s' not properly defined!", jointName.c_str());
            return false;
        }     
        if((item = element->FirstChildElement("limits")) != nullptr) //Optional
        {
            if((item->QueryAttribute("min", &posMin) != XML_SUCCESS) || (item->QueryAttribute("max", &posMax) != XML_SUCCESS))
                log.Print(MessageType::WARNING, "Limits of joint '%s' not properly defined - using defaults.", jointName.c_str());
        }
        if((item = element->FirstChildElement("damping")) != nullptr) //Optional
        {
            if(item->QueryAttribute("value", &damping) != XML_SUCCESS)
                log.Print(MessageType::WARNING, "Damping of joint '%s' not properly defined - using defaults.", jointName.c_str());
        }
        
        if(typeStr == "prismatic")
            robot->DefinePrismaticJoint(jointName, parentName, childName, origin, axis, std::make_pair(posMin, posMax), damping);
        else
            robot->DefineRevoluteJoint(jointName, parentName, childName, origin, axis, std::make_pair(posMin, posMax), damping);
    }
    else
    {
        log.Print(MessageType::ERROR, "Joint '%s' has unknown type!", jointName.c_str());
        return false;
    }
    
    return true;
}

bool ScenarioParser::ParseActuator(XMLElement* element, Robot* robot)
{
    //Parse
    std::unique_ptr<Actuator> act = ParseActuator(element, robot->getName());
    if(act == nullptr)
        return false;
    
    //Watchdog
    XMLElement* item;
    Scalar timeout(-1);
    if((item = element->FirstChildElement("watchdog")) != nullptr
        && item->QueryAttribute("timeout", &timeout) == XML_SUCCESS)
        {
            act->setWatchdog(timeout);
        }
   
    //Attach
    switch(act->getType())
    {
        //Joint actuators
        case ActuatorType::JOINT:
        {
            const char* jointName = nullptr;
            
            if((item = element->FirstChildElement("joint")) == nullptr
                || item->QueryStringAttribute("name", &jointName) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Joint definition for actuator '%s' missing!", act->getName().c_str());
                return false;
            }
            robot->AddJointActuator(std::unique_ptr<JointActuator>(static_cast<JointActuator*>(act.release())), robot->getName() + "/" + std::string(jointName));
        }
            break;

        //Link actuators
        case ActuatorType::LINK:
        {
            const char* linkName = nullptr;
            Transform origin; 

            if((item = element->FirstChildElement("link")) == nullptr
                || item->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Link definition for actuator '%s' missing!", act->getName().c_str());
                return false;
            }
            if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            {
                log.Print(MessageType::ERROR, "Origin frame of actuator '%s' missing!", act->getName().c_str());
                return false;
            }
            robot->AddLinkActuator(std::unique_ptr<LinkActuator>(static_cast<LinkActuator*>(act.release())), robot->getName() + "/" + std::string(linkName), origin);
        }
            break;

        //Unsupported
        default:
        {
            log.Print(MessageType::ERROR, "Unsupported actuator type found in definition of robot '%s'!", robot->getName().c_str());
            return false;
        }
            break;
    }
    return true;
}

bool ScenarioParser::ParseSensor(XMLElement* element, Robot* robot)
{
    //Parse
    std::unique_ptr<Sensor> sens = ParseSensor(element, robot->getName());
    if(sens == nullptr)
        return false;

    //Attach
    XMLElement* item;
    switch(sens->getType())
    {
        case SensorType::JOINT:
        {
            const char* jointName = nullptr;
            
            if((item = element->FirstChildElement("joint")) == nullptr
                 || item->QueryStringAttribute("name", &jointName) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Joint definition for sensor '%s' missing!", sens->getName().c_str());
                return false;
            }
            robot->AddJointSensor(std::unique_ptr<JointSensor>(static_cast<JointSensor*>(sens.release())), robot->getName() + "/" + std::string(jointName));
        }
            break;

        case SensorType::LINK:
        case SensorType::VISION:
        {
            const char* linkName = nullptr;
            Transform origin;
        
            if((item = element->FirstChildElement("link")) == nullptr 
                || item->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Link definition for sensor '%s' missing!", sens->getName().c_str());
                return false;
            }
            if((item = element->FirstChildElement("origin")) == nullptr 
                || !ParseTransform(item, origin))
            {
                log.Print(MessageType::ERROR, "Origin frame of sensor '%s' missing!", sens->getName().c_str());
                return false;
            }
            if(sens->getType() == SensorType::LINK)
                robot->AddLinkSensor(std::unique_ptr<LinkSensor>(static_cast<LinkSensor*>(sens.release())), robot->getName() + "/" + std::string(linkName), origin);
            else
                robot->AddVisionSensor(std::unique_ptr<VisionSensor>(static_cast<VisionSensor*>(sens.release())), robot->getName() + "/" + std::string(linkName), origin);
        }
            break;

        //Unsupported
        default:
        {
            log.Print(MessageType::ERROR, "Unsupported sensor type found in definition of robot '%s'!", robot->getName().c_str());
            return false;
        }
            break;
    }
    return true;
}

bool ScenarioParser::ParseSensor(XMLElement* element, Entity* ent)
{
    //Parse
    std::unique_ptr<Sensor> sens = ParseSensor(element, ent != nullptr ? ent->getName() : "");
    if(sens == nullptr)
        return false;

    //Attach
    XMLElement* item;
    switch(sens->getType())
    {
        case SensorType::JOINT:
        {
            log.Print(MessageType::ERROR, "Joint sensors can only be attached to robotic joints!");
            return false;
        }
            break;

        case SensorType::LINK:
        {
            Transform origin;
            if((item = element->FirstChildElement("origin")) == nullptr 
                || !ParseTransform(item, origin))
            {
                return false;
            }
            if(ent == nullptr)
            {
                log.Print(MessageType::ERROR, "Link sensors can only be attached to robotic links and moving bodies!");
                return false;
            }
            else if(ent->getType() == EntityType::SOLID || ent->getType() == EntityType::ANIMATED)
            {
                static_cast<LinkSensor*>(sens.get())->AttachToSolid(static_cast<MovingEntity*>(ent), origin);
                sm_->AddSensor(std::move(sens));
            }
            else
            {
                log.Print(MessageType::ERROR, "Link sensors can only be attached to robotic links and moving bodies!");
                return false;
            }
        }
            break;

        case SensorType::VISION:
        {
            Transform origin;
            if((item = element->FirstChildElement(ent == nullptr ? "world_transform" : "origin")) == nullptr 
                || !ParseTransform(item, origin))
            {
                return false;
            }
            if(ent == nullptr)
                static_cast<VisionSensor*>(sens.get())->AttachToWorld(origin);
            else if(ent->getType() == EntityType::SOLID || ent->getType() == EntityType::ANIMATED)
                static_cast<VisionSensor*>(sens.get())->AttachToSolid(static_cast<MovingEntity*>(ent), origin);
            else if(ent->getType() == EntityType::STATIC)
                static_cast<VisionSensor*>(sens.get())->AttachToStatic(static_cast<StaticEntity*>(ent), origin);   
            else
            {
                log.Print(MessageType::ERROR, "Trying to attach vision sensor to a non-physical body!");
                return false;
            }
            sm_->AddSensor(std::move(sens));
        }
            break;

        default:
        {
            return false;
        }
            break;
    }
    return true;
}

std::unique_ptr<Actuator> ScenarioParser::ParseActuator(XMLElement* element, const std::string& namePrefix)
{
    //---- Common ----
    const char* name = nullptr;
    const char* type = nullptr;
    
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Actuator name missing (namespace '%s')!", namePrefix.c_str());   
        return nullptr;
    }
    std::string actuatorName = std::string(name);
    if(namePrefix != "")
        actuatorName = namePrefix + "/" + actuatorName;

    if(element->QueryStringAttribute("type", &type) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Type of actuator '%s' missing!", actuatorName.c_str());
        return nullptr;
    }
    std::string typeStr(type);
 
    //---- Specific ----
    const ActuatorFactoryEntry* factory = ActuatorFactory::Instance().Find(typeStr);
    if (factory == nullptr)
    {
        log.Print(MessageType::ERROR, "Actuator type '%s' not supported!", typeStr.c_str());
        return nullptr;
    }

    ConstructInfo info = factory->getConstructInfo();

    if (ParseConstructInfo(element, info))
        return factory->construct(actuatorName, info);
    else
        return nullptr;
}

std::unique_ptr<Sensor> ScenarioParser::ParseSensor(XMLElement* element, const std::string& namePrefix)
{
    //---- Common ----
    std::unique_ptr<Sensor> sens {};
    const char* name = nullptr;
    const char* type = nullptr;
    Scalar rate;
    XMLElement* item;
    
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Sensor name missing (namespace '%s')!", namePrefix.c_str());
        return nullptr;
    }
    std::string sensorName = std::string(name);
    if(namePrefix != "")
        sensorName = namePrefix + "/" + sensorName;
    
    if(element->QueryStringAttribute("type", &type) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Type of sensor '%s' missing!", sensorName.c_str());
        return nullptr;
    }
    if(element->QueryAttribute("rate", &rate) != XML_SUCCESS)
        rate = Scalar(-1);
    std::string typeStr(type);

    //---- Specific ----
    const SensorFactoryEntry* factory = SensorFactory::Instance().Find(typeStr);
    if (factory == nullptr)
    {
        log.Print(MessageType::ERROR, "Sensor type '%s' not supported!", typeStr.c_str());
        return nullptr;
    }

    ConstructInfo info = factory->getConstructInfo();

    if (ParseConstructInfo(element, info))
        sens = factory->construct(sensorName, rate, info);
    else
        return nullptr;

    //---- Visuals ----
    const char* visFile = nullptr;
    if((item = element->FirstChildElement("visual")) != nullptr && item->QueryStringAttribute("filename", &visFile) == XML_SUCCESS)
    {
        if(!isGraphicalSim())
        {
            log.Print(MessageType::WARNING, "Visual representation of sensor '%s' not available in console mode!", sensorName.c_str());
            return sens;
        }

        Scalar scale(1.0);
        item->QueryAttribute("scale", &scale);

        const char* look = nullptr;
        std::string lookStr = "";
        if(item->QueryStringAttribute("look", &look) == XML_SUCCESS)
            lookStr = std::string(look);

        sens->setVisual(GetFullPath(std::string(visFile)), scale, lookStr);
    }
    return sens;
}

std::unique_ptr<Light> ScenarioParser::ParseLight(XMLElement* element, const std::string& namePrefix)
{
    if(!isGraphicalSim())
    {
        log.Print(MessageType::ERROR, "Lights not supported in console mode!");
        return nullptr;
    }

    const char* name = nullptr;
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Light name missing (namespace '%s')!", namePrefix.c_str());
        return nullptr;
    }

    std::string lightName = std::string(name);
    if(namePrefix != "")
        lightName = namePrefix + "/" + lightName;
    
    XMLElement* item;
    Scalar illu, radius;
	Scalar cone = Scalar(0);
    Color color = Color::Gray(1.f);
    
    if((item = element->FirstChildElement("specs")) != nullptr)
    {
        if((item->QueryAttribute("illuminance", &illu) != XML_SUCCESS)
           || (item->QueryAttribute("radius", &radius) != XML_SUCCESS))
        {
            log.Print(MessageType::ERROR, "Specs of light '%s' not properly defined!", lightName.c_str());
            return nullptr;
        }        
        item->QueryAttribute("cone_angle", &cone);
    } 
    else
    {
        log.Print(MessageType::ERROR, "Specs of light '%s' not defined!", lightName.c_str());
        return nullptr;
    }
    
    if((item = element->FirstChildElement("color")) == nullptr || !ParseColor(item, color))
    {
        log.Print(MessageType::ERROR, "Color of light '%s' not properly defined!", lightName.c_str());
        return nullptr;
    }
        
    if(cone > Scalar(0))
        return std::make_unique<Light>(lightName, radius, cone, color, illu);
    else
        return std::make_unique<Light>(lightName, radius, color, illu);
}

std::unique_ptr<Comm> ScenarioParser::ParseComm(XMLElement* element, const std::string& namePrefix)
{
    const char* name = nullptr;
    const char* type = nullptr;
    unsigned int devId;
    
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Communication device name missing (namespace '%s')!", namePrefix.c_str());
        return nullptr;
    }
    std::string commName = std::string(name);
    if(namePrefix != "")
        commName = namePrefix + "/" + commName;

    if(element->QueryStringAttribute("type", &type) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Type of communication device '%s' missing!", commName.c_str());
        return nullptr;
    }
    std::string typeStr(type);

    if(element->QueryAttribute("device_id", &devId) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Id of communication device '%s' missing!", commName.c_str());
        return nullptr;
    }
     
    XMLElement* item;
    std::unique_ptr<Comm> comm;
    if(typeStr == "acoustic_modem")
    {
        Scalar minFovDeg;
        Scalar maxFovDeg;
        Scalar range;
        unsigned int cId = 0;
        bool occlusion = true;
        
        if((item = element->FirstChildElement("specs")) == nullptr
            || item->QueryAttribute("min_vertical_fov", &minFovDeg) != XML_SUCCESS
            || item->QueryAttribute("max_vertical_fov", &maxFovDeg) != XML_SUCCESS
            || item->QueryAttribute("range", &range) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Specs of communication device '%s' not properly defined!", commName.c_str());
            return nullptr;
        }
        item = element->FirstChildElement("connect");
        if(item == nullptr || item->QueryAttribute("device_id", &cId) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Communication device '%s' not connected!", commName.c_str());
            return nullptr;
        }
        item->QueryAttribute("occlusion_test", &occlusion);
    
        comm = std::make_unique<AcousticModem>(commName, devId, minFovDeg, maxFovDeg, range);
        comm->Connect(cId);
        static_cast<AcousticModem*>(comm.get())->setOcclusionTest(occlusion);
        return comm;
    }
    else if(typeStr == "usbl")
    {
        Scalar minFovDeg;
        Scalar maxFovDeg;
        Scalar range;
        unsigned int cId = 0;
        Scalar pingRate;
        bool occlusion = true;
        
        if((item = element->FirstChildElement("specs")) == nullptr
            || item->QueryAttribute("min_vertical_fov", &minFovDeg) != XML_SUCCESS
            || item->QueryAttribute("max_vertical_fov", &maxFovDeg) != XML_SUCCESS
            || item->QueryAttribute("range", &range) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Specs of communication device '%s' not properly defined!", commName.c_str());
            return nullptr;
        }
        item = element->FirstChildElement("connect");
        if(item == nullptr || item->QueryAttribute("device_id", &cId) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Communication device '%s' not connected!", commName.c_str());
            return nullptr;
        }
        item->QueryAttribute("occlusion_test", &occlusion);

        comm = std::make_unique<SimpleUSBL>(commName, devId, minFovDeg, maxFovDeg, range);
        comm->Connect(cId);
        static_cast<AcousticModem*>(comm.get())->setOcclusionTest(occlusion);
        
        if((item = element->FirstChildElement("autoping")) != nullptr
            && item->QueryAttribute("rate", &pingRate) == XML_SUCCESS)
            static_cast<USBL*>(comm.get())->EnableAutoPing(pingRate);
        
        //Optional noise definitions
        if((item = element->FirstChildElement("noise")) != nullptr)
        {
            Scalar rangeDev = Scalar(0);
            Scalar hAngleDevDeg = Scalar(0);
            Scalar vAngleDevDeg = Scalar(0);
            int c = 0;
            if(item->QueryAttribute("range", &rangeDev) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("horizontal_angle", &hAngleDevDeg) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("vertical_angle", &vAngleDevDeg) == XML_SUCCESS)
                ++c;
            if(c == 0)
                log.Print(MessageType::WARNING, "Noise of communication device '%s' not properly defined - using defaults.", commName.c_str());
            else
                static_cast<SimpleUSBL*>(comm.get())->setNoise(rangeDev, hAngleDevDeg, vAngleDevDeg);
        }
        //Optional resolution definitions
        if((item = element->FirstChildElement("resolution")) != nullptr)
        {
            Scalar rangeRes = Scalar(0);
            Scalar angleResDeg = Scalar(0);
            int c = 0;
            if(item->QueryAttribute("range", &rangeRes) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("angle", &angleResDeg) == XML_SUCCESS)
                ++c;
            if(c == 0)
                log.Print(MessageType::WARNING, "Resolution of communication device '%s' not properly defined - using defaults.", commName.c_str());
            else
                static_cast<SimpleUSBL*>(comm.get())->setResolution(rangeRes, angleResDeg);
        }
        return comm;
    }
    else if(typeStr == "usbl2")
    {
        Scalar minFovDeg;
        Scalar maxFovDeg;
        Scalar range;
        Scalar freq;
        Scalar baseline;
        unsigned int cId = 0;
        Scalar pingRate;
        bool occlusion = true;
        
        if((item = element->FirstChildElement("specs")) == nullptr
            || item->QueryAttribute("min_vertical_fov", &minFovDeg) != XML_SUCCESS
            || item->QueryAttribute("max_vertical_fov", &maxFovDeg) != XML_SUCCESS
            || item->QueryAttribute("range", &range) != XML_SUCCESS
            || item->QueryAttribute("frequency", &freq) != XML_SUCCESS
            || item->QueryAttribute("baseline", &baseline) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Specs of communication device '%s' not properly defined!", commName.c_str());
            return nullptr;
        }
        item = element->FirstChildElement("connect");
        if(item == nullptr || item->QueryAttribute("device_id", &cId) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Communication device '%s' not connected!", commName.c_str());
            return nullptr;
        }
        item->QueryAttribute("occlusion_test", &occlusion);

        comm = std::make_unique<RealUSBL>(commName, devId, minFovDeg, maxFovDeg, range, freq, baseline);
        comm->Connect(cId);
        static_cast<AcousticModem*>(comm.get())->setOcclusionTest(occlusion);
        
        if((item = element->FirstChildElement("autoping")) != nullptr
            && item->QueryAttribute("rate", &pingRate) == XML_SUCCESS)
            static_cast<USBL*>(comm.get())->EnableAutoPing(pingRate);
        
        //Optional noise definitions
        if((item = element->FirstChildElement("noise")) != nullptr)
        {
            Scalar timeDev(0);
            Scalar svDev(0);
            Scalar phaseDev(0);
            Scalar blError(0);
            Scalar depthDev(0);
            int c = 0;
            if(item->QueryAttribute("tof", &timeDev) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("sound_velocity", &svDev) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("phase", &phaseDev) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("baseline_error", &blError) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("depth", &depthDev) == XML_SUCCESS)
                ++c;
            if(c == 0)
                log.Print(MessageType::WARNING, "Noise of communication device '%s' not properly defined - using defaults.", commName.c_str());
            else
                static_cast<RealUSBL*>(comm.get())->setNoise(timeDev, svDev, phaseDev, blError, depthDev);
        }
        return comm;
    }
    else if(typeStr == "optical_modem" || typeStr == "vlc")
    {
        Scalar fovDeg;
        Scalar range;
        Scalar ambientLightSensitivity {1};
        unsigned int cId {0};
        
        if((item = element->FirstChildElement("specs")) == nullptr
            || item->QueryAttribute("fov", &fovDeg) != XML_SUCCESS
            || item->QueryAttribute("range", &range) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Specs of communication device '%s' not properly defined!", commName.c_str());
            return nullptr;
        }
        item->QueryAttribute("ambient_light_sensitivity", &ambientLightSensitivity);
        
        item = element->FirstChildElement("connect");
        if(item == nullptr || item->QueryAttribute("device_id", &cId) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Communication device '%s' not connected!", commName.c_str());
            return nullptr;
        }
        
        comm = std::make_unique<OpticalModem>(commName, devId, fovDeg, range, ambientLightSensitivity);
        comm->Connect(cId);
        return comm;
    }
    else 
    {
        log.Print(MessageType::ERROR, "Unknown type of communication device '%s'!", commName.c_str());
        return nullptr;
    }
}

bool ScenarioParser::ParseContact(XMLElement* element)
{
    const char* name = nullptr;
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
    {        
        log.Print(MessageType::ERROR, "Name of contact missing!");
        return false;
    }
    std::string contactName(name);
        
    XMLElement* itemA;
    XMLElement* itemB;
    if((itemA = element->FirstChildElement("first_body")) == nullptr
        || (itemB = element->FirstChildElement("second_body")) == nullptr)
    {
        log.Print(MessageType::ERROR, "Body definitions for contact '%s' missing!", contactName.c_str());
        return false;
    }
    
    const char* nameA = nullptr;
    const char* nameB = nullptr;
    const char* dispA = nullptr;
    const char* dispB = nullptr;
    if(itemA->QueryStringAttribute("name", &nameA) != XML_SUCCESS
        || itemB->QueryStringAttribute("name", &nameB) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Body names for contact '%s' missing!", contactName.c_str());
        return false;
    }
    
    Entity* entA;
    Entity* entB;
    entA = sm_->getEntity(std::string(nameA));
    entB = sm_->getEntity(std::string(nameB));
    if(entA == nullptr)
    {
        Robot* rob;
        unsigned int i = 0;
        while((rob = sm_->getRobot(i++)) != nullptr)
        {
            entA = rob->getLink(std::string(nameA));
            if(entA != nullptr)
                break;
        }
    }
    if(entB == nullptr)
    {
        Robot* rob;
        unsigned int i = 0;
        while((rob = sm_->getRobot(i++)) != nullptr)
        {
            entB = rob->getLink(std::string(nameB));
            if(entB != nullptr)
                break;
        }
    }
    if(entA == nullptr || entB == nullptr
       || (entA->getType() != EntityType::SOLID && entA->getType() != EntityType::STATIC)
       || (entB->getType() != EntityType::SOLID && entB->getType() != EntityType::STATIC))
    {
        log.Print(MessageType::ERROR, "Bodies defined for contact '%s' not found!", contactName.c_str());
        return false;
    }
    
    int16_t displayMask = 0;
    if(itemA->QueryStringAttribute("display", &dispA) == XML_SUCCESS)
    {
        std::string dispAStr(dispA);
        if(dispAStr == "force")
            displayMask |= CONTACT_DISPLAY_NORMAL_FORCE_A;
        else if(dispAStr == "slip")
            displayMask |= CONTACT_DISPLAY_LAST_SLIP_VELOCITY_A;
        else if(dispAStr == "path")
            displayMask |= CONTACT_DISPLAY_PATH_A;
    }
    if(itemB->QueryStringAttribute("display", &dispB) == XML_SUCCESS)
    {
        std::string dispBStr(dispB);
        if(dispBStr == "force")
            displayMask |= CONTACT_DISPLAY_NORMAL_FORCE_B;
        else if(dispBStr == "slip")
            displayMask |= CONTACT_DISPLAY_LAST_SLIP_VELOCITY_B;
        else if(dispBStr == "path")
            displayMask |= CONTACT_DISPLAY_PATH_B;
    }
    
    unsigned int history = 0;
    if((itemA = element->FirstChildElement("history")) != nullptr)
        itemA->QueryAttribute("points", &history);
    
    std::unique_ptr<Contact> cnt = std::make_unique<Contact>(contactName, entA, entB, history);
    cnt->setDisplayMask(displayMask);
    sm_->AddContact(std::move(cnt));
    
    return true;
}

FixedJoint* ScenarioParser::ParseGlue(XMLElement* element)
{
    const char* name = nullptr;
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
    {        
        log.Print(MessageType::ERROR, "Name of glue missing!");
        return nullptr;
    }
    std::string glueName(name);
    
    XMLElement* itemA;
    XMLElement* itemB;
    if((itemA = element->FirstChildElement("first_body")) == nullptr
        || (itemB = element->FirstChildElement("second_body")) == nullptr)
    {
        log.Print(MessageType::ERROR, "Body definitions for glue '%s' missing!", glueName.c_str());
        return nullptr;
    }
    
    const char* nameA = nullptr;
    const char* nameB = nullptr;
    if(itemA->QueryStringAttribute("name", &nameA) != XML_SUCCESS
        || itemB->QueryStringAttribute("name", &nameB) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Body names for glue '%s' missing!", glueName.c_str());
        return nullptr;
    }

    bool activated = true;
    element->QueryAttribute("activated", &activated); // Optional

    //Find if bodies are independent dynamic bodies or links of robots
    Entity* entA = sm_->getEntity(std::string(nameA));
    Entity* entB = sm_->getEntity(std::string(nameB));
    FeatherstoneRobot* robotA = nullptr;
    FeatherstoneRobot* robotB = nullptr;
    int linkIdA = -2;
    int linkIdB = -2;
    
    if(entA == nullptr) //Maybe a robot link?
    {
        Robot* rob;
        unsigned int i = 0;
        while((rob = sm_->getRobot(i++)) != nullptr)
        {
            if(rob->getType() == RobotType::FEATHERSTONE)
            {
                FeatherstoneRobot* fr = (FeatherstoneRobot*)rob;
                int linkId = -2;
                if( (linkId = fr->getLinkIndex(std::string(nameA))) >= -1)
                {
                    robotA = fr;
                    linkIdA = linkId;
                    break;
                }
            }
        }   
        if(robotA == nullptr)
        {
            log.Print(MessageType::ERROR, "Invalid body name '%s' (glue '%s')!", nameA, glueName.c_str()); 
            return nullptr;
        }
    }

    if(entB == nullptr) //Maybe a robot link?
    {
        Robot* rob;
        unsigned int i = 0;
        while((rob = sm_->getRobot(i++)) != nullptr)
        {
            if(rob->getType() == RobotType::FEATHERSTONE)
            {
                FeatherstoneRobot* fr = (FeatherstoneRobot*)rob;
                int linkId = -2;
                if( (linkId = fr->getLinkIndex(std::string(nameB))) >= -1)
                {
                    robotB = fr;
                    linkIdB = linkId;
                    break;
                }
            }
        }   
        if(robotB == nullptr)
        {
            log.Print(MessageType::ERROR, "Invalid body name '%s' (glue '%s')!", nameB, glueName.c_str()); 
            return nullptr;
        }
    }

    std::unique_ptr<FixedJoint> fix {};

    if(entA != nullptr && entB != nullptr) //Glue two independent bodies
    {
        if(entA->getType() == EntityType::SOLID && entB->getType() == EntityType::SOLID)
            fix = std::make_unique<FixedJoint>(std::string(glueName), (SolidEntity*)entA, (SolidEntity*)entB); // Attach two dynamic bodies
        else if(entA->getType() == EntityType::SOLID && entB->getType() == EntityType::STATIC) 
            fix = std::make_unique<FixedJoint>(std::string(glueName), (SolidEntity*)entA); // Attach to world
        else if(entA->getType() == EntityType::STATIC && entB->getType() == EntityType::SOLID)
            fix = std::make_unique<FixedJoint>(std::string(glueName), (SolidEntity*)entB); // Attach to world
        else
        {
            log.Print(MessageType::ERROR, "Only two dynamic bodies or a static and dynamic body can be glued together (glue '%s')!", glueName.c_str()); 
            return nullptr;
        }
    }
    else if(robotA != nullptr && robotB != nullptr) //Glue together links of two robots
    {
        fix = std::make_unique<FixedJoint>(std::string(glueName), robotA->getDynamics(), robotB->getDynamics(), linkIdA, linkIdB);
    }
    else if(entA != nullptr && entA->getType() == EntityType::SOLID && robotB != nullptr) //Glue robot link to dynamic body
    {
        fix = std::make_unique<FixedJoint>(std::string(glueName), static_cast<SolidEntity*>(entA), robotB->getDynamics(), linkIdB);
    }
    else if(robotA != nullptr && entB != nullptr && entB->getType() == EntityType::SOLID) //Glue robot link to dynamic body
    {
        fix = std::make_unique<FixedJoint>(std::string(glueName), static_cast<SolidEntity*>(entB), robotA->getDynamics(), linkIdA);
    }
    else
    {
        log.Print(MessageType::ERROR, "Glue '%s' configuration not supported!", glueName.c_str());
        return nullptr;
    }
    
    if(fix != nullptr)
    {
        FixedJoint* fixPtr = fix.get();
        sm_->AddJoint(std::move(fix));
        if(!activated)
            fixPtr->RemoveFromSimulation(sm_);
        log.Print(MessageType::INFO, "Glue created between '%s' and '%s'.", nameA, nameB);
        return fixPtr;
    }
    else 
        return nullptr;
}

//Private
bool ScenarioParser::CopyNode(XMLNode* destParent, const XMLNode* src)
{
    //Should not happen, could maybe return false
    if(destParent == nullptr || src == nullptr)
        return true;

    //Get the document context where new memory will be allocated from
    tinyxml2::XMLDocument* doc = destParent->GetDocument();

    //Make the copy
    tinyxml2::XMLNode* srcCopy = src->ShallowClone(doc);
    if(srcCopy == nullptr)
    {
        log.Print(MessageType::ERROR, "Performing XML node copy failed!");
        return false;
    }

    //Add this child
    destParent->InsertEndChild(srcCopy);

    //Add the grandkids
    for(const tinyxml2::XMLNode* node = src->FirstChild(); node != nullptr; node = node->NextSibling())
        CopyNode(srcCopy, node);

    return true;
}

bool ScenarioParser::ParseVector(const char* components, Vector3& v)
{
    Scalar x, y, z;
    if(sscanf(components, "%lf %lf %lf", &x, &y, &z) != 3) 
        return false;
    v.setX(x);
    v.setY(y);
    v.setZ(z);
    return true;
}

bool ScenarioParser::ParseTransform(XMLElement* element, Transform& T)
{
    const char* trans = nullptr;
    const char* rot = nullptr;
    Vector3 xyz;
    Vector3 rpy;
    
    if(element->QueryStringAttribute("xyz", &trans) != XML_SUCCESS
       || element->QueryStringAttribute("rpy", &rot) != XML_SUCCESS
       || !ParseVector(trans, xyz)
       || !ParseVector(rot, rpy))
    {
        log.Print(MessageType::ERROR, "Unable to parse transform!");
        return false;
    }
        
    T = Transform(Quaternion(rpy.z(), rpy.y(), rpy.x()), xyz);
    return true;
}

bool ScenarioParser::ParseColor(XMLElement* element, Color& c)
{
    const char* components = nullptr;
    float c1, c2, c3;
    
    if(element->QueryStringAttribute("rgb", &components) == XML_SUCCESS && sscanf(components, "%f %f %f", &c1, &c2, &c3) == 3)
        c = Color::RGB(c1, c2, c3);
    else if(element->QueryStringAttribute("hsv", &components) == XML_SUCCESS && sscanf(components, "%f %f %f", &c1, &c2, &c3) == 3)
        c = Color::HSV(c1, c2, c3);
    else if(element->QueryAttribute("temperature", &c1) == XML_SUCCESS)
        c = Color::BlackBody(c1);
    else if(element->QueryAttribute("gray", &c1) == XML_SUCCESS)
        c = Color::Gray(c1);
    else
    {
        log.Print(MessageType::ERROR, "Unable to parse color definition!");
        return false;
    }
        
    return true;
}

bool ScenarioParser::ParseColorMap(XMLElement* element, ColorMap& cm)
{
    const char* colorMap = nullptr;

    if(element->QueryStringAttribute("colormap", &colorMap) == XML_SUCCESS)
    {
        std::string colorMapStr(colorMap);
        if(colorMapStr == "hot")
            cm = ColorMap::HOT;
        else if(colorMapStr == "jet")
            cm = ColorMap::JET;
        else if(colorMapStr == "perula")
            cm = ColorMap::PERULA;
        else if(colorMapStr == "greenblue")
            cm = ColorMap::GREEN_BLUE;
        else if(colorMapStr == "coldblue")
            cm = ColorMap::COLD_BLUE;
        else if(colorMapStr == "orangecopper")
            cm = ColorMap::ORANGE_COPPER;
        else if(colorMapStr == "grey")
            cm = ColorMap::GREY;
        else
        {
            log.Print(MessageType::ERROR, "Unknown color map name '%s'!", colorMapStr.c_str());
            return false;
        }
        return true;
    }
    else
    {
        log.Print(MessageType::ERROR, "Color map definition not found!");
        return false;
    }
}

bool ScenarioParser::ParseConstructInfo(XMLElement* element, ConstructInfo& info)
{
    std::function<bool(XMLElement*, const std::string&, ConstructInfoNode&)> parseNode = 
    [&](XMLElement* e, const std::string& name, ConstructInfoNode& node) 
    {
        XMLElement* item = e->FirstChildElement(name.c_str());
        if (item != nullptr)
        {
            // Parse all attributes
            for (auto& attribute : node.attributes)
            {
                switch(attribute.second.valueType)
                {
                    case ConstructInfoValueType::BOOL:
                    {
                        bool flag;
                        if (item->QueryBoolAttribute(attribute.first.c_str(), &flag) == XML_SUCCESS)
                        {
                            attribute.second.value = flag;
                            attribute.second.valid = true;
                        }
                        else if (!attribute.second.optional) // && !XML_SUCCESS
                        {
                            log.Print(MessageType::ERROR, "Required attribute '%s' of element '%s' not defined or wrong type!",
                                attribute.first.c_str(), name.c_str());
                            return false;
                        }
                    }
                        break;

                    case ConstructInfoValueType::INT:
                    {
                        int number;
                        if (item->QueryIntAttribute(attribute.first.c_str(), &number) == XML_SUCCESS)
                        {
                            attribute.second.value = number;
                            attribute.second.valid = true;
                        }
                        else if (!attribute.second.optional) // && !XML_SUCCESS
                        {
                            log.Print(MessageType::ERROR, "Required attribute '%s' of element '%s' not defined or wrong type!",
                                attribute.first.c_str(), name.c_str());
                            return false;
                        }
                    }
                        break;

                    case ConstructInfoValueType::SCALAR:
                    {
                        Scalar number;
                        if (item->QueryDoubleAttribute(attribute.first.c_str(), &number) == XML_SUCCESS)
                        {
                            attribute.second.value = number;
                            attribute.second.valid = true;
                        }
                        else if (!attribute.second.optional) // && !XML_SUCCESS
                        {
                            log.Print(MessageType::ERROR, "Required attribute '%s' of element '%s' not defined or wrong type!",
                                attribute.first.c_str(), name.c_str());
                            return false;
                        }
                    }
                        break;

                    case ConstructInfoValueType::VECTOR3:
                    {
                        Vector3 v;
                        const char* vstr = nullptr;

                        if (item->QueryStringAttribute(attribute.first.c_str(), &vstr) == XML_SUCCESS && ParseVector(vstr, v))
                        {
                            attribute.second.value = v;
                            attribute.second.valid = true;
                        }
                        else if (!attribute.second.optional) // && !XML_SUCCESS
                        {
                            log.Print(MessageType::ERROR, "Required attribute '%s' of element '%s' not defined or wrong type!",
                                attribute.first.c_str(), name.c_str());
                            return false;
                        }
                    }
                        break;

                    case ConstructInfoValueType::TRANSFORM:
                    {
                        Transform T;
                        
                        if (ParseTransform(item, T))
                        {
                            attribute.second.value = T;
                            attribute.second.valid = true;
                        }
                        else if (!attribute.second.optional) // && !XML_SUCCESS
                        {
                            log.Print(MessageType::ERROR, "Required transform '%s' not defined or wrong format!", name.c_str());
                            return false;
                        }
                    }
                        break;
                    
                    case ConstructInfoValueType::STRING:
                    {
                        const char* str = nullptr;
                        if (item->QueryStringAttribute(attribute.first.c_str(), &str) == XML_SUCCESS)
                        {
                            attribute.second.value = std::string(str);
                            attribute.second.valid = true;
                        }
                        else if (!attribute.second.optional) // && !XML_SUCCESS
                        {
                            log.Print(MessageType::ERROR, "Required attribute '%s' of element '%s' not defined or wrong type!",
                                attribute.first.c_str(), name.c_str());
                            return false;
                        }
                    }
                        break;

                    case ConstructInfoValueType::COLORMAP:
                    {
                        ColorMap cMap;

                        if (ParseColorMap(item, cMap))
                        {
                            attribute.second.value = cMap;
                            attribute.second.valid = true;
                        }
                        else if (!attribute.second.optional) // && !XML_SUCCESS
                        {
                            log.Print(MessageType::ERROR, "Required colormap '%s' not defined or wrong format!", name.c_str());
                            return false;
                        }
                    }
                        break;
                } 
            }
        
            // Parse child nodes
            for (auto& child : node.childNodes)
                parseNode(item, child.first, child.second);
        }
        else if (!node.optional) // && item == nullptr
        {
            log.Print(MessageType::ERROR, "Required element '%s' not defined!", name.c_str());
            return false;
        }
      
        return true;
    };

    for (auto& node : info.nodes)
    {
        parseNode(element, node.first, node.second);    
    }

    return true;
}

bool ScenarioParser::isGraphicalSim()
{
    return graphical_;
}

}
