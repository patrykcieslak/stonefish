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
//  Copyright (c) 2019-2021 Patryk Cieslak. All rights reserved.
//

#include "core/ScenarioParser.h"
#include "core/SimulationManager.h"
#include "core/NED.h"
#include "core/Robot.h"
#include "entities/statics/Obstacle.h"
#include "entities/statics/Plane.h"
#include "entities/statics/Terrain.h"
#include "entities/AnimatedEntity.h"
#include "entities/animation/ManualTrajectory.h"
#include "entities/animation/PWLTrajectory.h"
#include "entities/animation/CRTrajectory.h"
#include "entities/solids/Box.h"
#include "entities/solids/Cylinder.h"
#include "entities/solids/Sphere.h"
#include "entities/solids/Torus.h"
#include "entities/solids/Wing.h"
#include "entities/solids/Polyhedron.h"
#include "entities/solids/Compound.h"
#include "entities/forcefields/Uniform.h"
#include "entities/forcefields/Jet.h"
#include "sensors/scalar/Accelerometer.h"
#include "sensors/scalar/Gyroscope.h"
#include "sensors/scalar/IMU.h"
#include "sensors/scalar/DVL.h"
#include "sensors/scalar/GPS.h"
#include "sensors/scalar/Compass.h"
#include "sensors/scalar/Odometry.h"
#include "sensors/scalar/Pressure.h"
#include "sensors/scalar/RotaryEncoder.h"
#include "sensors/scalar/Torque.h"
#include "sensors/scalar/ForceTorque.h"
#include "sensors/scalar/Profiler.h"
#include "sensors/scalar/Multibeam.h"
#include "sensors/vision/ColorCamera.h"
#include "sensors/vision/DepthCamera.h"
#include "sensors/vision/Multibeam2.h"
#include "sensors/vision/FLS.h"
#include "sensors/vision/SSS.h"
#include "sensors/vision/MSIS.h"
#include "sensors/Contact.h"
#include "actuators/Light.h"
#include "actuators/Servo.h"
#include "actuators/Propeller.h"
#include "actuators/Rudder.h"
#include "actuators/Thruster.h"
#include "actuators/VariableBuoyancy.h"
#include "comms/AcousticModem.h"
#include "comms/USBLSimple.h"
#include "comms/USBLReal.h"
#include "graphics/OpenGLDataStructs.h"
#include "utils/SystemUtil.hpp"

namespace sf
{

ScenarioParser::ScenarioParser(SimulationManager* sm) : log(false), sm(sm)
{
    graphical = SimulationApp::getApp()->hasGraphics();
}

std::vector<ConsoleMessage> ScenarioParser::getLog()
{
    return log.getLines();
}

SimulationManager* ScenarioParser::getSimulationManager()
{
    return sm;
}

bool ScenarioParser::Parse(std::string filename)
{
    cInfo("Scenario parser: Loading scenario from '%s'.", filename.c_str());
    log.Print(MessageType::INFO, "Scenario file: %s", filename.c_str());
    
    //Open file
    if(doc.LoadFile(filename.c_str()) != XML_SUCCESS)
    {
        cInfo("Scenario parser: File not found!");
        log.Print(MessageType::ERROR, "File not found!");
        return false;
    }
    
    //Find root node
    XMLNode* root = doc.FirstChildElement("scenario");
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
    XMLElement* element = root->FirstChildElement("include");
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
        if(includedDoc.LoadFile(includedPath.c_str()) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Included file '%s' not found!", includedPath.c_str());
            return false;
        }
        
        root->DeleteChild(element); //Delete "include" element
        
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
            if(!CopyNode(root, child))
            {
                log.Print(MessageType::ERROR, "Could not copy included XML elements!");
                return false;
            }
        }
        element = root->FirstChildElement("include");
    }
    
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
        Light* l = ParseLight(element, "");
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
                delete l;
                return false;
            }
            l->AttachToWorld(origin);
            sm->AddActuator(l);
        }
        element = element->NextSiblingElement("light");
    }

    //Load standalone communication devices (beacons, optional)
    element = root->FirstChildElement("comm");
    while(element != nullptr)
    {
        Comm* comm = ParseComm(element, "");
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
                delete comm;
                return false;
            }
            comm->AttachToWorld(origin);
            sm->AddComm(comm);
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
    if(args.size() > 0)
        return ReplaceArguments(root, args);
    else
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

            //Replace ${arg ....} with the actual value
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
    sm->getNED()->Init(lat, lon, Scalar(0));
    
    //Setup ocean
    XMLElement* ocean = element->FirstChildElement("ocean");
    if(ocean != nullptr)
    {
        log.Print(MessageType::INFO, "Ocean simulation enabled.");
        //Basic setup
        Scalar wavesHeight(0);
        Scalar waterDensity(1000);
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
        }
        
        std::string waterName = sm->getMaterialManager()->CreateFluid("Water", waterDensity, 1.308e-3, 1.55); 
        sm->EnableOcean(wavesHeight, sm->getMaterialManager()->getFluid(waterName));
        sm->getOcean()->setWaterType(jerlov);
        
        //Currents
        if((item = ocean->FirstChildElement("current")) != nullptr)
        {
            Ocean* ocn = sm->getOcean();
            do
            {
                VelocityField* current = ParseVelocityField(item);
                if(current != nullptr)
                    ocn->AddVelocityField(current);
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
                sm->getAtmosphere()->SetupSunPosition(az, elev);
        }

        //Winds
        if((item = atmosphere->FirstChildElement("wind")) != nullptr)
        {
            Atmosphere* atm = sm->getAtmosphere();
            do
            {
                VelocityField* wind = ParseVelocityField(item);
                if(wind != nullptr)
                    atm->AddVelocityField(wind);
            }
            while((item = item->NextSiblingElement("wind")) != nullptr);
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
        sm->getMaterialManager()->CreateMaterial(materialName, density, restitution);
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
                if(!sm->getMaterialManager()->SetMaterialsInteraction(std::string(name1), std::string(name2), fstatic, fdynamic))
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
        const char* texture = nullptr;
        std::string textureStr = "";
        const char* normalMap = nullptr;
        std::string normalMapStr = "";
        
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
        
        sm->CreateLook(lookName, color, roughness, metalness, reflectivity, textureStr, normalMapStr);
        look = look->NextSiblingElement("look");
    }
    
    return true;
}

