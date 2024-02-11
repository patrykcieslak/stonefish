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
//  ScenarioParser.h
//  Stonefish
//
//  Created by Patryk Cieslak on 17/07/19.
//  Copyright (c) 2019-2023 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ScenarioParser__
#define __Stonefish_ScenarioParser__

#include "StonefishCommon.h"
#include "core/Console.h"
#include "tinyxml2.h"
#include <map>

using namespace tinyxml2;

namespace sf
{
    class SimulationManager;
    class Robot;
    class Entity;
    class SolidEntity;
    class Sensor;
    class Actuator;
    class Light;
    class Comm;
    class VelocityField;
    struct Color;
    enum class ColorMap;
  
    //! A class that implements parsing of XML files describing a simulation scenario.
    class ScenarioParser
    {
    public:
        //! A constructor.
        /*!
         \param sm a pointer to the simulation manager
         */
        ScenarioParser(SimulationManager* sm);
        
        //! A method used to parse a scenario description file.
        /*!
         \param filename path to the scenario description file
         \return success
         */
        virtual bool Parse(std::string filename);

        //! A method saving the log to a text file.
        /*!
         \param filename path to the log file
         \return success
         */
        bool SaveLog(std::string filename);

        //! A method returning a copy of the log.
        std::vector<ConsoleMessage> getLog();

        //! A method used to get the pointer to the associated simulation manager.
        SimulationManager* getSimulationManager();

    protected:
        Console log;

        //! A method used to pre-process the xml description file after loading.
        /*!
         \param root a pointer to a root node
         \param args a map of arguments and their values
         \return success
         */
        virtual bool PreProcess(XMLNode* root, 
                                const std::map<std::string, std::string>& args = std::map<std::string, std::string>());

        //! A method used to replace the arguments passed to the include files.
        /*!
         \param node a pointer to a node
         \param args a map of arguments and their values
         \return success
         */
        virtual bool ReplaceArguments(XMLNode* node, const std::map<std::string, std::string>& args);

        //! A method used to evaluate mathematical expressions.
        /*!
         \param node a pointer to a node
         \return success
         */
        virtual bool EvaluateMath(XMLNode* node);

        //! A method used to parse solver configuration.
        /*!
         \param element a pointer to the XML node
         \return success
         */
        virtual bool ParseSolver(XMLElement* element);

        //! A method used to parse environment configuration.
        /*!
         \param element a pointer to the XML node
         \return success
         */
        virtual bool ParseEnvironment(XMLElement* element);
        
        //! A method used to parse the physical materials information.
        /*!
         \param element a pointer to the XML node
         \return success
         */
        virtual bool ParseMaterials(XMLElement* element);
        
        //! A method used to parse the graphical materials information.
        /*!
         \param element a pointer to the XML node
         \return success
         */
        virtual bool ParseLooks(XMLElement* element);
        
        //! A method used to parse a definition of a velocity field (current, wind).
        /*!
         \param element a pointer to the XML node
         \return a pointer to the new velocity field object
         */
        virtual VelocityField* ParseVelocityField(XMLElement* element);

        //! A method used to parse a static object description.
        /*!
         \param element a pointer to the XML node
         \return success
         */
        virtual bool ParseStatic(XMLElement* element);

        //! A method used to parse an animated object description.
        /*!
         \param element a pointer to the XML node
         \return success
         */
        virtual bool ParseAnimated(XMLElement* element);

        //! A method used to parse a dynamic object description.
        /*!
         \param element a pointer to the XML node
         \return success
         */
        virtual bool ParseDynamic(XMLElement* element);

        //! A method used to parse a dynamic object description.
        /*!
         \param element a pointer to the XML node
         \param solid a reference to the loaded solid entity
         \param ns an optional namespace
         \param compoundPart is this solid a part of a compound body?
         \return success
         */
        virtual bool ParseSolid(XMLElement* element, SolidEntity*& solid, 
                                        std::string ns = "", bool compoundPart = false);

        //! A method used to parse a robot description.
        /*!
         \param element a pointer to the XML node
         \return success
         */
        virtual bool ParseRobot(XMLElement* element);
        
        //! A method used to parse a single robot link description.
        /*!
         \param element a pointer to the XML node
         \param robot a pointer to the robot object
         \param link a reference to the loaded robot link
         \return success
         */
         
         
         
        virtual bool ParseLink(XMLElement* element, Robot* robot, SolidEntity*& link);
        
        //! A method used to parse a single robot joint description.
        /*!
         \param element a pointer to the XML node
         \param robot a pointer to the robot object
         \return success
         */
        virtual bool ParseJoint(XMLElement* element, Robot* robot);
        
        //! A method used to parse a single robot actuator description.
        /*!
         \param element a pointer to the XML node
         \param robot a pointer to the robot object
         \return success
         */
        virtual bool ParseActuator(XMLElement* element, Robot* robot);
        
        //! A method used to parse a single robot sensor description.
        /*!
         \param element a pointer to the XML node
         \param robot a pointer to the robot object
         \return success
         */
        virtual bool ParseSensor(XMLElement* element, Robot* robot);
        
        //! A method used to parse a description of a sensor attached to single body or world.
        /*!
         \param element a pointer to the XML node
         \param ent a pointer to an entity (static, solid, animated)
         \return success
         */
        virtual bool ParseSensor(XMLElement* element, Entity* ent = nullptr);
        
        //! A method used to parse a description of an actuator.
        /*!
         \param element a pointer to the XML node
         \param namePrefix a string added at the beginning of the actuator name
         \return pointer to actuator
         */
        virtual Actuator* ParseActuator(XMLElement* element, const std::string& namePrefix);

        //! A method used to parse a description of a sensor.
        /*!
         \param element a pointer to the XML node
         \param namePrefix a string added at the beginning of the sensor name
         \return pointer to sensor
         */
        virtual Sensor* ParseSensor(XMLElement* element, const std::string& namePrefix);
        
        //! A method used to parse a description of a light source.
        /*!
         \param element a pointer to the XML node
         \param namePrefix a string added at the beginning of the light name
         \return pointer to light
         */
        virtual Light* ParseLight(XMLElement* element, const std::string& namePrefix);

        //! A method used to parse a communication device description.
        /*!
         \param element a pointer to the XML node
         \param namePrefix a string added at the beginning of the comm name
         \return pointer to the communication device
         */
        virtual Comm* ParseComm(XMLElement* element, const std::string& namePrefix);
        
        //! A method used to parse a single contact description.
        /*!
         \param element a pointer to the XML node
         \return success
         */
        virtual bool ParseContact(XMLElement* element);

        //! A method used to parse a single glue joint description.
        /*!
         \param element a pointer to the XML node
         \return success
        */
        virtual bool ParseGlue(XMLElement* element);
        virtual bool ParseTether(XMLElement* element);
        //! A method to get the full file path depending on the format of the passed string.
        /*!
         \param path a file path candidate
         \return full file path
         */
        std::string GetFullPath(const std::string& path);

        //! A method informing if the simulation is working in graphical mode.
        bool isGraphicalSim();

    private:
        bool CopyNode(XMLNode* destParent, const XMLNode* src);
        bool ParseVector(const char* components, Vector3& v);
        bool ParseTransform(XMLElement* element, Transform& T);
        bool ParseColor(XMLElement* element, Color& c);
        bool ParseColorMap(XMLElement* element, ColorMap& cm);
        XMLDocument doc;
        SimulationManager* sm;
        bool graphical;
        
    };
}

#endif
