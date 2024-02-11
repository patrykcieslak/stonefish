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
//  GeneralRobot.h
//  Stonefish
//
//  Created by Patryk Cieslak on 27/01/2023.
//  Copyright(c) 2023 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GeneralRobot__
#define __Stonefish_GeneralRobot__

#include "core/Robot.h"

namespace sf
{
    class Joint;

    //! A class implementing a robotic system composed of a dynamic rigid-body chain/tree/loop, actuators and sensors.
    class GeneralRobot : public Robot
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the robot
         \param fixedBase is the robot fixed to the world?
         */
        GeneralRobot(std::string uniqueName, bool fixedBase = false);
        
        //! A destructor.
        virtual ~GeneralRobot();

        //! A method used to define a list of rigid bodies constituting the mechanical part of the robot (dynamic tree).
        /*!
         \param baseLink a solid contituting the base of the robot
         \param otherLinks a vector of subsequent links
         \param selfCollision a flag determining if self collision between multibody links should be enabled (subsequent links never collide)
         */
        void DefineLinks(SolidEntity* baseLink, std::vector<SolidEntity*> otherLinks = std::vector<SolidEntity*>(0), bool selfCollision = false);


        
        //! A method which uses links and joints definitions to build the kinematic structure of the robot.
        void BuildKinematicStructure();
        
        //! A method used to attach a sensor to a specified joint of the robot.
        /*!
         \param s a pointer to a joint sensor object
         \param monitoredJointName a name of the joint at which the sensor is attached
         */
        void AddJointSensor(JointSensor* s, const std::string& monitoredJointName);

        //! A method used to attach an actuator to a specified joint of the robot.
        /*!
         \param a a pointer to a joint actuator object
         \param actuatedJointName a name of the joint which is to be driven
         */
        void AddJointActuator(JointActuator* a, const std::string& actuatedJointName);

        //! A method adding the robot to the simulation world (includes consistency checking).
        /*!
         \param sm a pointer to a simulation manager
         \param origin a trasformation from the world origin to the robot origin
         */
        void AddToSimulation(SimulationManager* sm, const Transform& origin);

        //! A method returning the pose of the robot in the world frame.
        Transform getTransform() const;

        //! A method returning type of algorithm used for the robot.
        RobotType getType() const;
        
    private:
        Joint* getJoint(const std::string& name);
        std::vector<Joint*> joints;
        std::vector<std::pair<JointSensor*, std::string>> jsAttachments;
        std::vector<std::pair<JointActuator*, std::string>> jaAttachments;
    };
}

#endif