VelocityField* ScenarioParser::ParseVelocityField(XMLElement* element)
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
        return new Uniform(v);
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
        return new Jet(c, dir, radius, v.norm());
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
    StaticEntity* object;
        
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
        object = new Obstacle(objectName, dim, std::string(mat), std::string(look), uvMode);
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
        object = new Obstacle(objectName, radius, height, std::string(mat), std::string(look));
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
        object = new Obstacle(objectName, radius, std::string(mat), std::string(look));
    }
    else if(typestr == "model")
    {
        const char* phyMesh = nullptr;
        Scalar phyScale;
        Transform phyOrigin;
        
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
        if(item->QueryAttribute("scale", &phyScale) != XML_SUCCESS)
            phyScale = Scalar(1);
        if((item = item->NextSiblingElement("origin")) == nullptr || !ParseTransform(item, phyOrigin))
        {
            log.Print(MessageType::ERROR, "Physical mesh of static body '%s' not properly defined!", objectName.c_str());
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
                log.Print(MessageType::ERROR, "Visual mesh of static body '%s' not properly defined!", objectName.c_str());
                return false;
            }
            if(item->QueryAttribute("scale", &graScale) != XML_SUCCESS)
                graScale = Scalar(1);
            if((item = item->NextSiblingElement("origin")) == nullptr || !ParseTransform(item, graOrigin))
            {
                log.Print(MessageType::ERROR, "Visual mesh of static body '%s' not properly defined!", objectName.c_str());
                return false;
            }
            object = new Obstacle(objectName, GetFullPath(std::string(graMesh)), graScale, graOrigin, GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, std::string(mat), std::string(look));
        }
        else
        {
            object = new Obstacle(objectName, GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, std::string(mat), std::string(look));
        }
    }
    else if(typestr == "plane")
    {
        object = new Plane(objectName, Scalar(10000), std::string(mat), std::string(look), uvScale);
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
        object = new Terrain(objectName, GetFullPath(std::string(heightmap)), scaleX, scaleY, height, std::string(mat), std::string(look), uvScale);
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
        if(!ParseSensor(item, (Entity*)object))
        {
            log.Print(MessageType::ERROR, "Sensor of static body '%s' not properly defined!", objectName.c_str());
            delete object;
            return false;
        }
        item = item->NextSiblingElement("sensor");
    }

    //---- Lights ----
    item = element->FirstChildElement("light");
    while(item != nullptr)
    {
        Light* l = ParseLight(item, object->getName());
        if(l == nullptr)
        {
            log.Print(MessageType::ERROR, "Light of static body '%s' not properly defined!", objectName.c_str());
            delete object;
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                log.Print(MessageType::ERROR, "Light of static body '%s' not properly defined!", objectName.c_str());
                delete l;
                delete object;
                return false;
            }
            l->AttachToStatic(object, origin);
            sm->AddActuator(l);
        }
        item = item->NextSiblingElement("light");
    }
    
    //---- Communication devices ----
    item = element->FirstChildElement("comm");
    while(item != nullptr)
    {
        Comm* comm = ParseComm(item, object->getName());
        if(comm == nullptr)
        {
            log.Print(MessageType::ERROR, "Communication device of static body '%s' not properly defined!", objectName.c_str());
            delete object;
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                log.Print(MessageType::ERROR, "Communication device of static body '%s' not properly defined!", objectName.c_str());
                delete comm;
                delete object;
                return false;
            }
            comm->AttachToStatic(object, origin);
            sm->AddComm(comm);
        }
        item = item->NextSiblingElement("comm");
    }

    //---- Add to world ----
    sm->AddStaticEntity(object, trans);

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
    Trajectory* tr;
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
            tr = new ManualTrajectory();
            XMLElement* key = item->FirstChildElement("keypoint");
            Transform T;
            Scalar t;    
            if(key != nullptr && key->QueryAttribute("time", &t) == XML_SUCCESS
                && ParseTransform(key, T) && t == 0.0)
            {
                ((ManualTrajectory*)tr)->setTransform(T);
            }
        }
        else if(trTypeStr == "pwl" || trTypeStr == "spline")
        {
            tr = trTypeStr == "pwl" ? new PWLTrajectory(pm) : new CRTrajectory(pm);
            PWLTrajectory* pwl = (PWLTrajectory*)tr; //Spline has the same mechanism of adding points
            
            XMLElement* key = item->FirstChildElement("keypoint");
            while(key != nullptr)
            {
                Transform T;
                Scalar t;
                if(key->QueryAttribute("time", &t) != XML_SUCCESS || !ParseTransform(key, T))
                {
                    log.Print(MessageType::ERROR, "Trajectory keypoint not properly defined for animated body '%s'!", objectName.c_str());
                    delete tr;
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
    AnimatedEntity* object;
        
    if(typestr == "empty")
    {
        object = new AnimatedEntity(objectName, tr);
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
        object = new AnimatedEntity(objectName, tr, dim, origin, std::string(mat), std::string(look), collides);
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
        object = new AnimatedEntity(objectName, tr, radius, height, origin, std::string(mat), std::string(look), collides);
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
        object = new AnimatedEntity(objectName, tr, radius, origin, std::string(mat), std::string(look), collides);
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
            object = new AnimatedEntity(objectName, tr, GetFullPath(std::string(graMesh)), graScale, graOrigin, GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, std::string(mat), std::string(look), collides);
        }
        else
        {
            object = new AnimatedEntity(objectName, tr, GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, std::string(mat), std::string(look), collides);
        }
    }
    else
    {
        log.Print(MessageType::ERROR, "Incorrect animated body type!");
        delete tr;
        return false;
    }
        
    //---- Sensors ----
    item = element->FirstChildElement("sensor");
    while(item != nullptr)
    {
        if(!ParseSensor(item, (Entity*)object))
        {
            log.Print(MessageType::ERROR, "Sensor of animated body '%s' not properly defined!", objectName.c_str());
            delete object;
            return false;
        }
        item = item->NextSiblingElement("sensor");
    }

    //---- Lights ----
    item = element->FirstChildElement("light");
    while(item != nullptr)
    {
        Light* l = ParseLight(item, object->getName());
        if(l == nullptr)
        {
            log.Print(MessageType::ERROR, "Light of animated body '%s' not properly defined!", objectName.c_str());
            delete object;
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                log.Print(MessageType::ERROR, "Light of animated body '%s' not properly defined!", objectName.c_str());
                delete l;
                delete object;
                return false;
            }
            l->AttachToAnimated(object, origin);
            sm->AddActuator(l);
        }
        item = item->NextSiblingElement("light");
    }

    //---- Communication devices ----
    item = element->FirstChildElement("comm");
    while(item != nullptr)
    {
        Comm* comm = ParseComm(item, object->getName());
        if(comm == nullptr)
        {
            log.Print(MessageType::ERROR, "Communication device of animated body '%s' not properly defined!", objectName.c_str());
            delete object;
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                log.Print(MessageType::ERROR, "Communication device of animated body '%s' not properly defined!", objectName.c_str());
                delete comm;
                delete object;
                return false;
            }
            comm->AttachToSolid(object, origin);
            sm->AddComm(comm);
        }
        item = item->NextSiblingElement("comm");
    }

    //---- Add to world ----
    sm->AddAnimatedEntity(object);

    return true;
}

bool ScenarioParser::ParseDynamic(XMLElement* element)
{
    //---- Solid ----
    SolidEntity* solid;
    if(!ParseSolid(element, solid))
        return false;
    
    XMLElement* item;
    Transform trans;
    if((item = element->FirstChildElement("world_transform")) == nullptr || !ParseTransform(item, trans))
    {
        log.Print(MessageType::ERROR, "Initial pose of dynamic object '%s', in the world frame, missing!", solid->getName().c_str());
        delete solid;
        return false;
    }

    //---- Sensors -----
    item = element->FirstChildElement("sensor");
    while(item != nullptr)
    {
        if(!ParseSensor(item, (Entity*)solid))
        {
            log.Print(MessageType::ERROR, "Sensor of dynamic body '%s' not properly defined!", solid->getName().c_str());
            delete solid;
            return false;
        }
        item = item->NextSiblingElement("sensor");
    }

    //---- Lights ----
    item = element->FirstChildElement("light");
    while(item != nullptr)
    {
        Light* l = ParseLight(item, solid->getName());
        if(l == nullptr)
        {
            log.Print(MessageType::ERROR, "Light of dynamic body '%s' not properly defined!", solid->getName().c_str());
            delete solid;
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                log.Print(MessageType::ERROR, "Lght of dynamic body '%s' not properly defined!", solid->getName().c_str());
                delete l;
                delete solid;
                return false;
            }
            l->AttachToSolid(solid, origin);
            sm->AddActuator(l);
        }
        item = item->NextSiblingElement("light");
    }

    //---- Communication devices ----
    item = element->FirstChildElement("comm");
    while(item != nullptr)
    {
        Comm* comm = ParseComm(item, solid->getName());
        if(comm == nullptr)
        {
            log.Print(MessageType::ERROR, "Communication device of dynamic body '%s' not properly defined!", solid->getName().c_str());
            delete solid;
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                log.Print(MessageType::ERROR, "Communication device of dynamic body '%s' not properly defined!", solid->getName().c_str());
                delete comm;
                delete solid;
                return false;
            }
            comm->AttachToSolid(solid, origin);
            sm->AddComm(comm);
        }
        item = item->NextSiblingElement("comm");
    }

    //---- Add to world ----
    sm->AddSolidEntity(solid, trans);

    return true;
}

