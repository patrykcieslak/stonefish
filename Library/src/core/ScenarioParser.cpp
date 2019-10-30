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
#include "actuators/Light.h"
#include "actuators/Servo.h"
#include "actuators/Propeller.h"
#include "actuators/Thruster.h"
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
    XMLElement* root = doc.FirstChildElement("scenario");
    if(root == nullptr)
    {
        cError("Scenario parser: root node not found!");
        return false;
    }

    //Include other scenario files
    XMLElement* element = root->FirstChildElement("include");
    while(element != nullptr)
    {
        const char* path = nullptr;
        if(element->QueryStringAttribute("file", &path) != XML_SUCCESS)
        {
            cError("Scenario parser: include not properly defined!");
            return false;
        }
        
        XMLDocument includedDoc;
        if(includedDoc.LoadFile(GetFullPath(std::string(path)).c_str()) != XML_SUCCESS)
        {
            cError("Scenario parser: included file not found! '%s'", GetFullPath(std::string(path)).c_str());
            return false;
        }
        
        root->DeleteChild(element); //Delete "include" element
        
        XMLNode* includedRoot = includedDoc.FirstChildElement("scenario");
        if(includedRoot == nullptr)
        {
            cError("Scenario parser: root node not found in included file!");
            return false;
        }
        
        for (const XMLNode* child = includedRoot->FirstChild(); child != nullptr; child = child->NextSibling())
        {
            if (!CopyNode(root, child))
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
        Color color = Color::Gray(1.f);
        Scalar roughness;
        Scalar metalness;
        Scalar reflectivity;
        const char* texture = nullptr;
        std::string textureStr = "";
        
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
            textureStr = GetDataPath() + std::string(texture);
        sm->CreateLook(name, color, roughness, metalness, reflectivity, textureStr);
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
            
        object = new Terrain(std::string(name), GetFullPath(std::string(heightmap)), scaleX, scaleY, height, std::string(mat), std::string(look));
    }
    else
        return false;
        
    sm->AddStaticEntity(object, trans);
    return true;
}

