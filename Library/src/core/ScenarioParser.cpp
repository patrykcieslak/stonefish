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
//  Copyright (c) 2019 Patryk Cieslak. All rights reserved.
//

#include "core/ScenarioParser.h"
#include "core/Console.h"
#include "core/SimulationManager.h"
#include "core/NED.h"
#include "core/Robot.h"
#include "entities/statics/Obstacle.h"
#include "entities/statics/Plane.h"
#include "entities/statics/Terrain.h"
#include "entities/solids/Box.h"
#include "entities/solids/Cylinder.h"
#include "entities/solids/Sphere.h"
#include "entities/solids/Torus.h"
#include "entities/solids/Wing.h"
#include "entities/solids/Polyhedron.h"
#include "entities/solids/Compound.h"

namespace sf
{

ScenarioParser::ScenarioParser(SimulationManager* sm) : sm(sm) 
{
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
    XMLElement* root = doc.FirstChildElement("scenario");
    if(root == nullptr)
    {
        cError("Scenario parser: root node not found!");
        return false;
    }
    
    //Load environment settings
    XMLElement* element = root->FirstChildElement("environment");
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
    if(element != nullptr)
    {
        if(!ParseLooks(element))
        {
            cError("Scenario parser: looks not properly defined!");
            return false;
        }
    }
    else
        cInfo("Scenario parser: looks not defined -> using standard look.");
    
    //Load static objects (optional)
    element = root->FirstChildElement("static");
    while(element != nullptr)
    {
        if(!ParseStatic(element))
        {
            cError("Scenario parser: static objects not properly defined!");
            return false;
        }
        element = element->NextSiblingElement("static");
    }
    
    //Load dynamic objects (optional)
    element = root->FirstChildElement("dynamic");
    while(element != nullptr)
    {
        SolidEntity* solid;
        if(!ParseSolid(element, solid))
        {
            cError("Scenario parser: dynamic objects not properly defined!");
            return false;
        }
        XMLElement* item;
        Transform trans;
        if((item = element->FirstChildElement("world_transform")) == nullptr || !ParseTransform(item, trans))
        {
            cError("Scenario parser: dynamic objects not properly defined!");
            return false;
        }
        sm->AddSolidEntity(solid, trans);
        element = element->NextSiblingElement("dynamic");
    }
    
    //Load robots (optional)
    element = root->FirstChildElement("robot");
    while(element != nullptr)
    {
        if(!ParseRobot(element))
        {
            cError("Scenario parser: robots not properly defined!");
            return false;
        }
        element = element->NextSiblingElement("robot");
    }
    
    return true;
}

bool ScenarioParser::ParseEnvironment(XMLElement* element)
{
    //Get setting nodes
    XMLElement* ned = element->FirstChildElement("ned");
    XMLElement* sun = element->FirstChildElement("sun");
    XMLElement* ocean = element->FirstChildElement("ocean");
    
    if(ned == nullptr || sun == nullptr || ocean == nullptr)
        return false;
    
    //Setup NED home
    Scalar lat, lon;
    if(ned->QueryAttribute("latitude", &lat) != XML_SUCCESS)
        return false;
    if(ned->QueryAttribute("longitude", &lon) != XML_SUCCESS)
        return false;
    sm->getNED()->Init(lat, lon, Scalar(0));
    
    //Setup sun position
    Scalar az, elev;
    if(sun->QueryAttribute("azimuth", &az) != XML_SUCCESS)
        return false;
    if(sun->QueryAttribute("elevation", &elev) != XML_SUCCESS)
        return false;
    sm->getAtmosphere()->SetupSunPosition(az, elev);
    
    //Setup ocean
    bool oceanEnabled;
    Scalar oceanWaves;
    if(ocean->QueryAttribute("enabled", &oceanEnabled) != XML_SUCCESS)
        return false;
    if(oceanEnabled && ocean->QueryAttribute("waves", &oceanWaves) != XML_SUCCESS)
        return false;
    if(oceanEnabled)
        sm->EnableOcean(oceanWaves);
    
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
        const char* color = nullptr;
        float R, G, B;
        Scalar roughness;
        Scalar metalness;
        Scalar reflectivity;
        const char* texture = nullptr;
        if(look->QueryStringAttribute("name", &name) != XML_SUCCESS)
            return false;
        if(look->QueryStringAttribute("color", &color) != XML_SUCCESS)
            return false;
        if(sscanf(color, "%f %f %f", &R, &G, &B) != 3)
            return false;
        if(look->QueryAttribute("roughness", &roughness) != XML_SUCCESS)
            return false;
        if(look->QueryAttribute("metalness", &metalness) != XML_SUCCESS)
            metalness = Scalar(0);
        if(look->QueryAttribute("reflectivity", &reflectivity) != XML_SUCCESS)
            reflectivity = Scalar(0);
        if(look->QueryStringAttribute("texture", &texture) != XML_SUCCESS)
            texture = "";
        sm->CreateLook(name, Color::RGB(R, G, B), roughness, metalness, reflectivity, texture);
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
            
        object = new Obstacle(std::string(name), Vector3(dimX, dimY, dimZ), std::string(mat), std::string(look));
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
        
        if((item = element->FirstChildElement("physics")) == nullptr)
            return false;
        if((item = item->FirstChildElement("mesh")) == nullptr)
            return false;
        if(item->QueryStringAttribute("filename", &phyMesh) != XML_SUCCESS)
            return false;
        if((item = item->NextSiblingElement("scale")) == nullptr)
            return false;
        if(item->QueryAttribute("value", &phyScale) != XML_SUCCESS)
            return false;
        if((item = item->NextSiblingElement("origin")) == nullptr || !ParseTransform(item, phyOrigin))
            return false;
        
        if((item = element->FirstChildElement("graphics")) != nullptr)
        {
            const char* graMesh = nullptr;
            Scalar graScale;
            Transform graOrigin;
            
            if((item = item->FirstChildElement("mesh")) == nullptr)
                return false;
            if(item->QueryStringAttribute("filename", &graMesh) != XML_SUCCESS)
                return false;
            if((item = item->NextSiblingElement("scale")) == nullptr)
                return false;
            if(item->QueryAttribute("value", &graScale) != XML_SUCCESS)
                return false;
            if((item = item->NextSiblingElement("origin")) == nullptr || !ParseTransform(item, graOrigin))
                return false;
          
            object = new Obstacle(std::string(name), std::string(graMesh), graScale, graOrigin, std::string(phyMesh), phyScale, phyOrigin, std::string(mat), std::string(look));
        }
        else
        {
            object = new Obstacle(std::string(name), std::string(phyMesh), phyScale, phyOrigin, std::string(mat), std::string(look));
        }
    }
    else if(typestr == "plane")
    {
        object = new Plane(std::string(name), Scalar(10000), std::string(mat), std::string(look));
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
            
        object = new Terrain(std::string(name), std::string(heightmap), scaleX, scaleY, height, std::string(mat), std::string(look));
    }
    else
        return false;
        
