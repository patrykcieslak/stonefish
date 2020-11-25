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
//  Copyright (c) 2019-2020 Patryk Cieslak. All rights reserved.
//

#include "core/ScenarioParser.h"
#include "core/Console.h"
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
#include "actuators/Thruster.h"
#include "actuators/VariableBuoyancy.h"
#include "comms/AcousticModem.h"
#include "comms/USBL.h"
#include "graphics/OpenGLDataStructs.h"
#include "utils/SystemUtil.hpp"

namespace sf
{

ScenarioParser::ScenarioParser(SimulationManager* sm) : sm(sm) 
{
}

SimulationManager* ScenarioParser::getSimulationManager()
{
    return sm;
}

bool ScenarioParser::Parse(std::string filename)
{
    cInfo("Loading scenario from: %s", filename.c_str());
    
    //Open file
    if(doc.LoadFile(filename.c_str()) != XML_SUCCESS)
    {
        cError("Scenario parser: file not found!");
        return false;
    }
    
    //Find root node
    XMLNode* root = doc.FirstChildElement("scenario");
    if(root == nullptr)
    {
        cError("Scenario parser: root node not found!");
        return false;
    }
    
    if(!PreProcess(root))
    {
        cError("Scenario parser: pre-processing failed!");
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
            cError("Scenario parser: include not properly defined!");
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
				cError("Scenario parser: Include file argument not properly defined!");
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
            cError("Scenario parser: included file '%s' not found!", includedPath.c_str());
            return false;
        }
        
        root->DeleteChild(element); //Delete "include" element
        
        XMLNode* includedRoot = includedDoc.FirstChildElement("scenario");
        if(includedRoot == nullptr)
        {
            cError("Scenario parser: root node not found in included file '%s'!", includedPath.c_str());
            return false;
        }
        
        if(!PreProcess(includedRoot, args))
        {
            cError("Scenario parser: pre-processing of included file '%s' failed!", includedPath.c_str());
            return false;
        }
        
        for(const XMLNode* child = includedRoot->FirstChild(); child != nullptr; child = child->NextSibling())
        {
            if(!CopyNode(root, child))
            {
                cError("Scenario parser: could not copy included xml elements!");
                return false;
            }
        }
        element = root->FirstChildElement("include");
    }
    
    //Load environment settings
    element = root->FirstChildElement("environment");
    if(element == nullptr)
    {
        cError("Scenario parser: environment settings not defined!");
        return false;
    }
    if(!ParseEnvironment(element))
    {
        cError("Scenario parser: environment settings not properly defined!");
        return false;
    }
        
    //Load materials
    element = root->FirstChildElement("materials");
    if(element == nullptr)
    {
        cError("Scenario parser: materials not defined!");
        return false;
    }
    if(!ParseMaterials(element))
    {
        cError("Scenario parser: materials not properly defined!");
        return false;
    }
    
    //Load looks (optional)
    element = root->FirstChildElement("looks");
    if(element == nullptr)
        cWarning("Scenario parser: looks not defined -> using standard look.");

    while(element != nullptr)
    {
        if(!ParseLooks(element))
        {
            cError("Scenario parser: looks not properly defined!");
            return false;
        }
        element = element->NextSiblingElement("looks");
    }
    
    //Load static objects (optional)
    element = root->FirstChildElement("static");
    while(element != nullptr)
    {
        if(!ParseStatic(element))
        {
            cError("Scenario parser: static object not properly defined!");
            return false;
        }
        element = element->NextSiblingElement("static");
    }