bool ScenarioParser::ParseSolid(XMLElement* element, SolidEntity*& solid, std::string ns, bool compoundPart)
{
    //---- Basic ----
    const char* name = nullptr;
    const char* type = nullptr;
    const char* phyType = nullptr;
    bool buoyant;
    BodyPhysicsType ePhyType;
    
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Rigid body name missing!");
        return false;
    }
    std::string solidName = ns != "" ? ns + "/" + std::string(name) : std::string(name);

    if(element->QueryStringAttribute("type", &type) != XML_SUCCESS)
    {
        log.Print(MessageType::ERROR, "Type of rigid body '%s' missing!", solidName.c_str());
        return false;
    }
    if(element->QueryStringAttribute("physics", &phyType) != XML_SUCCESS)
    {
        ePhyType = BodyPhysicsType::SUBMERGED;
    }
    else
    {
        std::string phyTypeStr(phyType);
        if(phyTypeStr == "surface")
            ePhyType = BodyPhysicsType::SURFACE;
        else if(phyTypeStr == "floating")
            ePhyType = BodyPhysicsType::FLOATING;
        else if(phyTypeStr == "submerged")
            ePhyType = BodyPhysicsType::SUBMERGED;
        else if(phyTypeStr == "aerodynamic")
            ePhyType = BodyPhysicsType::AERODYNAMIC;
        else 
        {
            log.Print(MessageType::ERROR, "Incorrect physics type for rigid body '%s'!", solidName.c_str());
            return false;
        }
    }
    if(element->QueryAttribute("buoyant", &buoyant) != XML_SUCCESS)
        buoyant = true;
    
    std::string typeStr(type);
    
    XMLElement* item;

    if(typeStr == "compound")
    {
        //First external part
        SolidEntity* part = nullptr;
        Compound* comp = nullptr;
        Transform partOrigin;
      
        if((item = element->FirstChildElement("external_part")) == nullptr
            || !ParseSolid(item, part, solidName, true))
        {
            log.Print(MessageType::ERROR, "No properly defined external part of compound rigid body '%s' found!", solidName.c_str());
            return false;
        }
        XMLElement* item2;
        if((item2 = item->FirstChildElement("compound_transform")) == nullptr || !ParseTransform(item2, partOrigin))
        {
            log.Print(MessageType::ERROR, "Incorrect definition of external part's '%s' origin frame, for rigid body '%s'!", part->getName().c_str(), solidName.c_str());
            return false;
        }
        comp = new Compound(solidName, part, partOrigin, ePhyType);
        
        //Iterate through all external parts
        item = item->NextSiblingElement("external_part");
        while(item != nullptr)
        {
            if(!ParseSolid(item, part, solidName, true))
            {
                log.Print(MessageType::ERROR, "Incorrect definition of external part of rigid body '%s'!", solidName.c_str());
                delete comp;
                return false;
            }
            if((item2 = item->FirstChildElement("compound_transform")) == nullptr || !ParseTransform(item2, partOrigin))
            {
                log.Print(MessageType::ERROR, "Incorrect definition of external part's '%s' origin frame, for rigid body '%s'!", part->getName().c_str(), solidName.c_str());
                delete part;
                delete comp;
                return false;
            }
                
            comp->AddExternalPart(part, partOrigin);
            item = item->NextSiblingElement("external_part");
        }
        
        //Iterate through all internal parts
        item = element->FirstChildElement("internal_part");
        while(item != nullptr)
        {
            if(!ParseSolid(item, part, solidName, true))
            {
                log.Print(MessageType::ERROR, "Incorrect definition of internal part of rigid body '%s'!", solidName.c_str());
                delete comp;
                return false;
            }
            if((item2 = item->FirstChildElement("compound_transform")) == nullptr || !ParseTransform(item2, partOrigin))
            {
                log.Print(MessageType::ERROR, "Incorrect definition of internal part's '%s' origin frame, for rigid body '%s'!", part->getName().c_str(), solidName.c_str());
                delete part;
                delete comp;
                return false;
            }
                
            comp->AddInternalPart(part, partOrigin);
            item = item->NextSiblingElement("internal_part");
        }
        
        solid = comp;
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
        bool cgok;
        
        //Material
        if((item = element->FirstChildElement("material")) == nullptr
            || item->QueryStringAttribute("name", &mat) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Material of rigid body '%s' not properly defined!", solidName.c_str());
            return false;
        }
        //Look
        if((item = element->FirstChildElement("look")) == nullptr
            || item->QueryStringAttribute("name", &look) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Look of rigid body '%s' not properly defined!", solidName.c_str());
            return false;
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
        
        //Origin    
        if(typeStr != "model")
        {
            if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            {
                log.Print(MessageType::ERROR, "Definition of origin frame of rigid body '%s' missing!", solidName.c_str());
                return false;
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
                return false;
            }    
            if(item->QueryAttribute("thickness", &thickness) != XML_SUCCESS)
                thickness = Scalar(-1);
            solid = new Box(solidName, dim, origin, std::string(mat), ePhyType, std::string(look), thickness, buoyant);
        }
        else if(typeStr == "cylinder")
        {
            Scalar radius, height, thickness;
            if((item = element->FirstChildElement("dimensions")) == nullptr
                || item->QueryAttribute("radius", &radius) != XML_SUCCESS
                || item->QueryAttribute("height", &height) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Dimensions of rigid body '%s' not properly defined!", solidName.c_str());
                return false;
            }
            if(item->QueryAttribute("thickness", &thickness) != XML_SUCCESS)
                thickness = Scalar(-1);
            solid = new Cylinder(solidName, radius, height, origin, std::string(mat), ePhyType, std::string(look), thickness, buoyant);
        }
        else if(typeStr == "sphere")
        {
            Scalar radius, thickness;
            if((item = element->FirstChildElement("dimensions")) == nullptr
                || item->QueryAttribute("radius", &radius) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Dimensions of rigid body '%s' not properly defined!", solidName.c_str());
                return false;
            }
            if(item->QueryAttribute("thickness", &thickness) != XML_SUCCESS)
                thickness = Scalar(-1);
            solid = new Sphere(solidName, radius, origin, std::string(mat), ePhyType, std::string(look), thickness, buoyant);
        }
        else if(typeStr == "torus")
        {
            Scalar radiusMaj, radiusMin, thickness;
            if((item = element->FirstChildElement("dimensions")) == nullptr
                || item->QueryAttribute("major_radius", &radiusMaj) != XML_SUCCESS
                || item->QueryAttribute("minor_radius", &radiusMin) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Dimensions of rigid body '%s' not properly defined!", solidName.c_str());
                return false;
            }
            if(item->QueryAttribute("thickness", &thickness) != XML_SUCCESS)
                thickness = Scalar(-1);
            solid = new Torus(solidName, radiusMaj, radiusMin, origin, std::string(mat), ePhyType, std::string(look), thickness, buoyant);
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
                return false;
            }
            if(item->QueryAttribute("thickness", &thickness) != XML_SUCCESS)
                thickness = Scalar(-1);

            std::string nacaStr(naca);
            if(nacaStr.size() != 4)
            {
                log.Print(MessageType::ERROR, "Incorrect NACA code for wind '%s'!", solidName.c_str());
                return false;
            }
            solid = new Wing(solidName, baseChord, tipChord, nacaStr, length, origin, std::string(mat), ePhyType, std::string(look), thickness, buoyant);
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
                return false;
            }
            XMLElement* item2;
            if((item2 = item->FirstChildElement("mesh")) == nullptr
                || item2->QueryStringAttribute("filename", &phyMesh) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Physical mesh of rigid body '%s' not properly defined!", solidName.c_str());
                return false;
            }
            item2->QueryAttribute("scale", &phyScale);
            if((item2 = item->FirstChildElement("thickness")) != nullptr)
                item2->QueryAttribute("value", &thickness);
            if((item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, phyOrigin))
            {
                log.Print(MessageType::ERROR, "Physical mesh of rigid body '%s' not properly defined!", solidName.c_str());
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
                    log.Print(MessageType::ERROR, "Visual mesh of rigid body '%s' not properly defined!", solidName.c_str());
                    return false;
                }
                item->QueryAttribute("scale", &graScale);
                if((item = item->NextSiblingElement("origin")) == nullptr || !ParseTransform(item, graOrigin))
                {
                    log.Print(MessageType::ERROR, "Visual mesh of rigid body '%s' not properly defined!", solidName.c_str());
                    return false;
                }          
                solid = new Polyhedron(solidName, GetFullPath(std::string(graMesh)), graScale, graOrigin, GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, std::string(mat), ePhyType, std::string(look), thickness, buoyant); 
            }
            else
            {
                solid = new Polyhedron(solidName, GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, std::string(mat), ePhyType, std::string(look), thickness, buoyant); 
            }
        }
        else
        {
            log.Print(MessageType::ERROR, "Unknown type of rigid body '%s'!", solidName.c_str());
            return false;
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
    return true;
}

bool ScenarioParser::ParseRobot(XMLElement* element)
{
    //---- Basic ----
    XMLElement* item;
    const char* name = nullptr;
    bool fixed;
    bool selfCollisions;
    Transform trans;
    
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
    if((item = element->FirstChildElement("world_transform")) == nullptr || !ParseTransform(item, trans))
    {
        log.Print(MessageType::ERROR, "Initial pose of robot '%s', in the world frame, missing!", robotName.c_str());
        return false;
    }

    Robot* robot = new Robot(robotName, fixed);

    //---- Links ----
    //Base link
    SolidEntity* baseLink = nullptr;
    
    if((item = element->FirstChildElement("base_link")) == nullptr)
    {
        log.Print(MessageType::ERROR, "Base link of robot '%s' missing!", robotName.c_str());
        delete robot;
        return false;
    }
    if(!ParseLink(item, robot, baseLink))
    {
        log.Print(MessageType::ERROR, "Base link of robot '%s' not properly defined!", robotName.c_str());
        delete robot;
        return false;
    }
    
    //Other links
    SolidEntity* link = nullptr;
    std::vector<SolidEntity*> links(0);
    
    item = element->FirstChildElement("link");
    while(item != nullptr)
    {
        if(!ParseLink(item, robot, link))
        {
            log.Print(MessageType::ERROR, "Link of robot '%s' not properly defined!", robotName.c_str());
            delete robot;
            return false;
        }        
        links.push_back(link);
        item = item->NextSiblingElement("link");
    }
    
    robot->DefineLinks(baseLink, links, selfCollisions);
    
    //---- Joints ----
    item = element->FirstChildElement("joint");
    while(item != nullptr)
    {
        if(!ParseJoint(item, robot))
        {
            log.Print(MessageType::ERROR, "Joint of robot '%s' not properly defined!", robotName.c_str());
            delete robot;
            return false;
        }
        item = item->NextSiblingElement("joint");
    }
    
    robot->BuildKinematicTree();
    
    //---- Sensors ----
    item = element->FirstChildElement("sensor");
    while(item != nullptr)
    {
        if(!ParseSensor(item, robot))
        {
            log.Print(MessageType::ERROR, "Sensor of robot '%s' not properly defined!", robotName.c_str());
            delete robot;
            return false;
        }
        item = item->NextSiblingElement("sensor");
    }
    
    //---- Actuators ----
    item = element->FirstChildElement("actuator");
    while(item != nullptr)
    {
        if(!ParseActuator(item, robot))
        {
            log.Print(MessageType::ERROR, "Actuator of robot '%s' not properly defined!", robotName.c_str());
            delete robot;
            return false;
        }
        item = item->NextSiblingElement("actuator");
    }

    //---- Lights ----
    item = element->FirstChildElement("light");
    while(item != nullptr)
    {
        Light* l = ParseLight(item, robot->getName());
        if(l == nullptr)
        {
            log.Print(MessageType::ERROR, "Light of robot '%s' not properly defined!", robotName.c_str());
            delete robot;
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
                delete l;
                delete robot;
                return false;
            }
            robot->AddLinkActuator(l, robotName + "/" + std::string(linkName), origin);
        }
        item = item->NextSiblingElement("light");
    }
    
    //---- Communication devices ----
    item = element->FirstChildElement("comm");
    while(item != nullptr)
    {
        Comm* comm = ParseComm(item, robot->getName());
        if(comm == nullptr)
        {
            log.Print(MessageType::ERROR, "Communication device of robot '%s' not properly defined!", robotName.c_str());
            delete robot;
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
                delete comm;
                delete robot;
                return false;
            }
            robot->AddComm(comm, robotName + "/" + std::string(linkName), origin);
        }
        item = item->NextSiblingElement("comm");
    }
    
    sm->AddRobot(robot, trans);
    return true;
}

