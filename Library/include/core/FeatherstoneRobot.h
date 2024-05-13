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
//  FeatherstoneRobot.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/11/2018.
//  Copyright(c) 2018-2022 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FeatherstoneRobot__
#define __Stonefish_FeatherstoneRobot__

#include "core/Robot.h"

namespace sf
{
    class FeatherstoneEntity;

    //! A class implementing a robotic system composed of a dynamic rigid-body tree, actuators and sensors.
    class FeatherstoneRobot : public Robot
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the robot
         \param fixedBase is the robot fixed to the world?
         */
        FeatherstoneRobot(std::string uniqueName, bool fixedBase = false);
        
        //! A destructor.
        virtual ~FeatherstoneRobot();

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

        //! A method used to attach an actuator to a specified link of the robot.
        /*!
         \param a a pointer to a link actuator object
         \param actuatedLinkName a name of the link which is to be actuated
         \param origin a transformation from the link origin to the actuator frame
         */
        void AddLinkActuator(LinkActuator* a, const std::string& actuatedLinkName, const Transform& origin);

        //! A method adding the robot to the simulation world (includes consistency checking).
        /*!
         \param sm a pointer to a simulation manager
         \param origin a trasformation from the world origin to the robot origin
         */
        void AddToSimulation(SimulationManager* sm, const Transform& origin);

        //! A method respawning the robot in the simulation world.
        /*!
         \param sm a pointer to a simulation manager
         \param origin a trasformation from the world origin to the robot origin
         */
        void Respawn(SimulationManager* sm, const Transform& origin) override;

        //! A method returning the pose of the robot in the world frame.
        Transform getTransform() const;

        //! A method returning the index of the link.
        /*!
         \param name the name of the link
         \return index of the link
        */
        int getLinkIndex(const std::string& name) const;

        //! A method returning type of algorithm used for the robot.
        RobotType getType() const;

        //! A method returning a point to the underlaying Featherstone entity.
        FeatherstoneEntity* getDynamics();

    private:
        int getJoint(const std::string& name);

        FeatherstoneEntity* dynamics;
    };
}

#endif