    //Load animated objects (optional)
    element = root->FirstChildElement("animated");
    while(element != nullptr)
    {
        if(!ParseAnimated(element))
        {
            cError("Scenario parser: animated object not properly defined!");
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
            cError("Scenario parser: dynamic object not properly defined!");
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
            cError("Scenario parser: robot not properly defined!");
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
            cError("Scenario parser: sensor not properly defined!");
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
            cError("Scenario parser: light not properly defined!");
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item;
            if((item = element->FirstChildElement("world_transform")) == nullptr || !ParseTransform(item, origin))
            {
                cError("Scenario parser: light not properly defined!");
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
            cError("Scenario parser: communication device not properly defined!");
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item;
            if((item = element->FirstChildElement("world_transform")) == nullptr || !ParseTransform(item, origin))
            {
                cError("Scenario parser: communication device not properly defined!");
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
            cError("Scenario parser: contact not properly defined!");
            return false;
        }
        element = element->NextSiblingElement("contact");
    }
    
    return true;
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
                    cError("Scenario parser: argument '%s' does not exist!", argName.c_str());
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
        return false;
    
    Scalar lat, lon;
    if(ned->QueryAttribute("latitude", &lat) != XML_SUCCESS)
        return false;
    if(ned->QueryAttribute("longitude", &lon) != XML_SUCCESS)
        return false;
    sm->getNED()->Init(lat, lon, Scalar(0));
    
    //Setup ocean
    XMLElement* ocean = element->FirstChildElement("ocean");
    if(ocean != nullptr)
    {
        //Basic setup
        Scalar wavesHeight(0);
        Scalar waterDensity(1000);
        Scalar jerlov(0.2);

        if((item = ocean->FirstChildElement("waves")) != nullptr)
        {
            item->QueryAttribute("height", &wavesHeight);
        }
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
            XMLElement* item2;
            do
            {
                //Get type of current
                const char* currentType;
                if(item->QueryStringAttribute("type", &currentType) != XML_SUCCESS)
                    return false;
                std::string currentTypeStr(currentType);
                
                //Create current
                if(currentTypeStr == "uniform")
                {
                    const char* vel;
                    Scalar vx,vy,vz;
            
                    if((item2 = item->FirstChildElement("velocity")) == nullptr)
                        return false;
                    if(item2->QueryStringAttribute("xyz", &vel) != XML_SUCCESS)
                        return false;
                    if(sscanf(vel, "%lf %lf %lf", &vx, &vy, &vz) != 3)
                        return false;
        
                    ocn->AddVelocityField(new Uniform(Vector3(vx, vy, vz)));
                }
                else if(currentTypeStr == "jet")
                {
                    const char* center;
                    const char* vel;
                    Scalar cx, cy, cz;
                    Scalar vx, vy, vz;
                    Scalar radius;
                    
                    if((item2 = item->FirstChildElement("center")) == nullptr)
                        return false;
                    if(item2->QueryStringAttribute("xyz", &center) != XML_SUCCESS)
                        return false;
                    if(sscanf(center, "%lf %lf %lf", &cx, &cy, &cz) != 3)
                        return false;
                    if((item2 = item->FirstChildElement("outlet")) == nullptr)
                        return false;
                    if(item2->QueryAttribute("radius", &radius) != XML_SUCCESS)
                        return false;
                    if((item2 = item->FirstChildElement("velocity")) == nullptr)
                        return false;
                    if(item2->QueryStringAttribute("xyz", &vel) != XML_SUCCESS)
                        return false;
                    if(sscanf(vel, "%lf %lf %lf", &vx, &vy, &vz) != 3)
                        return false;
                    
                    Vector3 velocity(vx, vy, vz);
                    Vector3 dir = velocity.normalized();
                    ocn->AddVelocityField(new Jet(Vector3(cx, cy, cz), dir, radius, velocity.norm()));
                }
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
            if(item->QueryAttribute("azimuth", &az) != XML_SUCCESS)
                return false;
            if(item->QueryAttribute("elevation", &elev) != XML_SUCCESS)
                return false;
            sm->getAtmosphere()->SetupSunPosition(az, elev);
        }

        //Winds
        if((item = atmosphere->FirstChildElement("wind")) != nullptr)
        {
            Atmosphere* atm = sm->getAtmosphere();
            XMLElement* item2;
            do
            {
                //Get type of wind
                const char* windType;
                if(item->QueryStringAttribute("type", &windType) != XML_SUCCESS)
                    return false;
                std::string windTypeStr(windType);
                
                //Create wind
                if(windTypeStr == "uniform")
                {
                    const char* vel;
                    Scalar vx,vy,vz;
            
                    if((item2 = item->FirstChildElement("velocity")) == nullptr)
                        return false;
                    if(item2->QueryStringAttribute("xyz", &vel) != XML_SUCCESS)
                        return false;
                    if(sscanf(vel, "%lf %lf %lf", &vx, &vy, &vz) != 3)
                        return false;
        
                    atm->AddVelocityField(new Uniform(Vector3(vx, vy, vz)));
                }
                else if(windTypeStr == "jet")
                {
                    const char* center;
                    const char* vel;
                    Scalar cx, cy, cz;
                    Scalar vx, vy, vz;
                    Scalar radius;
                    
                    if((item2 = item->FirstChildElement("center")) == nullptr)
                        return false;
                    if(item2->QueryStringAttribute("xyz", &center) != XML_SUCCESS)
                        return false;
                    if(sscanf(center, "%lf %lf %lf", &cx, &cy, &cz) != 3)
                        return false;
                    if((item2 = item->FirstChildElement("outlet")) == nullptr)
                        return false;
                    if(item2->QueryAttribute("radius", &radius) != XML_SUCCESS)
                        return false;
                    if((item2 = item->FirstChildElement("velocity")) == nullptr)
                        return false;
                    if(item2->QueryStringAttribute("xyz", &vel) != XML_SUCCESS)
                        return false;
                    if(sscanf(vel, "%lf %lf %lf", &vx, &vy, &vz) != 3)
                        return false;
                    
                    Vector3 velocity(vx, vy, vz);
                    Vector3 dir = velocity.normalized();
                    atm->AddVelocityField(new Jet(Vector3(cx, cy, cz), dir, radius, velocity.norm()));
                }
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
        return false; //There has to be at least one material defined!
        
    //Iterate through all materials
    while(mat != nullptr)
    {
        const char* name = nullptr;
        Scalar density, restitution;
        if(mat->QueryStringAttribute("name", &name) != XML_SUCCESS)
            return false;
        if(mat->QueryAttribute("density", &density) != XML_SUCCESS)
            return false;
        if(mat->QueryAttribute("restitution", &restitution) != XML_SUCCESS)
            return false;
        sm->getMaterialManager()->CreateMaterial(std::string(name), density, restitution);
        mat = mat->NextSiblingElement("material");
    }
    
    //Read friction table
    XMLElement* table = element->FirstChildElement("friction_table");
    
    if(table != nullptr) //Optional
    {
        XMLElement* friction = table->FirstChildElement("friction");
        
        while(friction != nullptr)
        {
            const char* name1 = nullptr;
            const char* name2 = nullptr;
            Scalar fstatic, fdynamic;
            if(friction->QueryStringAttribute("material1", &name1) != XML_SUCCESS)
                return false;
            if(friction->QueryStringAttribute("material2", &name2) != XML_SUCCESS)
                return false;
            if(friction->QueryAttribute("static", &fstatic) != XML_SUCCESS)
                return false;
            if(friction->QueryAttribute("dynamic", &fdynamic) != XML_SUCCESS)
                return false;
            sm->getMaterialManager()->SetMaterialsInteraction(std::string(name1), std::string(name2), fstatic, fdynamic);
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
        return false; //There has to be at least one look defined!
    
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
            return false;
        if(!ParseColor(look, color))
            return false;    
        if(look->QueryAttribute("roughness", &roughness) != XML_SUCCESS)
            return false;
        if(look->QueryAttribute("metalness", &metalness) != XML_SUCCESS)
            metalness = Scalar(0);
        if(look->QueryAttribute("reflectivity", &reflectivity) != XML_SUCCESS)
            reflectivity = Scalar(0);
        if(look->QueryStringAttribute("texture", &texture) == XML_SUCCESS)
            textureStr = GetFullPath(std::string(texture));
        if(look->QueryStringAttribute("normal_map", &normalMap) == XML_SUCCESS)
            normalMapStr = GetFullPath(std::string(normalMap));
        
        sm->CreateLook(name, color, roughness, metalness, reflectivity, textureStr, normalMapStr);
        look = look->NextSiblingElement("look");
    }
    
    return true;
}

bool ScenarioParser::ParseStatic(XMLElement* element)
{
    //---- Basic ----
    const char* name = nullptr;
    const char* type = nullptr;
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
        return false;
    if(element->QueryStringAttribute("type", &type) != XML_SUCCESS)
        return false;
    std::string typestr(type);
        
    //---- Common ----
    XMLElement* item;
    const char* mat = nullptr;
    const char* look = nullptr;
    unsigned int uvMode = 0;
    float uvScale = 1.f;
    Transform trans;
    
    //Material
    if((item = element->FirstChildElement("material")) == nullptr)
        return false;
    if(item->QueryStringAttribute("name", &mat) != XML_SUCCESS)
        return false;
    //Look
    if((item = element->FirstChildElement("look")) == nullptr)
        return false;
    if(item->QueryStringAttribute("name", &look) != XML_SUCCESS)
        return false;
    item->QueryAttribute("uv_mode", &uvMode); //Optional
    item->QueryAttribute("uv_scale", &uvScale); //Optional
    //Transform
    if((item = element->FirstChildElement("world_transform")) == nullptr || !ParseTransform(item, trans))
        return false;
  
    //---- Object specific ----
    StaticEntity* object;
        
    if(typestr == "box")
    {
        const char* dims = nullptr;
        Scalar dimX, dimY, dimZ;
            
        if((item = element->FirstChildElement("dimensions")) == nullptr)
            return false;
        if(item->QueryStringAttribute("xyz", &dims) != XML_SUCCESS)
            return false;
        if(sscanf(dims, "%lf %lf %lf", &dimX, &dimY, &dimZ) != 3)
            return false;
            
        object = new Obstacle(std::string(name), Vector3(dimX, dimY, dimZ), std::string(mat), std::string(look), uvMode);
    }
    else if(typestr == "cylinder")
    {
        Scalar radius, height;
            
        if((item = element->FirstChildElement("dimensions")) == nullptr)
            return false;
        if(item->QueryAttribute("radius", &radius) != XML_SUCCESS)
            return false;
        if(item->QueryAttribute("height", &height) != XML_SUCCESS)
            return false;
                
        object = new Obstacle(std::string(name), radius, height, std::string(mat), std::string(look));
    }
    else if(typestr == "sphere")
    {
        Scalar radius;
            
        if((item = element->FirstChildElement("dimensions")) == nullptr)
            return false;
        if(item->QueryAttribute("radius", &radius) != XML_SUCCESS)
            return false;
            
        object = new Obstacle(std::string(name), radius, std::string(mat), std::string(look));
    }
    else if(typestr == "model")
    {
        const char* phyMesh = nullptr;
        Scalar phyScale;
        Transform phyOrigin;
        
        if((item = element->FirstChildElement("physical")) == nullptr)
            return false;
        if((item = item->FirstChildElement("mesh")) == nullptr)
            return false;
        if(item->QueryStringAttribute("filename", &phyMesh) != XML_SUCCESS)
            return false;
        if(item->QueryAttribute("scale", &phyScale) != XML_SUCCESS)
            phyScale = Scalar(1);
        if((item = item->NextSiblingElement("origin")) == nullptr || !ParseTransform(item, phyOrigin))
            return false;
        
        if((item = element->FirstChildElement("visual")) != nullptr)
        {
            const char* graMesh = nullptr;
            Scalar graScale;
            Transform graOrigin;
            
            if((item = item->FirstChildElement("mesh")) == nullptr)
                return false;
            if(item->QueryStringAttribute("filename", &graMesh) != XML_SUCCESS)
                return false;
            if(item->QueryAttribute("scale", &graScale) != XML_SUCCESS)
                graScale = Scalar(1);
            if((item = item->NextSiblingElement("origin")) == nullptr || !ParseTransform(item, graOrigin))
                return false;
          
            object = new Obstacle(std::string(name), GetFullPath(std::string(graMesh)), graScale, graOrigin, GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, std::string(mat), std::string(look));
        }
        else
        {
            object = new Obstacle(std::string(name), GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, std::string(mat), std::string(look));
        }
    }
    else if(typestr == "plane")
    {
        object = new Plane(std::string(name), Scalar(10000), std::string(mat), std::string(look), uvScale);
    } 
    else if(typestr == "terrain")
    {
        const char* heightmap = nullptr;
        Scalar scaleX, scaleY, height;
            
        if((item = element->FirstChildElement("height_map")) == nullptr)
            return false;
        if(item->QueryStringAttribute("filename", &heightmap) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("dimensions")) == nullptr)
            return false;
        if(item->QueryAttribute("scalex", &scaleX) != XML_SUCCESS)
            return false;
        if(item->QueryAttribute("scaley", &scaleY) != XML_SUCCESS)
            return false;
        if(item->QueryAttribute("height", &height) != XML_SUCCESS)
            return false;
            
        object = new Terrain(std::string(name), GetFullPath(std::string(heightmap)), scaleX, scaleY, height, std::string(mat), std::string(look), uvScale);
    }
    else
        return false;

    //---- Vision sensors ----
    item = element->FirstChildElement("sensor");
    while(item != nullptr)
    {
        if(!ParseSensor(item, (Entity*)object))
        {
            cError("Scenario parser: sensor of static body '%s' not properly defined!", name);
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
            cError("Scenario parser: light of static body '%s' not properly defined!", name);
            delete object;
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                cError("Scenario parser: light of static body '%s' not properly defined!", name);
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
            cError("Scenario parser: communication device of static body '%s' not properly defined!", name);
            delete object;
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                cError("Scenario parser: communication device of static body '%s' not properly defined!", name);
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
        return false;
    if(element->QueryStringAttribute("type", &type) != XML_SUCCESS)
        return false;
    element->QueryAttribute("collisions", &collides); //Optional
    std::string typestr(type);
        
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
        if((item = element->FirstChildElement("material")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &mat) != XML_SUCCESS)
            return false;
        //Look
        if((item = element->FirstChildElement("look")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &look) != XML_SUCCESS)
            return false;
        item->QueryAttribute("uv_mode", &uvMode); //Optional
        item->QueryAttribute("uv_scale", &uvScale); //Optional
    }
        
    //---- Trajectory ----
    Trajectory* tr;
    if((item = element->FirstChildElement("trajectory")) != nullptr)
    {
        const char* trType = nullptr;
        if(item->QueryStringAttribute("type", &trType) != XML_SUCCESS)
        {
            cError("Scenario parser: trajectory type not defined!");
            return false;
        }
        std::string trTypeStr(trType);

        const char* playMode = nullptr;
        if(item->QueryStringAttribute("playback", &playMode) != XML_SUCCESS && trTypeStr != "manual")
        {
            cError("Scenario parser: trajectory playback mode not defined!");
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
            cError("Scenario parser: incorrect trajectory playback mode!");
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
                    cError("Scenario parser: trajectory keypoint not properly defined!");
                    delete tr;
                    return false;
                }
                pwl->AddKeyPoint(t, T);
                key = key->NextSiblingElement("keypoint");
            }
        }
        else
        {
            cError("Scenario parser: unknown trajectory type!");
            return false;        
        }
    }
    else
    {
        cError("Scenario parser: no trajectory defined for animated object!");
        return false;
    }

    //---- Object specific ----
    AnimatedEntity* object;
        
    if(typestr == "empty")
    {
        object = new AnimatedEntity(std::string(name), tr);
    }
    else if(typestr == "box")
    {
        const char* dims = nullptr;
        Scalar dimX, dimY, dimZ;
        Transform origin;

        if((item = element->FirstChildElement("dimensions")) == nullptr)
            return false;
        if(item->QueryStringAttribute("xyz", &dims) != XML_SUCCESS)
            return false;
        if(sscanf(dims, "%lf %lf %lf", &dimX, &dimY, &dimZ) != 3)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;        

        object = new AnimatedEntity(std::string(name), tr, Vector3(dimX, dimY, dimZ), origin, std::string(mat), std::string(look), collides);
    }
    else if(typestr == "cylinder")
    {
        Scalar radius;
        Scalar height;
        Transform origin;
            
        if((item = element->FirstChildElement("dimensions")) == nullptr)
            return false;
        if(item->QueryAttribute("radius", &radius) != XML_SUCCESS)
            return false;
        if(item->QueryAttribute("height", &height) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;
            
        object = new AnimatedEntity(std::string(name), tr, radius, height, origin, std::string(mat), std::string(look), collides);
    }
    else if(typestr == "sphere")
    {
        Scalar radius;
        Transform origin;
            
        if((item = element->FirstChildElement("dimensions")) == nullptr)
            return false;
        if(item->QueryAttribute("radius", &radius) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;
            
        object = new AnimatedEntity(std::string(name), tr, radius, origin, std::string(mat), std::string(look), collides);
    }
    else if(typestr == "model")
    {
        const char* phyMesh = nullptr;
        Scalar phyScale;
        Transform phyOrigin;
        
        if((item = element->FirstChildElement("physical")) == nullptr)
            return false;
        if((item = item->FirstChildElement("mesh")) == nullptr)
            return false;
        if(item->QueryStringAttribute("filename", &phyMesh) != XML_SUCCESS)
            return false;
        if(item->QueryAttribute("scale", &phyScale) != XML_SUCCESS)
            phyScale = Scalar(1);
        if((item = item->NextSiblingElement("origin")) == nullptr || !ParseTransform(item, phyOrigin))
            return false;
        
        if((item = element->FirstChildElement("visual")) != nullptr)
        {
            const char* graMesh = nullptr;
            Scalar graScale;
            Transform graOrigin;
            
            if((item = item->FirstChildElement("mesh")) == nullptr)
                return false;
            if(item->QueryStringAttribute("filename", &graMesh) != XML_SUCCESS)
                return false;
            if(item->QueryAttribute("scale", &graScale) != XML_SUCCESS)
                graScale = Scalar(1);
            if((item = item->NextSiblingElement("origin")) == nullptr || !ParseTransform(item, graOrigin))
                return false;
          
            object = new AnimatedEntity(std::string(name), tr, GetFullPath(std::string(graMesh)), graScale, graOrigin, GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, std::string(mat), std::string(look), collides);
        }
        else
        {
            object = new AnimatedEntity(std::string(name), tr, GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, std::string(mat), std::string(look), collides);
        }
    }
    else
    {
        cError("Scenario parser: incorrect animated object type!");
        delete tr;
        return false;
    }
        
    //---- Sensors ----
    item = element->FirstChildElement("sensor");
    while(item != nullptr)
    {
        if(!ParseSensor(item, (Entity*)object))
        {
            cError("Scenario parser: sensor of animated body '%s' not properly defined!", name);
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
            cError("Scenario parser: light of animated body '%s' not properly defined!", name);
            delete object;
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                cError("Scenario parser: light of animated body '%s' not properly defined!", name);
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
            cError("Scenario parser: communication device of animated body '%s' not properly defined!", name);
            delete object;
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                cError("Scenario parser: communication device of animated body '%s' not properly defined!", name);
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
        delete solid;
        return false;
    }

    //---- Sensors -----
    item = element->FirstChildElement("sensor");
    while(item != nullptr)
    {
        if(!ParseSensor(item, (Entity*)solid))
        {
            cError("Scenario parser: sensor of dynamic body '%s' not properly defined!", solid->getName().c_str());
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
            cError("Scenario parser: light of dynamic body '%s' not properly defined!", solid->getName().c_str());
            delete solid;
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                cError("Scenario parser: light of dynamic body '%s' not properly defined!", solid->getName().c_str());
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
            cError("Scenario parser: communication device of dynamic body '%s' not properly defined!", solid->getName().c_str());
            delete solid;
            return false;
        }
        else
        {
            Transform origin;
            XMLElement* item2;
            if( (item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, origin) )
            {
                cError("Scenario parser: communication device of dynamic body '%s' not properly defined!", solid->getName().c_str());
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
        return false;
    if(element->QueryStringAttribute("type", &type) != XML_SUCCESS)
        return false;
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
            return false;
    }
    if(element->QueryAttribute("buoyant", &buoyant) != XML_SUCCESS)
        buoyant = true;
    
    std::string typeStr(type);
    std::string solidName = ns != "" ? ns + "/" + std::string(name) : std::string(name);
    XMLElement* item;

    if(typeStr == "compound")
    {
        //First external part
        SolidEntity* part = nullptr;
        Compound* comp = nullptr;
        Transform partOrigin;
      
        if((item = element->FirstChildElement("external_part")) == nullptr)
            return false;
        if(!ParseSolid(item, part, solidName, true))
            return false;
        XMLElement* item2;
        if((item2 = item->FirstChildElement("compound_transform")) == nullptr || !ParseTransform(item2, partOrigin))
            return false;
        comp = new Compound(solidName, part, partOrigin, ePhyType);
        
        //Iterate through all external parts
        item = item->NextSiblingElement("external_part");
        while(item != nullptr)
        {
            if(!ParseSolid(item, part, solidName, true))
            {
                delete comp;
                return false;
            }
            if((item2 = item->FirstChildElement("compound_transform")) == nullptr || !ParseTransform(item2, partOrigin))
            {
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
                delete comp;
                return false;
            }
            if((item2 = item->FirstChildElement("compound_transform")) == nullptr || !ParseTransform(item2, partOrigin))
            {
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
        if((item = element->FirstChildElement("material")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &mat) != XML_SUCCESS)
            return false;
        //Look
        if((item = element->FirstChildElement("look")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &look) != XML_SUCCESS)
            return false;
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
                return false;
        }
        //---- Specific ----
        if(typeStr == "box")
        {
            const char* dims = nullptr;
            Scalar dimX, dimY, dimZ;
            Scalar thickness;
            
            if((item = element->FirstChildElement("dimensions")) == nullptr)
                return false;
            if(item->QueryStringAttribute("xyz", &dims) != XML_SUCCESS)
                return false;
            if(sscanf(dims, "%lf %lf %lf", &dimX, &dimY, &dimZ) != 3)
                return false;
            if(item->QueryAttribute("thickness", &thickness) != XML_SUCCESS)
                thickness = Scalar(-1);
            
            solid = new Box(solidName, Vector3(dimX, dimY, dimZ), origin, std::string(mat), ePhyType, std::string(look), thickness, buoyant);
        }
        else if(typeStr == "cylinder")
        {
            Scalar radius, height, thickness;
            
            if((item = element->FirstChildElement("dimensions")) == nullptr)
                return false;
            if(item->QueryAttribute("radius", &radius) != XML_SUCCESS)
                return false;
            if(item->QueryAttribute("height", &height) != XML_SUCCESS)
                return false;
            if(item->QueryAttribute("thickness", &thickness) != XML_SUCCESS)
                thickness = Scalar(-1);
                
            solid = new Cylinder(solidName, radius, height, origin, std::string(mat), ePhyType, std::string(look), thickness, buoyant);
        }
        else if(typeStr == "sphere")
        {
            Scalar radius, thickness;
            
            if((item = element->FirstChildElement("dimensions")) == nullptr)
                return false;
            if(item->QueryAttribute("radius", &radius) != XML_SUCCESS)
                return false;
            if(item->QueryAttribute("thickness", &thickness) != XML_SUCCESS)
                thickness = Scalar(-1);
            
            solid = new Sphere(solidName, radius, origin, std::string(mat), ePhyType, std::string(look), thickness, buoyant);
        }
        else if(typeStr == "torus")
        {
            Scalar radiusMaj, radiusMin, thickness;
            
            if((item = element->FirstChildElement("dimensions")) == nullptr)
                return false;
            if(item->QueryAttribute("major_radius", &radiusMaj) != XML_SUCCESS)
                return false;
            if(item->QueryAttribute("minor_radius", &radiusMin) != XML_SUCCESS)
                return false;
            if(item->QueryAttribute("thickness", &thickness) != XML_SUCCESS)
                thickness = Scalar(-1);
            
            solid = new Torus(solidName, radiusMaj, radiusMin, origin, std::string(mat), ePhyType, std::string(look), thickness, buoyant);
        }
        else if(typeStr == "wing")
        {
            Scalar baseChord, tipChord, length, thickness;
            const char* naca = nullptr;

            if((item = element->FirstChildElement("dimensions")) == nullptr)
                return false;
            if(item->QueryAttribute("base_chord", &baseChord) != XML_SUCCESS)
                return false;
            if(item->QueryAttribute("tip_chord", &tipChord) != XML_SUCCESS)
                return false;
            if(item->QueryAttribute("length", &length) != XML_SUCCESS)
                return false;
            if(item->QueryStringAttribute("naca", &naca) != XML_SUCCESS)
                return false;
            if(item->QueryAttribute("thickness", &thickness) != XML_SUCCESS)
                thickness = Scalar(-1);

            std::string nacaStr(naca);
            if(nacaStr.size() != 4)
                return false;
            
            solid = new Wing(solidName, baseChord, tipChord, nacaStr, length, origin, std::string(mat), ePhyType, std::string(look), thickness, buoyant);
        }
        else if(typeStr == "model")
        {
            const char* phyMesh = nullptr;
            Scalar phyScale;
            Transform phyOrigin;
            Scalar thickness;
        
            if((item = element->FirstChildElement("physical")) == nullptr)
                return false;
            XMLElement* item2;
            if((item2 = item->FirstChildElement("mesh")) == nullptr)
                return false;
            if(item2->QueryStringAttribute("filename", &phyMesh) != XML_SUCCESS)
                return false;
            if(item2->QueryAttribute("scale", &phyScale) != XML_SUCCESS)
                phyScale = Scalar(1);
            if((item2 = item->FirstChildElement("thickness")) != nullptr)
            {
                if(item2->QueryAttribute("value", &thickness) != XML_SUCCESS)
                    thickness = Scalar(-1);
            }
            else
                thickness = Scalar(-1);
            if((item2 = item->FirstChildElement("origin")) == nullptr || !ParseTransform(item2, phyOrigin))
                return false;
        
            if((item = element->FirstChildElement("visual")) != nullptr)
            {
                const char* graMesh = nullptr;
                Scalar graScale;
                Transform graOrigin;
            
                if((item = item->FirstChildElement("mesh")) == nullptr)
                    return false;
                if(item->QueryStringAttribute("filename", &graMesh) != XML_SUCCESS)
                    return false;
                if(item->QueryAttribute("scale", &graScale) != XML_SUCCESS)
                    graScale = Scalar(1);
                if((item = item->NextSiblingElement("origin")) == nullptr || !ParseTransform(item, graOrigin))
                    return false;
          
                solid = new Polyhedron(solidName, GetFullPath(std::string(graMesh)), graScale, graOrigin, GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, std::string(mat), ePhyType, std::string(look), thickness, buoyant); 
            }
            else
            {
                solid = new Polyhedron(solidName, GetFullPath(std::string(phyMesh)), phyScale, phyOrigin, std::string(mat), ePhyType, std::string(look), thickness, buoyant); 
            }
        }
        else
            return false;
         
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
            solid->SetContactProperties(true, contactK, contactD);
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
        return false;
    if(element->QueryAttribute("fixed", &fixed) != XML_SUCCESS)
        return false;
    if(element->QueryAttribute("self_collisions", &selfCollisions) != XML_SUCCESS)
        return false;
    if((item = element->FirstChildElement("world_transform")) == nullptr || !ParseTransform(item, trans))
        return false;

    Robot* robot = new Robot(std::string(name), fixed);

    //---- Links ----
    //Base link
    SolidEntity* baseLink = nullptr;
    
    if((item = element->FirstChildElement("base_link")) == nullptr)
    {
        cError("Scenario parser: base link of robot '%s' missing!", name);
        delete robot;
        return false;
    }
    if(!ParseLink(item, robot, baseLink))
    {
        cError("Scenario parser: base link of robot '%s' not properly defined!", name);
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
            cError("Scenario parser: link of robot '%s' not properly defined!", name);
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
            cError("Scenario parser: joint of robot '%s' not properly defined!", name);
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
            cError("Scenario parser: sensor of robot '%s' not properly defined!", name);
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
            cError("Scenario parser: actuator of robot '%s' not properly defined!", name);
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
            cError("Scenario parser: light of robot '%s' not properly defined!", name);
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
                cError("Scenario parser: light of robot '%s' not properly defined!", name);
                delete l;
                delete robot;
                return false;
            }
            robot->AddLinkActuator(l, robot->getName() + "/" + std::string(linkName), origin);
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
            cError("Scenario parser: communication device of robot '%s' not properly defined!", name);
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
                cError("Scenario parser: communication device of robot '%s' not properly defined!", name);
                delete comm;
                delete robot;
                return false;
            }
            robot->AddComm(comm, robot->getName() + "/" + std::string(linkName), origin);
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
        return false;
    if(element->QueryStringAttribute("type", &type) != XML_SUCCESS)
        return false;
    std::string typeStr(type);
    XMLElement* item;
    if((item = element->FirstChildElement("parent")) == nullptr)
        return false;
    if(item->QueryStringAttribute("name", &parent) != XML_SUCCESS)
        return false;
    if((item = element->FirstChildElement("child")) == nullptr)
        return false;
    if(item->QueryStringAttribute("name", &child) != XML_SUCCESS)
        return false;    
    if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
        return false;
    
    std::string jointName = robot->getName() + "/" + std::string(name);
    std::string parentName = robot->getName() + "/" + std::string(parent);
    std::string childName = robot->getName() + "/" + std::string(child);

    if(typeStr == "fixed")
    {
        robot->DefineFixedJoint(jointName, parentName, childName, origin);
    }
    else if(typeStr == "prismatic" || typeStr == "revolute")
    {
        const char* vec = nullptr;
        Scalar x, y, z;
        Scalar posMin(1);
        Scalar posMax(-1);
        Scalar damping(-1);
        
        if((item = element->FirstChildElement("axis")) == nullptr)
            return false;
        if(item->QueryStringAttribute("xyz", &vec) != XML_SUCCESS)
            return false;
        if(sscanf(vec, "%lf %lf %lf", &x, &y, &z) != 3)
            return false;
        if((item = element->FirstChildElement("limits")) != nullptr) //Optional
        {
            if((item->QueryAttribute("min", &posMin) != XML_SUCCESS) || (item->QueryAttribute("max", &posMax) != XML_SUCCESS))
                return false;
        }
        if((item = element->FirstChildElement("damping")) != nullptr) //Optional
        {
            if(item->QueryAttribute("value", &damping) != XML_SUCCESS)
                return false;
        }
        
        if(typeStr == "prismatic")
            robot->DefinePrismaticJoint(jointName, parentName, childName, origin, Vector3(x, y, z), std::make_pair(posMin, posMax), damping);
        else
            robot->DefineRevoluteJoint(jointName, parentName, childName, origin, Vector3(x, y, z), std::make_pair(posMin, posMax), damping);
    }
    else
        return false;
    
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
                delete sens;
                return false;
            }
            if((item = element->FirstChildElement("origin")) == nullptr 
                || !ParseTransform(item, origin))
            {
                delete sens;
                return false;
            }
            if(sens->getType() == SensorType::LINK)
                robot->AddLinkSensor((LinkSensor*)sens, robot->getName() + "/" + std::string(linkName), origin);
            else
                robot->AddVisionSensor((VisionSensor*)sens, robot->getName() + "/" + std::string(linkName), origin);
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

bool ScenarioParser::ParseActuator(XMLElement* element, Robot* robot)
{
    //---- Common ----
    const char* name = nullptr;
    const char* type = nullptr;
    
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
        return false;
    if(element->QueryStringAttribute("type", &type) != XML_SUCCESS)
        return false;
    std::string typeStr(type);
    
    std::string actuatorName = robot->getName() + "/" + std::string(name);
    
    //---- Specific ----
    XMLElement* item;
    if(typeStr == "servo")
    {
        const char* jointName = nullptr;
        Scalar kp, kv, maxTau;
        
        if((item = element->FirstChildElement("joint")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &jointName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("controller")) == nullptr 
            || item->QueryAttribute("position_gain", &kp) != XML_SUCCESS 
            || item->QueryAttribute("velocity_gain", &kv) != XML_SUCCESS
            || item->QueryAttribute("max_torque", &maxTau) != XML_SUCCESS)
            return false;
        
        Servo* srv = new Servo(actuatorName, kp, kv, maxTau);
        robot->AddJointActuator(srv, robot->getName() + "/" + std::string(jointName));
    }
    else if(typeStr == "thruster" || typeStr == "propeller")
    {
        const char* linkName = nullptr;
        const char* propFile = nullptr;
        const char* mat = nullptr;
        const char* look = nullptr;
        Scalar diameter, cThrust, cTorque, maxRpm, propScale, cThrustBack;
        bool rightHand;
        bool inverted = false;
        Transform origin;
        
        if((item = element->FirstChildElement("link")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;
        if((item = element->FirstChildElement("specs")) == nullptr 
            || item->QueryAttribute("thrust_coeff", &cThrust) != XML_SUCCESS 
            || item->QueryAttribute("torque_coeff", &cTorque) != XML_SUCCESS
            || item->QueryAttribute("max_rpm", &maxRpm) != XML_SUCCESS)
            return false;
        cThrustBack = cThrust;
        item->QueryAttribute("thrust_coeff_backward", &cThrustBack); //Optional
        item->QueryAttribute("inverted", &inverted); //Optional
        if((item = element->FirstChildElement("propeller")) == nullptr || item->QueryAttribute("diameter", &diameter) != XML_SUCCESS || item->QueryAttribute("right", &rightHand) != XML_SUCCESS)
            return false;
        XMLElement* item2;
        if((item2 = item->FirstChildElement("mesh")) == nullptr || item2->QueryStringAttribute("filename", &propFile) != XML_SUCCESS)
            return false;
        if(item2->QueryAttribute("scale", &propScale) != XML_SUCCESS)
            propScale = Scalar(1);
        if((item2 = item->FirstChildElement("material")) == nullptr || item2->QueryStringAttribute("name", &mat) != XML_SUCCESS)
            return false;
        if((item2 = item->FirstChildElement("look")) == nullptr || item2->QueryStringAttribute("name", &look) != XML_SUCCESS)
            return false;

        if(typeStr == "thruster")
        {
            Polyhedron* prop = new Polyhedron(actuatorName + "/Propeller", GetFullPath(std::string(propFile)), propScale, I4(), std::string(mat), BodyPhysicsType::SUBMERGED, std::string(look));
            Thruster* th = new Thruster(actuatorName, prop, diameter, std::make_pair(cThrust, cThrustBack), cTorque, maxRpm, rightHand, inverted);
            robot->AddLinkActuator(th, robot->getName() + "/" + std::string(linkName), origin);
        }
        else //propeller
        {
            Polyhedron* prop = new Polyhedron(actuatorName + "/Propeller", GetFullPath(std::string(propFile)), propScale, I4(), std::string(mat), BodyPhysicsType::AERODYNAMIC, std::string(look));
            Propeller* p = new Propeller(actuatorName, prop, diameter, cThrust, cTorque, maxRpm, rightHand, inverted);
            robot->AddLinkActuator(p, robot->getName() + "/" + std::string(linkName), origin);
        }
    }
    else if(typeStr == "vbs")
    {
        const char* linkName = nullptr;
        Scalar initialV;
        std::vector<std::string> vMeshes;
        Transform origin;
        
        if((item = element->FirstChildElement("link")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;
        if((item = element->FirstChildElement("volume")) == nullptr || item->QueryAttribute("initial", &initialV) != XML_SUCCESS)
            return false;
        XMLElement* item2;
        const char* meshFile;
        if((item2 = item->FirstChildElement("mesh")) == nullptr || item2->QueryStringAttribute("filename", &meshFile) != XML_SUCCESS)
            return false;
        vMeshes.push_back(GetFullPath(std::string(meshFile)));
        while((item2 = item2->NextSiblingElement("mesh")) != nullptr)
        {
            const char* meshFile2;
            if(item2->QueryStringAttribute("filename", &meshFile2) != XML_SUCCESS)
                return false;
            vMeshes.push_back(GetFullPath(std::string(meshFile2)));
        }
        if(vMeshes.size() < 2)
            return false;
        
        VariableBuoyancy* vbs = new VariableBuoyancy(actuatorName, vMeshes, initialV);
        robot->AddLinkActuator(vbs, robot->getName() + "/" + std::string(linkName), origin);
    }
    else
        return false;
    
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
            cError("Scenario parser: joint sensors can only be attached to robotic joints!");
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
                cError("Scenario parser: link sensors can only be attached to robotic links and moving bodies!");
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
                cError("Scenario parser: link sensors can only be attached to robotic links and moving bodies!");
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
                cError("Scenario parser: trying to attach vision sensor to a non-physical body!");
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

Sensor* ScenarioParser::ParseSensor(XMLElement* element, const std::string& namePrefix)
{
    //---- Common ----
    const char* name = nullptr;
    const char* type = nullptr;
    Scalar rate;
    
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
        return nullptr;
    if(element->QueryStringAttribute("type", &type) != XML_SUCCESS)
        return nullptr;
    if(element->QueryAttribute("rate", &rate) != XML_SUCCESS)
        rate = Scalar(-1);
    std::string typeStr(type);
    
    std::string sensorName = std::string(name);
    if(namePrefix != "")
        sensorName = namePrefix + "/" + sensorName;
    
    //---- Specific ----
    XMLElement* item;
    if(typeStr == "gyro")
    {
        int history;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        Gyroscope* gyro = new Gyroscope(sensorName, rate, history);
        
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            Scalar velocity;
            if(item->QueryAttribute("angular_velocity", &velocity) != XML_SUCCESS)
            {
                delete gyro;
                return nullptr;
            }
            gyro->setRange(velocity);
        }
        
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar velocity;
            Scalar bias = Scalar(0);
            if(item->QueryAttribute("angular_velocity", &velocity) != XML_SUCCESS)
            {
                delete gyro;
                return nullptr;
            }
            item->QueryAttribute("bias", &bias);
            gyro->setNoise(velocity, bias);
        }
        
        return gyro;
    }
    else if(typeStr == "imu")
    {
        int history;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        IMU* imu = new IMU(sensorName, rate, history);
        
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            const char* velocity = nullptr;
            Vector3 vxyz;
            Scalar v;
            if(item->QueryStringAttribute("angular_velocity", &velocity) == XML_SUCCESS  
               && ParseVector(velocity, vxyz))
            {
                imu->setRange(vxyz);    
            }
            else if(item->QueryAttribute("angular_velocity", &v) == XML_SUCCESS)
            {
                imu->setRange(Vector3(v, v, v));
            }
            else
            {
                delete imu;
                return nullptr;
            }
        }
        
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            const char* angle = nullptr;
            const char* velocity = nullptr;
            Vector3 axyz;
            Vector3 avxyz;
            Scalar a;
            Scalar av;
            Scalar yawDrift = Scalar(0);
            item->QueryAttribute("yaw_drift", &yawDrift);

            if(item->QueryStringAttribute("angle", &angle) == XML_SUCCESS && ParseVector(angle, axyz))
                ;
            else if(item->QueryAttribute("angle", &a) == XML_SUCCESS)
                axyz = Vector3(a, a, a);
            else
            {
                delete imu;
                return nullptr;
            }
            
            if(item->QueryStringAttribute("angular_velocity", &velocity) == XML_SUCCESS && ParseVector(velocity, avxyz))
                ;
            else if(item->QueryAttribute("angular_velocity", &av) == XML_SUCCESS)
                avxyz = Vector3(av, av, av);
            else
            {
                delete imu;
                return nullptr;
            }
            
            imu->setNoise(axyz, avxyz, yawDrift);
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
        
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            const char* velocity = nullptr;
            Scalar velx, vely, velz;
            Scalar altMin, altMax;
            
            if(item->QueryStringAttribute("velocity", &velocity) != XML_SUCCESS || sscanf(velocity, "%lf %lf %lf", &velx, &vely, &velz) != 3)
            {
                delete dvl;
                return nullptr;
            }
            if(item->QueryAttribute("altitude_min", &altMin) != XML_SUCCESS
               || item->QueryAttribute("altitude_max", &altMax) != XML_SUCCESS)
            {
                delete dvl;
                return nullptr;
            }
            dvl->setRange(Vector3(velx, vely, velz), altMin, altMax);
        }
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar velocity;
            Scalar altitude;
            if(item->QueryAttribute("velocity", &velocity) != XML_SUCCESS || item->QueryAttribute("altitude", &altitude) != XML_SUCCESS)
            {
                delete dvl;
                return nullptr;
            }
            dvl->setNoise(velocity, altitude);
        }
        
        return dvl;
    }
    else if(typeStr == "gps")
    {
        int history;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        GPS* gps = new GPS(sensorName, rate, history);
        
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar ned;
            if(item->QueryAttribute("ned_position", &ned) != XML_SUCCESS)
            {
                delete gps;
                return nullptr;
            }
            gps->setNoise(ned);
        }
        
        return gps;
    }
    else if(typeStr == "pressure")
    {
        int history;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        Pressure* press = new Pressure(sensorName, rate, history);
        
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            Scalar pressure;
            if(item->QueryAttribute("pressure", &pressure) != XML_SUCCESS)
            {
                delete press;
                return nullptr;
            }
            press->setRange(pressure);
        }
        
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar pressure;
            if(item->QueryAttribute("pressure", &pressure) != XML_SUCCESS)
            {
                delete press;
                return nullptr;
            }
            press->setNoise(pressure);
        }
        
        return press;
    }
    else if(typeStr == "odometry")
    {
        int history;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        Odometry* odom = new Odometry(sensorName, rate, history);
        
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar position, velocity, angle, aVelocity;
            if(item->QueryAttribute("position", &position) != XML_SUCCESS
              || item->QueryAttribute("velocity", &velocity) != XML_SUCCESS
              || item->QueryAttribute("angle", &angle) != XML_SUCCESS
              || item->QueryAttribute("angular_velocity", &aVelocity) != XML_SUCCESS)
            {
                delete odom;
                return nullptr;
            }
            odom->setNoise(position, velocity, angle, aVelocity);
        }
        
        return odom;
    }
    else if(typeStr == "compass")
    {
        int history;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        Compass* compass = new Compass(sensorName, rate, history);
        
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar heading;
            if(item->QueryAttribute("heading", &heading) != XML_SUCCESS)
            {
                delete compass;
                return nullptr;
            }
            compass->setNoise(heading);
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
            return nullptr;
            
        Profiler* prof = new Profiler(sensorName, fov, steps, rate, history);
        
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            Scalar distMin, distMax;
            
            if(item->QueryAttribute("distance_min", &distMin) != XML_SUCCESS
               || item->QueryAttribute("distance_max", &distMax) != XML_SUCCESS)
            {
                delete prof;
                return nullptr;
            }
            prof->setRange(distMin, distMax);
        }
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar distance;
            if(item->QueryAttribute("distance", &distance) != XML_SUCCESS)
            {
                delete prof;
                return nullptr;
            }
            prof->setNoise(distance);
        }
        
        return prof;
    }
    else if(typeStr == "multibeam1d")
    {
        int history;
        Scalar fov;
        int steps;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
        if((item = element->FirstChildElement("specs")) == nullptr || item->QueryAttribute("fov", &fov) != XML_SUCCESS || item->QueryAttribute("steps", &steps) != XML_SUCCESS)
            return nullptr;
            
        Multibeam* mult = new Multibeam(sensorName, fov, steps, rate, history);
        
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            Scalar distMin, distMax;
            
            if(item->QueryAttribute("distance_min", &distMin) != XML_SUCCESS
               || item->QueryAttribute("distance_max", &distMax) != XML_SUCCESS)
            {
                delete mult;
                return nullptr;
            }
            mult->setRange(distMin, distMax);
        }
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar distance;
            if(item->QueryAttribute("distance", &distance) != XML_SUCCESS)
            {
                delete mult;
                return nullptr;
            }
            mult->setNoise(distance);
        }
        
        return mult;
    }
    else if(typeStr == "torque")
    {
        int history;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        Torque* torque = new Torque(sensorName, rate, history);
        
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            Scalar tau;
            if(item->QueryAttribute("torque", &tau) != XML_SUCCESS)
            {
                delete torque;
                return nullptr;
            }
            torque->setRange(tau);
        }
        
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar tau;
            if(item->QueryAttribute("torque", &tau) != XML_SUCCESS)
            {
                delete torque;
                return nullptr;
            }
            torque->setNoise(tau);
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
        
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            const char* force = nullptr;
            const char* torque = nullptr;
            Scalar fx, fy, fz, tx, ty, tz;
            
            if(item->QueryStringAttribute("force", &force) != XML_SUCCESS || sscanf(force, "%lf %lf %lf", &fx, &fy, &fz) != 3)
            {
                delete ft;
                return nullptr;
            }
            if(item->QueryStringAttribute("torque", &torque) != XML_SUCCESS || sscanf(torque, "%lf %lf %lf", &tx, &ty, &tz) != 3)
            {
                delete ft;
                return nullptr;
            }
            ft->setRange(Vector3(fx, fy, fz), Vector3(tx, ty, tz));
        }
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar force;
            Scalar torque;
            if(item->QueryAttribute("force", &force) != XML_SUCCESS || item->QueryAttribute("torque", &torque) != XML_SUCCESS)
            {
                delete ft;
                return nullptr;
            }
            ft->setNoise(force, torque);
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
        int resX, resY;
        Scalar hFov;
        if((item = element->FirstChildElement("specs")) == nullptr 
            || item->QueryAttribute("resolution_x", &resX) != XML_SUCCESS 
            || item->QueryAttribute("resolution_y", &resY) != XML_SUCCESS
            || item->QueryAttribute("horizontal_fov", &hFov) != XML_SUCCESS)
            return nullptr;
            
        ColorCamera* cam;
        
        if((item = element->FirstChildElement("rendering")) != nullptr) //Optional parameters
        {
            Scalar minDist(0.02);
            Scalar maxDist(100000.0);
            item->QueryAttribute("minimum_distance", &minDist);
            item->QueryAttribute("maximum_distance", &maxDist);
            cam = new ColorCamera(sensorName, resX, resY, hFov, rate, minDist, maxDist);
        }
        else
        {
            cam = new ColorCamera(sensorName, resX, resY, hFov, rate);
        }
        return cam;
    }
    else if(typeStr == "depthcamera")
    {
        int resX, resY;
        Scalar hFov;
        Scalar depthMin, depthMax;
        if((item = element->FirstChildElement("specs")) == nullptr 
            || item->QueryAttribute("resolution_x", &resX) != XML_SUCCESS 
            || item->QueryAttribute("resolution_y", &resY) != XML_SUCCESS
            || item->QueryAttribute("horizontal_fov", &hFov) != XML_SUCCESS
            || item->QueryAttribute("depth_min", &depthMin) != XML_SUCCESS
            || item->QueryAttribute("depth_max", &depthMax) != XML_SUCCESS)
            return nullptr;
        
        DepthCamera* dcam = new DepthCamera(sensorName, resX, resY, hFov, depthMin, depthMax, rate);

        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            float depth;
            if(item->QueryAttribute("depth", &depth) != XML_SUCCESS)
            {
                delete dcam;
                return nullptr;
            }
            dcam->setNoise(depth);
        }
        return dcam;
    }
    else if(typeStr == "multibeam2d")
    {
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
            return nullptr;
        
        Multibeam2* mb = new Multibeam2(sensorName, resX, resY, hFov, vFov, rangeMin, rangeMax, rate);
        return mb;
    }
    else if(typeStr == "fls")
    {
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
            return nullptr;
        if((item = element->FirstChildElement("settings")) != nullptr)
        {
            item->QueryAttribute("range_min", &rangeMin);
            item->QueryAttribute("range_max", &rangeMax);
            item->QueryAttribute("gain", &gain);
        }
        if((item = element->FirstChildElement("display")) != nullptr)
            ParseColorMap(item, cMap);
        
        FLS* fls = new FLS(sensorName, nBeams, nBins, hFov, vFov, rangeMin, rangeMax, cMap, rate);
        fls->setGain(gain);

        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            float mul, add;
            if(item->QueryAttribute("multiplicative", &mul) != XML_SUCCESS || item->QueryAttribute("additive", &add) != XML_SUCCESS)
            {
                delete fls;
                return nullptr;
            }
            fls->setNoise(mul, add);
        }
        else
        {
            fls->setNoise(0.025f, 0.035f); //Default values that look realistic
        }
        return fls;
    }
    else if(typeStr == "sss")
    {
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
            return nullptr;
        if((item = element->FirstChildElement("settings")) != nullptr)
        {
            item->QueryAttribute("range_min", &rangeMin);
            item->QueryAttribute("range_max", &rangeMax);
            item->QueryAttribute("gain", &gain);
        }
        if((item = element->FirstChildElement("display")) != nullptr)
            ParseColorMap(item, cMap);
        
        SSS* sss = new SSS(sensorName, nBins, nLines, vFov, hFov, tilt, rangeMin, rangeMax, cMap, rate);
        sss->setGain(gain);

        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            float mul, add;
            if(item->QueryAttribute("multiplicative", &mul) != XML_SUCCESS || item->QueryAttribute("additive", &add) != XML_SUCCESS)
            {
                delete sss;
                return nullptr;
            }
            sss->setNoise(mul, add);
        }
        else
        {
            sss->setNoise(0.01f, 0.02f); //Default values that look realistic
        }
        return sss;
    }
    else if(typeStr == "msis")
    {
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
            return nullptr;
        if((item = element->FirstChildElement("settings")) != nullptr)
        {
            item->QueryAttribute("range_min", &rangeMin);
            item->QueryAttribute("range_max", &rangeMax);
            item->QueryAttribute("rotation_min", &rotMin);
            item->QueryAttribute("rotation_max", &rotMax);
            item->QueryAttribute("gain", &gain);
        }
        if((item = element->FirstChildElement("display")) != nullptr)
            ParseColorMap(item, cMap);
        
        MSIS* msis = new MSIS(sensorName, stepAngle, nBins, hFov, vFov, rotMin, rotMax, rangeMin, rangeMax, cMap, rate);
        msis->setGain(gain);

        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            float mul, add;
            if(item->QueryAttribute("multiplicative", &mul) != XML_SUCCESS || item->QueryAttribute("additive", &add) != XML_SUCCESS)
            {
                delete msis;
                return nullptr;
            }
            msis->setNoise(mul, add);
        }
        else
        {
            msis->setNoise(0.02f, 0.04f); //Default values that look realistic
        }
        return msis;
    }
    else
        return nullptr;
}