bool ScenarioParser::ParseLink(XMLElement* element, Robot* robot, SolidEntity*& link)
{
    return ParseSolid(element, link, robot->getName());
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
    Actuator* act = ParseActuator(element, robot->getName());
    if(act == nullptr)
        return false;

    //Attach
    XMLElement* item;
    switch(act->getType())
    {
        //Joint actuators
        case ActuatorType::SERVO:
        {
            const char* jointName = nullptr;
            
            if((item = element->FirstChildElement("joint")) == nullptr
                || item->QueryStringAttribute("name", &jointName) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Joint definition for actuator '%s' missing!", act->getName().c_str());
                delete act;
                return false;
            }
            robot->AddJointActuator((JointActuator*)act, robot->getName() + "/" + std::string(jointName));
        }
            break;

        //Link actuators
        case ActuatorType::THRUSTER:
        case ActuatorType::PROPELLER:
        case ActuatorType::RUDDER:
        case ActuatorType::VBS:
        {
            const char* linkName = nullptr;
            Transform origin; 

            if((item = element->FirstChildElement("link")) == nullptr
                || item->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Link definition for actuator '%s' missing!", act->getName().c_str());
                delete act;
                return false;
            }
            if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            {
                log.Print(MessageType::ERROR, "Origin frame of actuator '%s' missing!", act->getName().c_str());
                delete act;
                return false;
            }
            robot->AddLinkActuator((LinkActuator*)act, robot->getName() + "/" + std::string(linkName), origin);
        }
            break;

        //Unsupported
        default:
        {
            log.Print(MessageType::ERROR, "Unsupported actuator type found in definition of robot '%s'!", robot->getName().c_str());
            delete act;
            return false;
        }
            break;
    }
    return true;
}

bool ScenarioParser::ParseSensor(XMLElement* element, Robot* robot)
{
    //Parse
    Sensor* sens = ParseSensor(element, robot->getName());
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
                delete sens;
                return false;
            }
            robot->AddJointSensor((JointSensor*)sens, robot->getName() + "/" + std::string(jointName));
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
                delete sens;
                return false;
            }
            if((item = element->FirstChildElement("origin")) == nullptr 
                || !ParseTransform(item, origin))
            {
                log.Print(MessageType::ERROR, "Origin frame of sensor '%s' missing!", sens->getName().c_str());
                delete sens;
                return false;
            }
            if(sens->getType() == SensorType::LINK)
                robot->AddLinkSensor((LinkSensor*)sens, robot->getName() + "/" + std::string(linkName), origin);
            else
                robot->AddVisionSensor((VisionSensor*)sens, robot->getName() + "/" + std::string(linkName), origin);
        }
            break;

        //Unsupported
        default:
        {
            log.Print(MessageType::ERROR, "Unsupported sensor type found in definition of robot '%s'!", robot->getName().c_str());
            delete sens;
            return false;
        }
            break;
    }
    return true;
}

bool ScenarioParser::ParseSensor(XMLElement* element, Entity* ent)
{
    //Parse
    Sensor* sens = ParseSensor(element, ent != nullptr ? ent->getName() : "");
    if(sens == nullptr)
        return false;

    //Attach
    XMLElement* item;
    switch(sens->getType())
    {
        case SensorType::JOINT:
        {
            log.Print(MessageType::ERROR, "Joint sensors can only be attached to robotic joints!");
            delete sens;
            return false;
        }
            break;

        case SensorType::LINK:
        {
            Transform origin;
            if((item = element->FirstChildElement("origin")) == nullptr 
                || !ParseTransform(item, origin))
            {
                delete sens;
                return false;
            }
            if(ent == nullptr)
            {
                log.Print(MessageType::ERROR, "Link sensors can only be attached to robotic links and moving bodies!");
                delete sens;
                return false;
            }
            else if(ent->getType() == EntityType::SOLID || ent->getType() == EntityType::ANIMATED)
            {
                LinkSensor* lsens = (LinkSensor*)sens;
                lsens->AttachToSolid((MovingEntity*)ent, origin);
                sm->AddSensor(lsens);
            }
            else
            {
                log.Print(MessageType::ERROR, "Link sensors can only be attached to robotic links and moving bodies!");
                delete sens;
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
                delete sens;
                return false;
            }
            VisionSensor* vsens = (VisionSensor*)sens;
            if(ent == nullptr)
                vsens->AttachToWorld(origin);
            else if(ent->getType() == EntityType::SOLID || ent->getType() == EntityType::ANIMATED)
                vsens->AttachToSolid((MovingEntity*)ent, origin);
            else if(ent->getType() == EntityType::STATIC)
                vsens->AttachToStatic((StaticEntity*)ent, origin);   
            else
            {
                log.Print(MessageType::ERROR, "Trying to attach vision sensor to a non-physical body!");
                delete sens;
                return false;
            }
            sm->AddSensor(vsens);
        }
            break;

        default:
        {
            delete sens;
            return false;
        }
            break;
    }
    return true;
}

