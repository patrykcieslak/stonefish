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
//  Copyright (c) 2019-2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ScenarioParser__
#define __Stonefish_ScenarioParser__

#include "StonefishCommon.h"
#include "tinyxml2.h"
#include <map>

using namespace tinyxml2;

namespace sf
{
    class SimulationManager;
    class Robot;
    class SolidEntity;
    class Sensor;
    class Actuator;
    class Comm;
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
        
        //! A method used to get the pointer to the associated simulation manager.
        SimulationManager* getSimulationManager();
        
    protected:
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
        
        //! A method used to parse a static object description.
        /*!
         \param element a pointer to the XML node
         \return success
         */
        virtual bool ParseStatic(XMLElement* element);
        
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
        
        //! A method used to parse a single robot sensor description.
        /*!
         \param element a pointer to the XML node
         \param robot a pointer to the robot object
         \return success
         */
        virtual bool ParseSensor(XMLElement* element, Robot* robot);
        
        //! A method used to parse a single robot actuator description.
        /*!
         \param element a pointer to the XML node
         \param robot a pointer to the robot object
         \return success
         */
        virtual bool ParseActuator(XMLElement* element, Robot* robot);
        
        //! A method used to parse a communication device description.
        /*!
         \param element a pointer to the XML node
         \param robot a pointer to the robot object
         \return success
         */
        virtual bool ParseComm(XMLElement* element, Robot* robot = nullptr);
        
        //! A method used to parse a single contact description.
        /*!
         \param element a pointer to the XML node
         \return success
         */
        virtual bool ParseContact(XMLElement* element);
        
        //! A method to get the full file path depending on the format of the passed string.
        /*!
         \param path a file path candidate
         \return full file path
         */
        std::string GetFullPath(const std::string& path);
        
    private:
        bool CopyNode(XMLNode* destParent, const XMLNode* src);
        bool ParseTransform(XMLElement* element, Transform& T);
        bool ParseColor(XMLElement* element, Color& c);
        bool ParseColorMap(XMLElement* element, ColorMap& cm);
    
        XMLDocument doc;
        SimulationManager* sm;
    };
}

#endif