Light* ScenarioParser::ParseLight(XMLElement* element, const std::string& namePrefix)
{
    const char* name = nullptr;
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
        return nullptr;

    std::string lightName = std::string(name);
    if(namePrefix != "")
        lightName = namePrefix + "/" + lightName;
    
    XMLElement* item;
    Scalar illu, radius;
	Scalar cone = Scalar(0);
    Color color = Color::Gray(1.f);
    
    if((item = element->FirstChildElement("specs")) != nullptr)
    {
        if(item->QueryAttribute("illuminance", &illu) != XML_SUCCESS)
            return nullptr;
        if(item->QueryAttribute("radius", &radius) != XML_SUCCESS)
            return nullptr;
        item->QueryAttribute("cone_angle", &cone);
    } 
    else
        return nullptr;
    
    if((item = element->FirstChildElement("color")) == nullptr || !ParseColor(item, color))
        return nullptr;
        
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
        return nullptr;
    if(element->QueryStringAttribute("type", &type) != XML_SUCCESS)
        return nullptr;
    if(element->QueryAttribute("device_id", &devId) != XML_SUCCESS)
        return nullptr;
     
    std::string typeStr(type);
    std::string commName = std::string(name);
    if(namePrefix != "")
        commName = namePrefix + "/" + commName;

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
            return nullptr;
        item = element->FirstChildElement("connect");
        if(item == nullptr || item->QueryAttribute("device_id", &cId) != XML_SUCCESS || cId == 0)
            return nullptr;
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
            return nullptr;
        item = element->FirstChildElement("connect");
        if(item == nullptr || item->QueryAttribute("device_id", &cId) != XML_SUCCESS || cId == 0)
            return nullptr;
        item->QueryAttribute("occlusion_test", &occlusion);

        comm = new USBL(commName, devId, hFovDeg, vFovDeg, range);
        comm->Connect(cId);
        ((AcousticModem*)comm)->setOcclusionTest(occlusion);
        
        if((item = element->FirstChildElement("autoping")) != nullptr
            && item->QueryAttribute("rate", &pingRate) == XML_SUCCESS)
            ((USBL*)comm)->EnableAutoPing(pingRate);
        
        if((item = element->FirstChildElement("noise")) != nullptr)
        {
            Scalar rangeDev = Scalar(0);
            Scalar hAngleDevDeg = Scalar(0);
            Scalar vAngleDevDeg = Scalar(0);
            item->QueryAttribute("range", &rangeDev);
            item->QueryAttribute("horizontal_angle", &hAngleDevDeg);
            item->QueryAttribute("vertical_angle", &vAngleDevDeg);
            ((USBL*)comm)->setNoise(rangeDev, hAngleDevDeg, vAngleDevDeg);
        }

        if((item = element->FirstChildElement("resolution")) != nullptr)
        {
            Scalar rangeRes = Scalar(0);
            Scalar angleResDeg = Scalar(0);
            item->QueryAttribute("range", &rangeRes);
            item->QueryAttribute("angle", &angleResDeg);
            ((USBL*)comm)->setResolution(rangeRes, angleResDeg);
        }
        return comm;
    }
    else 
    {
        cError("Scenario parser: unknown communication device type!");
        return nullptr;
    }
}

