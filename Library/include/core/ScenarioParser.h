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
  
    //! A class that implements parsing of XML files describing a simulation scenario.
    class ScenarioParser
    {
    public:
        //! A constructor.
        ScenarioParser(SimulationManager* sm);
        
        //!
        virtual bool Parse(std::string filename);
        
    protected:
        //!
        virtual bool ParseEnvironment(XMLElement* element);
        
        //!
        virtual bool ParseMaterials(XMLElement* element);
        
        //!
        virtual bool ParseLooks(XMLElement* element);
        
        //!
        virtual bool ParseStatic(XMLElement* element);
        
        //!
        virtual bool ParseSolid(XMLElement* element, SolidEntity*& solid);
        
        //!
        virtual bool ParseRobot(XMLElement* element);
        
        //!
        virtual bool ParseLink(XMLElement* element, SolidEntity*& link);
        
        //!
        virtual bool ParseJoint(XMLElement* element, Robot* robot);
        
        //!
        virtual bool ParseSensor(XMLElement* element, Robot* robot);
        
        //!
        virtual bool ParseActuator(XMLElement* element, Robot* robot);
        
    private:
        bool ParseTransform(XMLElement* element, Transform& T);
    
        XMLDocument doc;
        SimulationManager* sm;
    };
}

#endif
