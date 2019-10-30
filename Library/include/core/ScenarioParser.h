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
//  Copyright (c) 2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ScenarioParser__
#define __Stonefish_ScenarioParser__

#include "StonefishCommon.h"
#include "utils/tinyxml2.h"

using namespace tinyxml2;

namespace sf
{
    class SimulationManager;
    class Robot;
    class SolidEntity;
    class Sensor;
    class Actuator;
    struct Color;
  
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
         */
        virtual bool Parse(std::string filename);
        
        //! A method used to get the pointer to the associated simulation manager.
        SimulationManager* getSimulationManager();
        
    protected:
        //! A method used to pre-process the xml description file after loading.
        /*!
         \param root a pointer to a root node
         */
        virtual bool PreProcess(XMLNode* root);
    
        //! A method used to parse environment configuration.
        /*!
         \param element a pointer to the XML node
         */
        virtual bool ParseEnvironment(XMLElement* element);
        
        //! A method used to parse the physical materials information.
        /*!
         \param element a pointer to the XML node
         */
        virtual bool ParseMaterials(XMLElement* element);
        
        //! A method used to parse the graphical materials information.
        /*!
         \param element a pointer to the XML node
         */
        virtual bool ParseLooks(XMLElement* element);
        
        //! A method used to parse a static object description.
        /*!
         \param element a pointer to the XML node
         */
        virtual bool ParseStatic(XMLElement* element);
        
        //! A method used to parse a dynamic object description.
        /*!
         \param element a pointer to the XML node
         \param solid a reference to the loaded solid entity
         \param ns an optional namespace
         */
        virtual bool ParseSolid(XMLElement* element, SolidEntity*& solid, std::string ns = "");
        
        //! A method used to parse a robot description.
        /*!
         \param element a pointer to the XML node
         */
        virtual bool ParseRobot(XMLElement* element);
        
        //! A method used to parse a single robot link description.
        /*!
         \param element a pointer to the XML node
         \param robot a pointer to the robot object
         \param link a reference to the loaded robot link
         */
        virtual bool ParseLink(XMLElement* element, Robot* robot, SolidEntity*& link);
        
        //! A method used to parse a single robot joint description.
        /*!
         \param element a pointer to the XML node
         \param robot a pointer to the robot object
         */
        virtual bool ParseJoint(XMLElement* element, Robot* robot);
        
        //! A method used to parse a single robot sensor description.
        /*!
         \param element a pointer to the XML node
         \param robot a pointer to the robot object
         */
        virtual bool ParseSensor(XMLElement* element, Robot* robot);
        
        //! A method used to parse a single robot actuator description.
        /*!
         \param element a pointer to the XML node
         \param robot a pointer to the robot object
         */
        virtual bool ParseActuator(XMLElement* element, Robot* robot);
        
        //! A method to get the full file path depending on the format of the passed string.
        /*!
         \param path a file path candidate
         */
        std::string GetFullPath(const std::string& path);
        
    private:
        bool CopyNode(XMLNode* destParent, const XMLNode* src);
        bool ParseTransform(XMLElement* element, Transform& T);
        bool ParseColor(XMLElement* element, Color& c);
    
        XMLDocument doc;
        SimulationManager* sm;
    };
}

#endif