bool ScenarioParser::ParseContact(XMLElement* element)
{
    const char* name = nullptr;
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
        return false;
        
    XMLElement* itemA;
    XMLElement* itemB;
    if((itemA = element->FirstChildElement("bodyA")) == nullptr
        || (itemB = element->FirstChildElement("bodyB")) == nullptr)
        return false;
    
    const char* nameA = nullptr;
    const char* nameB = nullptr;
    const char* dispA = nullptr;
    const char* dispB = nullptr;
    if(itemA->QueryStringAttribute("name", &nameA) != XML_SUCCESS
        || itemB->QueryStringAttribute("name", &nameB) != XML_SUCCESS)
        return false;
    
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
        return false;
    
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
    
    Contact* cnt = new Contact(name, entA, entB, history);
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
        //Error handling required (e.g. throw)
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
    if(sscanf(components, "%lf %lf %lf", &x, &y, &z) != 3) return false;
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
    
    if(element->QueryStringAttribute("xyz", &trans) != XML_SUCCESS)
        return false;
    if(element->QueryStringAttribute("rpy", &rot) != XML_SUCCESS)
        return false;
    if(!ParseVector(trans, xyz))
        return false;
    if(!ParseVector(rot, rpy))
        return false;
        
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
        return false;
        
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
            return false;

        return true;
    }
    else 
        return false;
}

}