    sm->AddStaticEntity(object, trans);
    return true;
}

bool ScenarioParser::ParseSolid(XMLElement* element, SolidEntity*& solid)
{
    //---- Basic ----
    const char* name = nullptr;
    const char* type = nullptr;
    const char* phyType = nullptr;
    bool buoyant;
    
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
        return false;
    if(element->QueryStringAttribute("type", &type) != XML_SUCCESS)
        return false;
    if(element->QueryStringAttribute("physics", &phyType) != XML_SUCCESS)
        return false;
    if(element->QueryAttribute("buoyant", &buoyant) != XML_SUCCESS)
        buoyant = true;
        
    std::string typeStr(type);
    std::string phyTypeStr(phyType);
    
    BodyPhysicsType ePhyType;
    if(phyTypeStr == "surface")
        ePhyType = BodyPhysicsType::SURFACE_BODY;
    else if(phyTypeStr == "floating")
        ePhyType = BodyPhysicsType::FLOATING_BODY;
    else if(phyTypeStr == "submerged")
        ePhyType = BodyPhysicsType::SUBMERGED_BODY;
    else if(phyTypeStr == "aerodynamic")
        ePhyType = BodyPhysicsType::AERODYNAMIC_BODY;
    else 
        return false;
        
    if(typeStr == "compound")
    {
        
        
        
        
        
        
        
        
        
    }
    else
    {
        //---- Common ----
        XMLElement* item;
        const char* mat = nullptr;
        const char* look = nullptr;
        Transform origin;
        
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
            
            solid = new Box(std::string(name), Vector3(dimX, dimY, dimZ), origin, std::string(mat), ePhyType, std::string(look), thickness, buoyant);
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
                
            solid = new Cylinder(std::string(name), radius, height, origin, std::string(mat), ePhyType, std::string(look), thickness, buoyant);
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
            
            solid = new Sphere(std::string(name), radius, origin, std::string(mat), ePhyType, std::string(look), thickness, buoyant);
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
            
            solid = new Torus(std::string(name), radiusMaj, radiusMin, origin, std::string(mat), ePhyType, std::string(look), thickness, buoyant);
        }
        else if(typeStr == "model")
        {
            const char* phyMesh = nullptr;
            Scalar phyScale;
            Transform phyOrigin;
            Scalar thickness;
        
            if((item = element->FirstChildElement("physics")) == nullptr)
                return false;
            if((item = item->FirstChildElement("mesh")) == nullptr)
                return false;
            if(item->QueryStringAttribute("filename", &phyMesh) != XML_SUCCESS)
                return false;
            if((item = item->NextSiblingElement("scale")) == nullptr)
                return false;
            if(item->QueryAttribute("value", &phyScale) != XML_SUCCESS)
                return false;
            if((item = item->NextSiblingElement("thickness")) != nullptr)
            {
                if(item->QueryAttribute("value", &thickness) != XML_SUCCESS)
                    thickness = Scalar(-1);
            }
            else
                thickness = Scalar(-1);
            if((item = item->NextSiblingElement("origin")) == nullptr || !ParseTransform(item, phyOrigin))
                return false;
        
            if((item = element->FirstChildElement("graphics")) != nullptr)
            {
                const char* graMesh = nullptr;
                Scalar graScale;
                Transform graOrigin;
            
                if((item = item->FirstChildElement("mesh")) == nullptr)
                    return false;
                if(item->QueryStringAttribute("filename", &graMesh) != XML_SUCCESS)
                    return false;
                if((item = item->NextSiblingElement("scale")) == nullptr)
                    return false;
                if(item->QueryAttribute("value", &graScale) != XML_SUCCESS)
                    return false;
                if((item = item->NextSiblingElement("origin")) == nullptr || !ParseTransform(item, graOrigin))
                    return false;
          
                solid = new Polyhedron(std::string(name), std::string(graMesh), graScale, graOrigin, std::string(phyMesh), phyScale, phyOrigin, std::string(mat), ePhyType, std::string(look), thickness, buoyant); 
            }
            else
            {
                solid = new Polyhedron(std::string(name), std::string(phyMesh), phyScale, phyOrigin, std::string(mat), ePhyType, std::string(look), thickness, buoyant); 
            }
        }
        else
            return false;
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

    //---- Links ----
    //Base link
    SolidEntity* baseLink = nullptr;
    
    if((item = element->FirstChildElement("base_link")) == nullptr)
    {
        cError("Scenario parser: base link of robot '%s' missing!", name);
        return false;
    }
    if(!ParseLink(item, baseLink))
    {
        cError("Scenario parser: base link of robot '%s' not properly defined!", name);
        return false;
    }
    
    //Other links
    SolidEntity* link = nullptr;
    std::vector<SolidEntity*> links(0);
    
    item = element->FirstChildElement("link");
    while(item != nullptr)
    {
        if(!ParseLink(item, link))
        {
            cError("Scenario parser: link of robot '%s' not properly defined!");
            return false;
        }        
        links.push_back(link);
        item = item->NextSiblingElement("link");
    }
    
    Robot* robot = new Robot(std::string(name), fixed);
    robot->DefineLinks(baseLink, links, selfCollisions);
    
    //---- Joints ----
    item = element->FirstChildElement("joint");
    while(item != nullptr)
    {
        if(!ParseJoint(item, robot))
        {
            cError("Scenario parser: joint of robot '%s' not properly defined!");
            return false;
        }
        item = item->NextSiblingElement("joint");
    }
    
    //---- Sensors ----
    item = element->FirstChildElement("sensor");
    while(item != nullptr)
    {
        if(!ParseSensor(item, robot))
        {
            cError("Scenario parser: sensor of robot '%s' not properly defined!");
            return false;
        }
        item = item->NextSiblingElement("sensor");
    }
    
    //---- Actuators -----
    item = element->FirstChildElement("actuator");
    while(item != nullptr)
    {
        if(!ParseActuator(item, robot))
        {
            cError("Scenario parser: actuator of robot '%s' not properly defined!");
            return false;
        }
        item = item->NextSiblingElement("actuator");
    }
    
    sm->AddRobot(robot, trans);
    return true;
}

bool ScenarioParser::ParseLink(XMLElement* element, SolidEntity*& link)
{
    return ParseSolid(element, link);
}
        
bool ScenarioParser::ParseJoint(XMLElement* element, Robot* robot)
{
    return true;
}
        
bool ScenarioParser::ParseSensor(XMLElement* element, Robot* robot)
{
    return true;
}
        
bool ScenarioParser::ParseActuator(XMLElement* element, Robot* robot)
{
    return true;
}

bool ScenarioParser::ParseTransform(XMLElement* element, Transform& T)
{
    const char* trans = nullptr;
    const char* rot = nullptr;
    Scalar x, y, z, roll, pitch, yaw;
    
    if(element->QueryStringAttribute("xyz", &trans) != XML_SUCCESS)
        return false;
    if(element->QueryStringAttribute("rpy", &rot) != XML_SUCCESS)
        return false;
    if(sscanf(trans, "%lf %lf %lf", &x, &y, &z) != 3)
        return false;
    if(sscanf(rot, "%lf %lf %lf", &roll, &pitch, &yaw) != 3)
        return false;
        
    T = Transform(Quaternion(yaw, pitch, roll), Vector3(x, y, z));
    return true;
}

}