Actuator* ScenarioParser::ParseActuator(XMLElement* element, const std::string& namePrefix)
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
    XMLElement* item;
    if(typeStr == "servo")
    {
        Scalar kp, kv, maxTau;
        if((item = element->FirstChildElement("controller")) == nullptr 
            || item->QueryAttribute("position_gain", &kp) != XML_SUCCESS 
            || item->QueryAttribute("velocity_gain", &kv) != XML_SUCCESS
            || item->QueryAttribute("max_torque", &maxTau) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Controller definition for actuator '%s' missing!", actuatorName.c_str());
            return nullptr;
        }    
        Servo* srv = new Servo(actuatorName, kp, kv, maxTau);
        return srv;
    }
    else if(typeStr == "thruster" || typeStr == "propeller")
    {
        const char* propFile = nullptr;
        const char* mat = nullptr;
        const char* look = nullptr;
        Scalar diameter, cThrust, cTorque, maxRpm, propScale, cThrustBack;
        bool rightHand;
        bool inverted = false;
        
        if((item = element->FirstChildElement("specs")) == nullptr 
            || item->QueryAttribute("thrust_coeff", &cThrust) != XML_SUCCESS 
            || item->QueryAttribute("torque_coeff", &cTorque) != XML_SUCCESS
            || item->QueryAttribute("max_rpm", &maxRpm) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Specs of actuator '%s' not properly defined!", actuatorName.c_str());
            return nullptr;
        }
        cThrustBack = cThrust;
        item->QueryAttribute("thrust_coeff_backward", &cThrustBack); //Optional
        item->QueryAttribute("inverted", &inverted); //Optional
        if((item = element->FirstChildElement("propeller")) == nullptr || item->QueryAttribute("diameter", &diameter) != XML_SUCCESS || item->QueryAttribute("right", &rightHand) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Propeller definition of actuator '%s' missing!", actuatorName.c_str());
            return nullptr;
        }
        XMLElement* item2;
        if((item2 = item->FirstChildElement("mesh")) == nullptr || item2->QueryStringAttribute("filename", &propFile) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Propeller mesh path of actuator '%s' missing!", actuatorName.c_str());
            return nullptr;
        }
        if(item2->QueryAttribute("scale", &propScale) != XML_SUCCESS)
            propScale = Scalar(1);
        if((item2 = item->FirstChildElement("material")) == nullptr || item2->QueryStringAttribute("name", &mat) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Propeller material of actuator '%s' missing!", actuatorName.c_str());
            return nullptr;
        }
        std::string lookStr = "";
        if((item2 = item->FirstChildElement("look")) != nullptr)
        {
            item2->QueryStringAttribute("name", &look);
            lookStr = std::string(look);
        }

        if(typeStr == "thruster")
        {
            Polyhedron* prop = new Polyhedron(actuatorName + "/Propeller", GetFullPath(std::string(propFile)), propScale, I4(), std::string(mat), BodyPhysicsType::SUBMERGED, lookStr);
            Thruster* th = new Thruster(actuatorName, prop, diameter, std::make_pair(cThrust, cThrustBack), cTorque, maxRpm, rightHand, inverted);
            return th;
        }
        else //propeller
        {
            Polyhedron* prop = new Polyhedron(actuatorName + "/Propeller", GetFullPath(std::string(propFile)), propScale, I4(), std::string(mat), BodyPhysicsType::AERODYNAMIC, lookStr);
            Propeller* p = new Propeller(actuatorName, prop, diameter, cThrust, cTorque, maxRpm, rightHand, inverted);
            return p;
        }
    }
    else if(typeStr == "rudder")
    {
        const char* rudderFile = nullptr;
        const char* mat = nullptr;
        const char* look = nullptr;

        Scalar area, dragCoeff, liftCoeff, maxAngle, rudderScale;
        bool inverted = false;
        
        if((item = element->FirstChildElement("specs")) == nullptr 
            || item->QueryAttribute("drag_coeff", &dragCoeff) != XML_SUCCESS
            || item->QueryAttribute("lift_coeff", &liftCoeff) != XML_SUCCESS
            || item->QueryAttribute("max_angle", &maxAngle) != XML_SUCCESS
            || item->QueryAttribute("area", &area) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Specs of actuator '%s' not properly defined!", actuatorName.c_str());
            return nullptr;
        }
        item->QueryAttribute("inverted", &inverted); //Optional

        if((item = element->FirstChildElement("visual")) == nullptr)
        {
            log.Print(MessageType::ERROR, "Visual definition of actuator '%s' missing!", actuatorName.c_str());
            return nullptr;
        }
        XMLElement* item2;
        if((item2 = item->FirstChildElement("mesh")) == nullptr || item2->QueryStringAttribute("filename", &rudderFile) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Visual mesh path of actuator '%s' missing!", actuatorName.c_str());
            return nullptr;
        }
        if(item2->QueryAttribute("scale", &rudderScale) != XML_SUCCESS)
            rudderScale = Scalar(1);
        if((item2 = item->FirstChildElement("material")) == nullptr || item2->QueryStringAttribute("name", &mat) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Visual material of actuator '%s' missing!", actuatorName.c_str());
            return nullptr;
        }
        std::string lookStr = "";
        if((item2 = item->FirstChildElement("look")) != nullptr)
        {
            item2->QueryStringAttribute("name", &look);
            lookStr = std::string(look);
        }

        Transform graOrigin = I4();
        if((item2 = item->FirstChildElement("origin")) != nullptr && !ParseTransform(item2, graOrigin))
        {
            log.Print(MessageType::ERROR, "Visual origin of actuator '%s' is not properly defined!", actuatorName.c_str());
            return nullptr;
        }

        Polyhedron* rudder = new Polyhedron(actuatorName + "/Rudder", GetFullPath(std::string(rudderFile)), rudderScale, graOrigin, std::string(mat), BodyPhysicsType::AERODYNAMIC, lookStr);
        Rudder* r = new Rudder(actuatorName, rudder, area, liftCoeff, dragCoeff, maxAngle, inverted);
        return r;
    }
    else if(typeStr == "vbs")
    {
        Scalar initialV;
        std::vector<std::string> vMeshes;
        if((item = element->FirstChildElement("volume")) == nullptr || item->QueryAttribute("initial", &initialV) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Volume of actuator '%s' not properly defined!", actuatorName.c_str());
            return nullptr;
        }
        XMLElement* item2;
        const char* meshFile;
        if((item2 = item->FirstChildElement("mesh")) == nullptr || item2->QueryStringAttribute("filename", &meshFile) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Volume mesh of actuator '%s' not properly defined!", actuatorName.c_str());
            return nullptr;
        }
        vMeshes.push_back(GetFullPath(std::string(meshFile)));
        while((item2 = item2->NextSiblingElement("mesh")) != nullptr)
        {
            const char* meshFile2;
            if(item2->QueryStringAttribute("filename", &meshFile2) != XML_SUCCESS)
            {
                log.Print(MessageType::ERROR, "Volume mesh of actuator '%s' not properly defined!", actuatorName.c_str());
                return nullptr;
            }
            vMeshes.push_back(GetFullPath(std::string(meshFile2)));
        }
        if(vMeshes.size() < 2)
        {
            log.Print(MessageType::ERROR, "Actuator '%s' requires definition of at least two volumes!", actuatorName.c_str());
            return nullptr;
        }
        VariableBuoyancy* vbs = new VariableBuoyancy(actuatorName, vMeshes, initialV);
        return vbs;
    }
    else
        return nullptr;
}