bool ScenarioParser::ParseSolid(XMLElement* element, SolidEntity*& solid, std::string ns)
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
        ePhyType = BodyPhysicsType::SUBMERGED_BODY;
    }
    else
    {
        std::string phyTypeStr(phyType);
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
    }
    if(element->QueryAttribute("buoyant", &buoyant) != XML_SUCCESS)
        buoyant = true;
    
    std::string typeStr(type);
    std::string solidName = ns != "" ? ns + "/" + std::string(name) : std::string(name);
    
    if(typeStr == "compound")
    {
        XMLElement* item;
        
        //First external part
        SolidEntity* part = nullptr;
        Compound* comp = nullptr;
        Transform partOrigin;
      
        if((item = element->FirstChildElement("external_part")) == nullptr)
            return false;
        if(!ParseSolid(item, part, solidName))
            return false;
        XMLElement* item2;
        if((item2 = item->FirstChildElement("compound_transform")) == nullptr || !ParseTransform(item2, partOrigin))
            return false;
        comp = new Compound(solidName, part, partOrigin, ePhyType);
        
        //Iterate through all external parts
        item = item->NextSiblingElement("external_part");
        while(item != nullptr)
        {
            if(!ParseSolid(item, part, solidName))
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
            if(!ParseSolid(item, part, solidName))
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
        XMLElement* item;
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
        return false;
    }
    if(!ParseLink(item, robot, baseLink))
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
        if(!ParseLink(item, robot, link))
        {
            cError("Scenario parser: link of robot '%s' not properly defined!", name);
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
            cError("Scenario parser: sensor of robot '%s' not properly defined!", name);
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
            cError("Scenario parser: actuator of robot '%s' not properly defined!", name);
            return false;
        }
        item = item->NextSiblingElement("actuator");
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
    //---- Common ----
    const char* name = nullptr;
    const char* type = nullptr;
    Scalar rate;
    
    if(element->QueryStringAttribute("name", &name) != XML_SUCCESS)
        return false;
    if(element->QueryStringAttribute("type", &type) != XML_SUCCESS)
        return false;
    if(element->QueryAttribute("rate", &rate) != XML_SUCCESS)
        rate = Scalar(-1);
    std::string typeStr(type);
    
    std::string sensorName = robot->getName() + "/" + std::string(name);
    
    //---- Specific ----
    XMLElement* item;
    if(typeStr == "imu")
    {
        const char* linkName = nullptr;
        Transform origin;
        int history;
        
        if((item = element->FirstChildElement("link")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        IMU* imu = new IMU(sensorName, rate, history);
        
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            Scalar velocity;
            if(item->QueryAttribute("angular_velocity", &velocity) != XML_SUCCESS)
            {
                delete imu;
                return false;
            }
            imu->setRange(velocity);
        }
        
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar angle;
            Scalar velocity;
            if(item->QueryAttribute("angle", &angle) != XML_SUCCESS || item->QueryAttribute("angular_velocity", &velocity) != XML_SUCCESS)
            {
                delete imu;
                return false;
            }
            imu->setNoise(angle, velocity);
        }
        
        robot->AddLinkSensor(imu, robot->getName() + "/" + std::string(linkName), origin);
    }
    else if(typeStr == "dvl")
    {
        const char* linkName = nullptr;
        Transform origin;
        int history;
        Scalar beamAngle;
        
        if((item = element->FirstChildElement("link")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
        if((item = element->FirstChildElement("specs")) == nullptr || item->QueryAttribute("beam_angle", &beamAngle) != XML_SUCCESS)
            return false;
            
        DVL* dvl = new DVL(sensorName, beamAngle, rate, history);
        
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            const char* velocity = nullptr;
            Scalar velx, vely, velz;
            Scalar altMin, altMax;
            
            if(item->QueryStringAttribute("velocity", &velocity) != XML_SUCCESS || sscanf(velocity, "%lf %lf %lf", &velx, &vely, &velz) != 3)
            {
                delete dvl;
                return false;
            }
            if(item->QueryAttribute("altitude_min", &altMin) != XML_SUCCESS
               || item->QueryAttribute("altitude_max", &altMax) != XML_SUCCESS)
            {
                delete dvl;
                return false;
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
                return false;
            }
            dvl->setNoise(velocity, altitude);
        }
        
        robot->AddLinkSensor(dvl, robot->getName() + "/" + std::string(linkName), origin);
    }
    else if(typeStr == "gps")
    {
        const char* linkName = nullptr;
        Transform origin;
        int history;
        
        if((item = element->FirstChildElement("link")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        GPS* gps = new GPS(sensorName, rate, history);
        
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar ned;
            if(item->QueryAttribute("ned_position", &ned) != XML_SUCCESS)
            {
                delete gps;
                return false;
            }
            gps->setNoise(ned);
        }
        
        robot->AddLinkSensor(gps, robot->getName() + "/" + std::string(linkName), origin);
    }
    else if(typeStr == "pressure")
    {
        const char* linkName = nullptr;
        Transform origin;
        int history;
        
        if((item = element->FirstChildElement("link")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        Pressure* press = new Pressure(sensorName, rate, history);
        
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            Scalar pressure;
            if(item->QueryAttribute("pressure", &pressure) != XML_SUCCESS)
            {
                delete press;
                return false;
            }
            press->setRange(pressure);
        }
        
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar pressure;
            if(item->QueryAttribute("pressure", &pressure) != XML_SUCCESS)
            {
                delete press;
                return false;
            }
            press->setNoise(pressure);
        }
        
        robot->AddLinkSensor(press, robot->getName() + "/" + std::string(linkName), origin);
    }
    else if(typeStr == "odometry")
    {
        const char* linkName = nullptr;
        Transform origin;
        int history;
        
        if((item = element->FirstChildElement("link")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;
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
                return false;
            }
            odom->setNoise(position, velocity, angle, aVelocity);
        }
        
        robot->AddLinkSensor(odom, robot->getName() + "/" + std::string(linkName), origin);
    }
    else if(typeStr == "compass")
    {
        const char* linkName = nullptr;
        Transform origin;
        int history;
        
        if((item = element->FirstChildElement("link")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        Compass* compass = new Compass(sensorName, rate, history);
        
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar heading;
            if(item->QueryAttribute("heading", &heading) != XML_SUCCESS)
            {
                delete compass;
                return false;
            }
            compass->setNoise(heading);
        }
        
        robot->AddLinkSensor(compass, robot->getName() + "/" + std::string(linkName), origin);
    }
    else if(typeStr == "profiler")
    {
        const char* linkName = nullptr;
        Transform origin;
        int history;
        Scalar fov;
        int steps;
        
        if((item = element->FirstChildElement("link")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
        if((item = element->FirstChildElement("specs")) == nullptr || item->QueryAttribute("fov", &fov) != XML_SUCCESS || item->QueryAttribute("steps", &steps) != XML_SUCCESS)
            return false;
            
        Profiler* prof = new Profiler(sensorName, fov, steps, rate, history);
        
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            Scalar distMin, distMax;
            
            if(item->QueryAttribute("distance_min", &distMin) != XML_SUCCESS
               || item->QueryAttribute("distance_max", &distMax) != XML_SUCCESS)
            {
                delete prof;
                return false;
            }
            prof->setRange(distMin, distMax);
        }
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar distance;
            if(item->QueryAttribute("distance", &distance) != XML_SUCCESS)
            {
                delete prof;
                return false;
            }
            prof->setNoise(distance);
        }
        
        robot->AddLinkSensor(prof, robot->getName() + "/" + std::string(linkName), origin);
    }
    else if(typeStr == "multibeam1d")
    {
        const char* linkName = nullptr;
        Transform origin;
        int history;
        Scalar fov;
        int steps;
        
        if((item = element->FirstChildElement("link")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
        if((item = element->FirstChildElement("specs")) == nullptr || item->QueryAttribute("fov", &fov) != XML_SUCCESS || item->QueryAttribute("steps", &steps) != XML_SUCCESS)
            return false;
            
        Multibeam* mult = new Multibeam(sensorName, fov, steps, rate, history);
        
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            Scalar distMin, distMax;
            
            if(item->QueryAttribute("distance_min", &distMin) != XML_SUCCESS
               || item->QueryAttribute("distance_max", &distMax) != XML_SUCCESS)
            {
                delete mult;
                return false;
            }
            mult->setRange(distMin, distMax);
        }
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar distance;
            if(item->QueryAttribute("distance", &distance) != XML_SUCCESS)
            {
                delete mult;
                return false;
            }
            mult->setNoise(distance);
        }
        
        robot->AddLinkSensor(mult, robot->getName() + "/" + std::string(linkName), origin);
    }
    else if(typeStr == "torque")
    {
        const char* jointName = nullptr;
        int history;
        
        if((item = element->FirstChildElement("joint")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &jointName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        Torque* torque = new Torque(sensorName, rate, history);
        
        if((item = element->FirstChildElement("range")) != nullptr)    
        {
            Scalar tau;
            if(item->QueryAttribute("torque", &tau) != XML_SUCCESS)
            {
                delete torque;
                return false;
            }
            torque->setRange(tau);
        }
        
        if((item = element->FirstChildElement("noise")) != nullptr)    
        {
            Scalar tau;
            if(item->QueryAttribute("torque", &tau) != XML_SUCCESS)
            {
                delete torque;
                return false;
            }
            torque->setNoise(tau);
        }
        
        robot->AddJointSensor(torque, robot->getName() + "/" + std::string(jointName));
    }
    else if(typeStr == "forcetorque")
    {
        const char* jointName = nullptr;
        Transform origin;
        int history;
        
        if((item = element->FirstChildElement("joint")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &jointName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;
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
                return false;
            }
            if(item->QueryStringAttribute("torque", &torque) != XML_SUCCESS || sscanf(torque, "%lf %lf %lf", &tx, &ty, &tz) != 3)
            {
                delete ft;
                return false;
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
                return false;
            }
            ft->setNoise(force, torque);
        }
        
        robot->AddJointSensor(ft, robot->getName() + "/" + std::string(jointName));    
    }
    else if(typeStr == "encoder")
    {
        const char* jointName = nullptr;
        int history;
        
        if((item = element->FirstChildElement("joint")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &jointName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("history")) == nullptr || item->QueryAttribute("samples", &history) != XML_SUCCESS)
            history = -1;
            
        RotaryEncoder* enc = new RotaryEncoder(sensorName, rate, history);
        robot->AddJointSensor(enc, robot->getName() + "/" + std::string(jointName));
    }
    else if(typeStr == "camera")
    {
        const char* linkName = nullptr;
        Transform origin;
        int resX, resY;
        Scalar hFov;
        
        if((item = element->FirstChildElement("link")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;
        if((item = element->FirstChildElement("specs")) == nullptr 
            || item->QueryAttribute("resolution_x", &resX) != XML_SUCCESS 
            || item->QueryAttribute("resolution_y", &resY) != XML_SUCCESS
            || item->QueryAttribute("horizontal_fov", &hFov) != XML_SUCCESS)
            return false;
            
        ColorCamera* cam;
        
        if((item = element->FirstChildElement("rendering")) != nullptr) //Optional parameters
        {
            int spp = 1;
            Scalar minDist(0.1);
            Scalar maxDist(1000.0);
            item->QueryAttribute("spp", &spp);
            item->QueryAttribute("minimum_distance", &minDist);
            item->QueryAttribute("maximum_distance", &maxDist);
            cam = new ColorCamera(sensorName, resX, resY, hFov, rate, spp, minDist, maxDist);
        }
        else
        {
            cam = new ColorCamera(sensorName, resX, resY, hFov, rate);
        }
        robot->AddVisionSensor(cam, robot->getName() + "/" + std::string(linkName), origin);
    }
    else if(typeStr == "depthcamera")
    {
        const char* linkName = nullptr;
        Transform origin;
        int resX, resY;
        Scalar hFov;
        Scalar depthMin, depthMax;
        
        if((item = element->FirstChildElement("link")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;
        if((item = element->FirstChildElement("specs")) == nullptr 
            || item->QueryAttribute("resolution_x", &resX) != XML_SUCCESS 
            || item->QueryAttribute("resolution_y", &resY) != XML_SUCCESS
            || item->QueryAttribute("horizonal_fov", &hFov) != XML_SUCCESS
            || item->QueryAttribute("depth_min", &depthMin) != XML_SUCCESS
            || item->QueryAttribute("depth_max", &depthMax) != XML_SUCCESS)
            return false;
        
        DepthCamera* dcam = new DepthCamera(sensorName, resX, resY, hFov, depthMin, depthMax, rate);
        robot->AddVisionSensor(dcam, robot->getName() + "/" + std::string(linkName), origin);
    }
    else if(typeStr == "multibeam2d")
    {
        const char* linkName = nullptr;
        Transform origin;
        int resX, resY;
        Scalar hFov, vFov;
        Scalar depthMin, depthMax;
        
        if((item = element->FirstChildElement("link")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;
        if((item = element->FirstChildElement("specs")) == nullptr 
            || item->QueryAttribute("resolution_x", &resX) != XML_SUCCESS 
            || item->QueryAttribute("resolution_y", &resY) != XML_SUCCESS
            || item->QueryAttribute("fov_x", &hFov) != XML_SUCCESS
            || item->QueryAttribute("fov_y", &vFov) != XML_SUCCESS
            || item->QueryAttribute("depth_min", &depthMin) != XML_SUCCESS
            || item->QueryAttribute("depth_max", &depthMax) != XML_SUCCESS)
            return false;
        
        Multibeam2* mb = new Multibeam2(sensorName, resX, resY, hFov, vFov, depthMin, depthMax, rate);
        robot->AddVisionSensor(mb, robot->getName() + "/" + std::string(linkName), origin);
        
    }
    else
        return false;
    
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
        Scalar diameter, cThrust, cTorque, maxRpm, propScale;
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
            Polyhedron* prop = new Polyhedron(actuatorName + "/Propeller", GetDataPath() + std::string(propFile), propScale, I4(), std::string(mat), BodyPhysicsType::SUBMERGED_BODY, std::string(look));
            Thruster* th = new Thruster(actuatorName, prop, diameter, cThrust, cTorque, maxRpm, rightHand, inverted);
            robot->AddLinkActuator(th, robot->getName() + "/" + std::string(linkName), origin);
        }
        else //propeller
        {
            Polyhedron* prop = new Polyhedron(actuatorName + "/Propeller", GetDataPath() + std::string(propFile), propScale, I4(), std::string(mat), BodyPhysicsType::AERODYNAMIC_BODY, std::string(look));
            Propeller* p = new Propeller(actuatorName, prop, diameter, cThrust, cTorque, maxRpm, rightHand, inverted);
            robot->AddLinkActuator(p, robot->getName() + "/" + std::string(linkName), origin);
        }
    }
    else if(typeStr == "light")
    {
        const char* linkName = nullptr;
        Scalar illu, cone;
        Color color = Color::Gray(1.f);
        Transform origin;
        
        if((item = element->FirstChildElement("link")) == nullptr)
            return false;
        if(item->QueryStringAttribute("name", &linkName) != XML_SUCCESS)
            return false;
        if((item = element->FirstChildElement("origin")) == nullptr || !ParseTransform(item, origin))
            return false;
        if((item = element->FirstChildElement("specs")) != nullptr)
        {
            if(item->QueryAttribute("illuminance", &illu) != XML_SUCCESS)
                return false;
            if(item->QueryAttribute("cone_angle", &cone) != XML_SUCCESS)
                cone = Scalar(-1);
        } 
        else
            return false;
        if((item = element->FirstChildElement("color")) == nullptr || !ParseColor(item, color))
            return false;
            
        Light* light;
        if(cone > Scalar(0))
            light = new Light(actuatorName, cone, color, illu);
        else
            light = new Light(actuatorName, color, illu);
            
        robot->AddLinkActuator(light, robot->getName() + "/" + std::string(linkName), origin);
    }
    else
        return false;
    
    return true;
}

//Private
std::string ScenarioParser::GetFullPath(const std::string& path)
{
    if(path.at(0) == '/') //Absolute path?
        return path;
    else
        return GetDataPath() + path;
}

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

}