Sensor* ScenarioParser::ParseSensor(XMLElement* element, const std::string& namePrefix)
{
    //---- Common ----
    const char* name = nullptr;
    const char* type = nullptr;
    Scalar rate;
    
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
    XMLElement* item;
    if(typeStr == "accelerometer")
    {
        int history;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        Accelerometer* acc = new Accelerometer(sensorName, rate, history);
        
        //Optional range definition
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            const char* accel = nullptr;
            Vector3 laxyz;
            Scalar la;

            if(item->QueryStringAttribute("linear_acceleration", &accel) == XML_SUCCESS  
               && ParseVector(accel, laxyz))
            {
                acc->setRange(laxyz);    
            }
            else if(item->QueryAttribute("linear_acceleration", &la) == XML_SUCCESS)
            {
                acc->setRange(Vector3(la, la, la));
            }
            else
            {
                log.Print(MessageType::WARNING, "Range of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
            }     
        }
        //Optional noise definition
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            const char* accel = nullptr;
            Vector3 laxyz;
            Scalar la;

            if(item->QueryStringAttribute("linear_acceleration", &accel) == XML_SUCCESS  
               && ParseVector(accel, laxyz))
            {
                acc->setNoise(laxyz);
            }
            else if(item->QueryAttribute("linear_acceleration", &la) == XML_SUCCESS)
            {
                acc->setNoise(Vector3(la, la, la));   
            }
            else 
            {
                log.Print(MessageType::WARNING, "Noise of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
            }
        }
        return acc;
    }
    else if(typeStr == "gyro")
    {
        int history;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        Gyroscope* gyro = new Gyroscope(sensorName, rate, history);
        
        //Optional range definition
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            Scalar av;
            Vector3 avxyz;
            const char* velocity = nullptr;
            
            if(item->QueryStringAttribute("angular_velocity", &velocity) == XML_SUCCESS 
               && ParseVector(velocity, avxyz))
            {
                gyro->setRange(avxyz);
            }
            else if(item->QueryAttribute("angular_velocity", &av) == XML_SUCCESS)
            {
                gyro->setRange(Vector3(av, av, av));
            }
            else
            {
                log.Print(MessageType::WARNING, "Range of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
            }
        }
        //Optional noise definition
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar av;
            Vector3 avxyz = V0();
            const char* velocity = nullptr;
            Scalar b;
            Vector3 bxyz = V0();
            const char* bias = nullptr;
            int c = 0;

            if(item->QueryStringAttribute("angular_velocity", &velocity) == XML_SUCCESS && ParseVector(velocity, avxyz))
            {
                ++c;
            }
            else if(item->QueryAttribute("angular_velocity", &av) == XML_SUCCESS)
            {
                avxyz = Vector3(av, av, av);
                ++c;
            }
            
            if(item->QueryStringAttribute("bias", &bias) == XML_SUCCESS && ParseVector(bias, bxyz))
            {
                ++c;
            }
            else if(item->QueryAttribute("bias", &b) == XML_SUCCESS)
            {
                bxyz = Vector3(b, b, b);
                ++c;
            }

            if(c == 0)
                log.Print(MessageType::WARNING, "Noise of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
            else
                gyro->setNoise(avxyz, bxyz);
        }
        return gyro;
    }
    else if(typeStr == "imu")
    {
        int history;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        IMU* imu = new IMU(sensorName, rate, history);
        
        //Optional range definition
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            const char* velocity = nullptr;
            Vector3 avxyz = VMAX();
            Scalar av;
            const char* acc = nullptr;
            Vector3 laxyz = VMAX();
            Scalar la;
            int c = 0;
            
            if(item->QueryStringAttribute("angular_velocity", &velocity) == XML_SUCCESS  
               && ParseVector(velocity, avxyz))
            {
                ++c;
            }
            else if(item->QueryAttribute("angular_velocity", &av) == XML_SUCCESS)
            {
                avxyz = Vector3(av, av, av);
                ++c;
            }
            
            if(item->QueryStringAttribute("linear_acceleration", &acc) == XML_SUCCESS  
               && ParseVector(acc, laxyz))
            {
                ++c;
            }
            else if(item->QueryAttribute("linear_acceleration", &la) == XML_SUCCESS)
            {
                laxyz = Vector3(la, la, la);
                ++c;
            }
            
            if(c == 0)
                log.Print(MessageType::WARNING, "Range of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
            else
                imu->setRange(avxyz, laxyz);
        }
        //Optional noise definition
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            const char* angle = nullptr;
            const char* velocity = nullptr;
            const char* acc = nullptr;
            Vector3 axyz = V0();
            Vector3 avxyz = V0();
            Vector3 laxyz = V0();
            Scalar a;
            Scalar av;
            Scalar la;
            Scalar yawDrift = Scalar(0);
            int c = 0;

            if(item->QueryAttribute("yaw_drift", &yawDrift) == XML_SUCCESS)
                ++c;

            if(item->QueryStringAttribute("angle", &angle) == XML_SUCCESS 
               && ParseVector(angle, axyz))
            {
                ++c;
            }
            else if(item->QueryAttribute("angle", &a) == XML_SUCCESS)
            {
                axyz = Vector3(a, a, a);
                ++c;
            }

            if(item->QueryStringAttribute("angular_velocity", &velocity) == XML_SUCCESS 
               && ParseVector(velocity, avxyz))
            {
                ++c;
            }
            else if(item->QueryAttribute("angular_velocity", &av) == XML_SUCCESS)
            {
                avxyz = Vector3(av, av, av);
                ++c;
            }

            if(item->QueryStringAttribute("linear_acceleration", &acc) == XML_SUCCESS 
               && ParseVector(acc, laxyz))
            {
                ++c;
            }
            else if(item->QueryAttribute("linear_acceleration", &la) == XML_SUCCESS)
            {
                laxyz = Vector3(la, la, la);
                ++c;
            }
            
            if(c == 0)
                log.Print(MessageType::WARNING, "Noise of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
            else
                imu->setNoise(axyz, avxyz, yawDrift, laxyz);
        }
        return imu;
    }
    else if(typeStr == "dvl")
    {
        int history;
        Scalar beamAngle;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
        if((item = element->FirstChildElement("specs")) == nullptr || item->QueryAttribute("beam_angle", &beamAngle) != XML_SUCCESS)
            return nullptr;
            
        DVL* dvl = new DVL(sensorName, beamAngle, rate, history);

        //Optional range definition        
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            const char* velocity = nullptr;
            Vector3 vxyz = VMAX();
            Scalar v;
            Scalar altMin(0);
            Scalar altMax(BT_LARGE_FLOAT);
            int c = 0;
            
            if(item->QueryStringAttribute("velocity", &velocity) == XML_SUCCESS  
               && ParseVector(velocity, vxyz))
            {
                ++c;
            }
            else if(item->QueryAttribute("velocity", &v) == XML_SUCCESS)
            {
                vxyz = Vector3(v, v, v);
                ++c;
            }
            
            if(item->QueryAttribute("altitude_min", &altMin) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("altitude_max", &altMax) == XML_SUCCESS)
                ++c;
            
            if(c == 0)
                log.Print(MessageType::WARNING, "Range of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
            else
                dvl->setRange(vxyz, altMin, altMax);
        }
        //Optional water mass measurmeent definition
        if((item = element->FirstChildElement("water_layer")) != nullptr)
        {
            Scalar minSize(0);
            Scalar boundaryNear(0);
            Scalar boundaryFar(0);
            item->QueryAttribute("minimum_layer_size", &minSize);
            item->QueryAttribute("boundary_near", &boundaryNear);
            item->QueryAttribute("boundary_far", &boundaryFar);
            dvl->setWaterLayer(minSize, boundaryNear, boundaryFar);
        }

        //Optional range definition
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar v(0);
            Scalar vp(0);
            Scalar wv(0);
            Scalar wvp(0);
            Scalar altitude(0);
            int c = 0;

            if(item->QueryAttribute("velocity", &v) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("velocity_percent", &vp) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("altitude", &altitude) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("water_velocity", &wv) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("water_velocity_percent", &wvp) == XML_SUCCESS)
                ++c;
            
            if(c == 0)
                log.Print(MessageType::WARNING, "Noise of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
            else
                dvl->setNoise(vp, v, altitude, wvp, wv);
        }
        return dvl;
    }
    else if(typeStr == "gps")
    {
        int history;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        GPS* gps = new GPS(sensorName, rate, history);
        
        //Optional noise definition
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar ned;
            if(item->QueryAttribute("ned_position", &ned) == XML_SUCCESS)
                gps->setNoise(ned);
            else
                log.Print(MessageType::WARNING, "Noise of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
        }
        return gps;
    }
    else if(typeStr == "pressure")
    {
        int history;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        Pressure* press = new Pressure(sensorName, rate, history);
        
        //Optional range definition
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            Scalar pressure;
            if(item->QueryAttribute("pressure", &pressure) == XML_SUCCESS)
                press->setRange(pressure);
            else
                log.Print(MessageType::WARNING, "Range of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
        }
        //Optional noise definition
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar pressure;
            if(item->QueryAttribute("pressure", &pressure) == XML_SUCCESS)
                press->setNoise(pressure);
            else
                log.Print(MessageType::WARNING, "Noise of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
        }
        return press;
    }
    else if(typeStr == "odometry")
    {
        int history;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        Odometry* odom = new Odometry(sensorName, rate, history);
        
        //Optional noise definition
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar p(0);
            Scalar v(0);
            Scalar angle(0);
            Scalar av(0);
            int c = 0;
        
            if(item->QueryAttribute("position", &p) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("velocity", &v) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("angle", &angle) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("angular_velocity", &av) == XML_SUCCESS)
                ++c;

            if(c == 0)
                log.Print(MessageType::WARNING, "Noise of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
            else
                odom->setNoise(p, v, angle, av);
        }
        return odom;
    }
    else if(typeStr == "compass")
    {
        int history;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        Compass* compass = new Compass(sensorName, rate, history);
        
        //Optional noise definition
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar heading;
            if(item->QueryAttribute("heading", &heading) == XML_SUCCESS)
                compass->setNoise(heading);
            else
                log.Print(MessageType::WARNING, "Noise of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
        }
        return compass;
    }
    else if(typeStr == "profiler")
    {
        int history;
        Scalar fov;
        int steps;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
        if((item = element->FirstChildElement("specs")) == nullptr || item->QueryAttribute("fov", &fov) != XML_SUCCESS || item->QueryAttribute("steps", &steps) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Specs of sensor '%s' not properly defined!", sensorName.c_str());
            return nullptr;
        }
            
        Profiler* prof = new Profiler(sensorName, fov, steps, rate, history);
        
        //Optional range definition
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            Scalar distMin(0);
            Scalar distMax(BT_LARGE_FLOAT);
            int c = 0;
            
            if(item->QueryAttribute("distance_min", &distMin) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("distance_max", &distMax) == XML_SUCCESS)
                ++c;
            
            if(c == 0)
                log.Print(MessageType::WARNING, "Range of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
            else
                prof->setRange(distMin, distMax);
        }
        //Optional noise definition
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar distance;
            if(item->QueryAttribute("distance", &distance) == XML_SUCCESS)
                prof->setNoise(distance);
            else
                log.Print(MessageType::WARNING, "Noise of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
        }
        return prof;
    }
    else if(typeStr == "multibeam" || typeStr == "multibeam1d")
    {
        int history;
        Scalar fov;
        int steps;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
        if((item = element->FirstChildElement("specs")) == nullptr || item->QueryAttribute("fov", &fov) != XML_SUCCESS || item->QueryAttribute("steps", &steps) != XML_SUCCESS)
            return nullptr;
            
        Multibeam* mult = new Multibeam(sensorName, fov, steps, rate, history);
        
        //Optional range definition
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            Scalar distMin(0);
            Scalar distMax(BT_LARGE_FLOAT);
            int c = 0;

            if(item->QueryAttribute("distance_min", &distMin) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("distance_max", &distMax) == XML_SUCCESS)
                ++c;
            
            if(c == 0)
                log.Print(MessageType::WARNING, "Range of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
            else
                mult->setRange(distMin, distMax);
        }
        //Optional noise definition
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar distance;
            if(item->QueryAttribute("distance", &distance) == XML_SUCCESS)
                mult->setNoise(distance);
            else
                log.Print(MessageType::WARNING, "Noise of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
        }
        return mult;
    }
    else if(typeStr == "torque")
    {
        int history;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        Torque* torque = new Torque(sensorName, rate, history);
        
        //Optional range definition
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            Scalar tau;
            if(item->QueryAttribute("torque", &tau) == XML_SUCCESS)
                torque->setRange(tau);
            else
                log.Print(MessageType::WARNING, "Range of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
        }
        //Optional noise definition
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar tau;
            if(item->QueryAttribute("torque", &tau) == XML_SUCCESS)
                torque->setNoise(tau);
            else
                log.Print(MessageType::WARNING, "Noise of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
        }
        return torque;
    }
    else if(typeStr == "forcetorque")
    {
        Transform origin;
        int history;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return nullptr;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        ForceTorque* ft = new ForceTorque(sensorName, origin, rate, history);
        
        //Optional range definition
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            const char* force = nullptr;
            const char* torque = nullptr;
            Vector3 f = VMAX();
            Vector3 t = VMAX();
            int c = 0;

            if(item->QueryStringAttribute("force", &force) == XML_SUCCESS && ParseVector(force, f))
                ++c;
            if(item->QueryStringAttribute("torque", &torque) != XML_SUCCESS && ParseVector(torque, t))
                ++c;
            if(c == 0)
                log.Print(MessageType::WARNING, "Range of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
            else
                ft->setRange(f, t);
        }
        //Optional noise definition
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar f(0);
            Scalar t(0);
            int c = 0;
            if(item->QueryAttribute("force", &f) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("torque", &t) == XML_SUCCESS)
                ++c;
            if(c == 0)
                log.Print(MessageType::WARNING, "Noise of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
            else
                ft->setNoise(f, t);
        }
        return ft;
    }
    else if(typeStr == "encoder")
    {
        int history;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        RotaryEncoder* enc = new RotaryEncoder(sensorName, rate, history);
        return enc;
    }
    else if(typeStr == "camera")
    {
        if(!isGraphicalSim())
        {
            log.Print(MessageType::ERROR, "Cameras not supported in console mode!");
            return nullptr;
        }

        int resX, resY;
        Scalar hFov;
        if((item = element->FirstChildElement("specs")) == nullptr 
            || item->QueryAttribute("resolution_x", &resX) != XML_SUCCESS 
            || item->QueryAttribute("resolution_y", &resY) != XML_SUCCESS
            || item->QueryAttribute("horizontal_fov", &hFov) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Specs of sensor '%s' not properly defined!", sensorName.c_str());
            return nullptr;
        }

        ColorCamera* cam;
        
        //Optional parameters
        if((item = element->FirstChildElement("rendering")) != nullptr) 
        {
            Scalar minDist(0.02);
            Scalar maxDist(100000.0);
            int c = 0;

            if(item->QueryAttribute("minimum_distance", &minDist) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("maximum_distance", &maxDist) == XML_SUCCESS)
                ++c;

            if(c == 0)
            {
                log.Print(MessageType::WARNING, "Rendering options of camera '%s' not properly defined - using defaults.", sensorName.c_str());
                cam = new ColorCamera(sensorName, resX, resY, hFov, rate);
            }
            else
                cam = new ColorCamera(sensorName, resX, resY, hFov, rate, minDist, maxDist);
        }
        else
            cam = new ColorCamera(sensorName, resX, resY, hFov, rate);
        return cam;
    }
    else if(typeStr == "depthcamera")
    {
        if(!isGraphicalSim())
        {
            log.Print(MessageType::ERROR, "Depth cameras not supported in console mode!");
            return nullptr;
        }

        int resX, resY;
        Scalar hFov;
        Scalar depthMin, depthMax;
        if((item = element->FirstChildElement("specs")) == nullptr 
            || item->QueryAttribute("resolution_x", &resX) != XML_SUCCESS 
            || item->QueryAttribute("resolution_y", &resY) != XML_SUCCESS
            || item->QueryAttribute("horizontal_fov", &hFov) != XML_SUCCESS
            || item->QueryAttribute("depth_min", &depthMin) != XML_SUCCESS
            || item->QueryAttribute("depth_max", &depthMax) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Specs of sensor '%s' not properly defined!", sensorName.c_str());
            return nullptr;
        }
        
        DepthCamera* dcam = new DepthCamera(sensorName, resX, resY, hFov, depthMin, depthMax, rate);

        //Optional noise definition
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            float depth;
            if(item->QueryAttribute("depth", &depth) == XML_SUCCESS)
                dcam->setNoise(depth);
            else
                log.Print(MessageType::WARNING, "Noise of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
        }
        return dcam;
    }
    else if(typeStr == "multibeam2d")
    {
        if(!isGraphicalSim())
        {
            log.Print(MessageType::ERROR, "Multibeam 2D not supported in console mode!");
            return nullptr;
        }

        int resX, resY;
        Scalar hFov, vFov;
        Scalar rangeMin, rangeMax;
        if((item = element->FirstChildElement("specs")) == nullptr 
            || item->QueryAttribute("resolution_x", &resX) != XML_SUCCESS 
            || item->QueryAttribute("resolution_y", &resY) != XML_SUCCESS
            || item->QueryAttribute("horizontal_fov", &hFov) != XML_SUCCESS
            || item->QueryAttribute("vertical_fov", &vFov) != XML_SUCCESS
            || item->QueryAttribute("range_min", &rangeMin) != XML_SUCCESS
            || item->QueryAttribute("range_max", &rangeMax) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Specs of sensor '%s' not properly defined!", sensorName.c_str());
            return nullptr;
        }

        Multibeam2* mb = new Multibeam2(sensorName, resX, resY, hFov, vFov, rangeMin, rangeMax, rate);
        return mb;
    }
    else if(typeStr == "fls")
    {
        if(!isGraphicalSim())
        {
            log.Print(MessageType::ERROR, "FLS not supported in console mode!");
            return nullptr;
        }

        Scalar hFov, vFov;
        int nBeams, nBins;
        Scalar rangeMin(0.5);
        Scalar rangeMax(10.0);
        Scalar gain(1.0);
        ColorMap cMap = ColorMap::GREEN_BLUE;
        
        if((item = element->FirstChildElement("specs")) == nullptr 
            || item->QueryAttribute("beams", &nBeams) != XML_SUCCESS 
            || item->QueryAttribute("bins", &nBins) != XML_SUCCESS
            || item->QueryAttribute("horizontal_fov", &hFov) != XML_SUCCESS
            || item->QueryAttribute("vertical_fov", &vFov) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Specs of sensor '%s' not properly defined!", sensorName.c_str());
            return nullptr;
        }

        //Optional settings
        if((item = element->FirstChildElement("settings")) != nullptr)
        {
            int c = 0;
            if(item->QueryAttribute("range_min", &rangeMin) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("range_max", &rangeMax) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("gain", &gain) == XML_SUCCESS)
                ++c;
            if(c == 0)
                log.Print(MessageType::WARNING, "Settings of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
        }
        if((item = element->FirstChildElement("display")) != nullptr)
            ParseColorMap(item, cMap);
        
        FLS* fls = new FLS(sensorName, nBeams, nBins, hFov, vFov, rangeMin, rangeMax, cMap, rate);
        fls->setGain(gain);

        //Optional noise definition
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            float mul = 0.025f;
            float add = 0.035f;
            int c = 0;
            if(item->QueryAttribute("multiplicative", &mul) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("additive", &add) == XML_SUCCESS)
                ++c;
            if(c == 0)
                log.Print(MessageType::WARNING, "Noise of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
            fls->setNoise(mul, add);
        }
        else
        {
            fls->setNoise(0.025f, 0.035f); //Default values that look realistic
            log.Print(MessageType::WARNING, "Noise of sensor '%s' not defined - using defaults.", sensorName.c_str());
        }
        return fls;
    }
    else if(typeStr == "sss")
    {
        if(!isGraphicalSim())
        {
            log.Print(MessageType::ERROR, "SSS not supported in console mode!");
            return nullptr;
        }

        Scalar hFov, vFov;
        int nLines, nBins;
        Scalar tilt;
        Scalar rangeMin(0.5);
        Scalar rangeMax(10.0);
        Scalar gain(1.0);
        ColorMap cMap = ColorMap::GREEN_BLUE;

        if((item = element->FirstChildElement("specs")) == nullptr 
            || item->QueryAttribute("bins", &nBins) != XML_SUCCESS
            || item->QueryAttribute("lines", &nLines) != XML_SUCCESS
            || item->QueryAttribute("horizontal_beam_width", &hFov) != XML_SUCCESS
            || item->QueryAttribute("vertical_beam_width", &vFov) != XML_SUCCESS
            || item->QueryAttribute("vertical_tilt", &tilt) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Specs of sensor '%s' not properly defined!", sensorName.c_str());
            return nullptr;
        }

        //Optional settings
        if((item = element->FirstChildElement("settings")) != nullptr)
        {
            int c = 0;
            if(item->QueryAttribute("range_min", &rangeMin) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("range_max", &rangeMax) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("gain", &gain) == XML_SUCCESS)
                ++c;
            if(c == 0)
                log.Print(MessageType::WARNING, "Settings of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
        }
        if((item = element->FirstChildElement("display")) != nullptr)
            ParseColorMap(item, cMap);
        
        SSS* sss = new SSS(sensorName, nBins, nLines, vFov, hFov, tilt, rangeMin, rangeMax, cMap, rate);
        sss->setGain(gain);

        //Optional noise definition
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            float mul = 0.01f;
            float add = 0.02f;
            int c = 0;
            if(item->QueryAttribute("multiplicative", &mul) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("additive", &add) == XML_SUCCESS)
                ++c;
            if(c == 0)
                log.Print(MessageType::WARNING, "Noise of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
            sss->setNoise(mul, add);
        }
        else
        {
            sss->setNoise(0.01f, 0.02f); //Default values that look realistic
            log.Print(MessageType::WARNING, "Noise of sensor '%s' not defined - using defaults.", sensorName.c_str());
        }
        return sss;
    }
    else if(typeStr == "msis")
    {
        if(!isGraphicalSim())
        {
            log.Print(MessageType::ERROR, "MSIS not supported in console mode!");
            return nullptr;
        }

        Scalar stepAngle;
        int nBins;
        Scalar hFov, vFov;
        Scalar rotMin(-180);
        Scalar rotMax(180);
        Scalar rangeMin(0.5);
        Scalar rangeMax(10.0);
        Scalar gain(1.0);
        ColorMap cMap = ColorMap::GREEN_BLUE;
        
        if((item = element->FirstChildElement("specs")) == nullptr 
            || item->QueryAttribute("step", &stepAngle) != XML_SUCCESS
            || item->QueryAttribute("bins", &nBins) != XML_SUCCESS
            || item->QueryAttribute("horizontal_beam_width", &hFov) != XML_SUCCESS
            || item->QueryAttribute("vertical_beam_width", &vFov) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Specs of sensor '%s' not properly defined!", sensorName.c_str());
            return nullptr;
        }

        //Optional settings
        if((item = element->FirstChildElement("settings")) != nullptr)
        {
            int c = 0;
            if(item->QueryAttribute("range_min", &rangeMin) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("range_max", &rangeMax) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("rotation_min", &rotMin) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("rotation_max", &rotMax) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("gain", &gain) == XML_SUCCESS)
                ++c;
            if(c == 0)
                log.Print(MessageType::WARNING, "Settings of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
        }
        if((item = element->FirstChildElement("display")) != nullptr)
            ParseColorMap(item, cMap);
        
        MSIS* msis = new MSIS(sensorName, stepAngle, nBins, hFov, vFov, rotMin, rotMax, rangeMin, rangeMax, cMap, rate);
        msis->setGain(gain);

        //Optional noise definition
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            float mul = 0.02f;
            float add = 0.04f;
            int c = 0;
            if(item->QueryAttribute("multiplicative", &mul) == XML_SUCCESS)
                ++c;
            if(item->QueryAttribute("additive", &add) == XML_SUCCESS)
                ++c;
            if(c == 0)
                log.Print(MessageType::WARNING, "Noise of sensor '%s' not properly defined - using defaults.", sensorName.c_str());
            msis->setNoise(mul, add);
        }
        else
        {
            msis->setNoise(0.02f, 0.04f); //Default values that look realistic
            log.Print(MessageType::WARNING, "Noise of sensor '%s' not defined - using defaults.", sensorName.c_str());
        }
        return msis;
    }
    else
        return nullptr;
}

Light* ScenarioParser::ParseLight(XMLElement* element, const std::string& namePrefix)
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
        
    Light* light;
    if(cone > Scalar(0))
        light = new Light(lightName, radius, cone, color, illu);
    else
        light = new Light(lightName, radius, color, illu);
    return light;
}

Comm* ScenarioParser::ParseComm(XMLElement* element, const std::string& namePrefix)
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
    Comm* comm;
    if(typeStr == "acoustic_modem")
    {
        Scalar hFovDeg;
        Scalar vFovDeg;
        Scalar range;
        unsigned int cId = 0;
        bool occlusion = true;
        
        if((item = element->FirstChildElement("specs")) == nullptr
            || item->QueryAttribute("horizontal_fov", &hFovDeg) != XML_SUCCESS
            || item->QueryAttribute("vertical_fov", &vFovDeg) != XML_SUCCESS
            || item->QueryAttribute("range", &range) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Specs of communication device '%s' not properly defined!", commName.c_str());
            return nullptr;
        }
        item = element->FirstChildElement("connect");
        if(item == nullptr || item->QueryAttribute("device_id", &cId) != XML_SUCCESS || cId == 0)
        {
            log.Print(MessageType::ERROR, "Communication device '%s' not connected!", commName.c_str());
            return nullptr;
        }
        item->QueryAttribute("occlusion_test", &occlusion);
    
        comm = new AcousticModem(commName, devId, hFovDeg, vFovDeg, range);
        comm->Connect(cId);
        ((AcousticModem*)comm)->setOcclusionTest(occlusion);
        return comm;
    }
    else if(typeStr == "usbl")
    {
        Scalar hFovDeg;
        Scalar vFovDeg;
        Scalar range;
        unsigned int cId = 0;
        Scalar pingRate;
        bool occlusion = true;
        
        if((item = element->FirstChildElement("specs")) == nullptr
            || item->QueryAttribute("horizontal_fov", &hFovDeg) != XML_SUCCESS
            || item->QueryAttribute("vertical_fov", &vFovDeg) != XML_SUCCESS
            || item->QueryAttribute("range", &range) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Specs of communication device '%s' not properly defined!", commName.c_str());
            return nullptr;
        }
        item = element->FirstChildElement("connect");
        if(item == nullptr || item->QueryAttribute("device_id", &cId) != XML_SUCCESS || cId == 0)
        {
            log.Print(MessageType::ERROR, "Communication device '%s' not connected!", commName.c_str());
            return nullptr;
        }
        item->QueryAttribute("occlusion_test", &occlusion);

        comm = new USBLSimple(commName, devId, hFovDeg, vFovDeg, range);
        comm->Connect(cId);
        ((AcousticModem*)comm)->setOcclusionTest(occlusion);
        
        if((item = element->FirstChildElement("autoping")) != nullptr
            && item->QueryAttribute("rate", &pingRate) == XML_SUCCESS)
            ((USBL*)comm)->EnableAutoPing(pingRate);
        
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
                ((USBLSimple*)comm)->setNoise(rangeDev, hAngleDevDeg, vAngleDevDeg);
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
                ((USBLSimple*)comm)->setResolution(rangeRes, angleResDeg);
        }
        return comm;
    }
    else if(typeStr == "usbl2")
    {
        Scalar hFovDeg;
        Scalar vFovDeg;
        Scalar range;
        Scalar freq;
        Scalar baseline;
        unsigned int cId = 0;
        Scalar pingRate;
        bool occlusion = true;
        
        if((item = element->FirstChildElement("specs")) == nullptr
            || item->QueryAttribute("horizontal_fov", &hFovDeg) != XML_SUCCESS
            || item->QueryAttribute("vertical_fov", &vFovDeg) != XML_SUCCESS
            || item->QueryAttribute("range", &range) != XML_SUCCESS
            || item->QueryAttribute("frequency", &freq) != XML_SUCCESS
            || item->QueryAttribute("baseline", &baseline) != XML_SUCCESS)
        {
            log.Print(MessageType::ERROR, "Specs of communication device '%s' not properly defined!", commName.c_str());
            return nullptr;
        }
        item = element->FirstChildElement("connect");
        if(item == nullptr || item->QueryAttribute("device_id", &cId) != XML_SUCCESS || cId == 0)
        {
            log.Print(MessageType::ERROR, "Communication device '%s' not connected!", commName.c_str());
            return nullptr;
        }
        item->QueryAttribute("occlusion_test", &occlusion);

        comm = new USBLReal(commName, devId, hFovDeg, vFovDeg, range, freq, baseline);
        comm->Connect(cId);
        ((AcousticModem*)comm)->setOcclusionTest(occlusion);
        
        if((item = element->FirstChildElement("autoping")) != nullptr
            && item->QueryAttribute("rate", &pingRate) == XML_SUCCESS)
            ((USBL*)comm)->EnableAutoPing(pingRate);
        
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
                ((USBLReal*)comm)->setNoise(timeDev, svDev, phaseDev, blError, depthDev);
        }
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
    if((itemA = element->FirstChildElement("bodyA")) == nullptr
        || (itemB = element->FirstChildElement("bodyB")) == nullptr)
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
    entA = sm->getEntity(std::string(nameA));
    entB = sm->getEntity(std::string(nameB));
    if(entA == nullptr)
    {
        Robot* rob;
        unsigned int i = 0;
        while((rob = sm->getRobot(i++)) != nullptr)
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
        while((rob = sm->getRobot(i++)) != nullptr)
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
    
    Contact* cnt = new Contact(contactName, entA, entB, history);
    cnt->setDisplayMask(displayMask);
    sm->AddContact(cnt);
    
    return true;
}

std::string ScenarioParser::GetFullPath(const std::string& path)
{
    if(path.at(0) == '/' || path.at(0) == '~') //Absolute path?
        return path;
    else
        return GetDataPath() + path;
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

bool ScenarioParser::isGraphicalSim()
{
    return graphical;
}

}
