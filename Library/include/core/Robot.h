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
//  Robot.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/11/2018.
//  Copyright(c) 2018-2025 Patryk Cieslak. All rights reserved.
//

#pragma once

#include <utility>
#include "StonefishCommon.h"
#include "joints/Joint.h"

namespace sf
{
    class SimulationManager;
    class SolidEntity;
    class Sensor;
    class Actuator;
    class Comm;
    class LinkSensor;
    class JointSensor;
    class VisionSensor;
    class LinkActuator;
    class JointActuator;
    
    //! An enum specifying the type of entity.
    enum class RobotType {GENERAL, FEATHERSTONE};

    //! A class implementing a robotic system composed of a dynamic rigid-body tree, actuators and sensors.
    class Robot
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the robot
         \param fixedBase is the robot fixed to the world?
         */
        Robot(std::string uniqueName, bool fixedBase = false);
        
        //! A destructor.
        virtual ~Robot();
        
        //DYNAMICS
        //! A method used to define a list of rigid bodies constituting the mechanical part of the robot (dynamic tree).
        /*!
         \param baseLink a solid contituting the base of the robot
         \param otherLinks a vector of subsequent links
         \param selfCollision a flag determining if self collision between multibody links should be enabled (subsequent links never collide)
         */
        virtual void DefineLinks(SolidEntity* baseLink, std::vector<SolidEntity*> otherLinks = std::vector<SolidEntity*>(0), bool selfCollision = false) = 0;
        
        //! A method used to define a revolute joint between two mechanical parts of the robot.
        /*!
         \param jointName a name for the joint
         \param parentName a name of the parent link
         \param childName a name of the child link
         \param origin frame of the joint
         \param axis an axis of the joint
         \param positionLimits a pair of min and max limit of joint position (if min > max then joint has no limits)
         \param damping joint motion damping (works when there is no actuator attached to the joint) 
         */
        void DefineRevoluteJoint(std::string jointName, std::string parentName, std::string childName, const Transform& origin, 
                                 const Vector3& axis, std::pair<Scalar, Scalar> positionLimits = std::make_pair(Scalar(1), Scalar(-1)), Scalar damping = Scalar(-1));
        
        //! A method used to define a prismatic joint between two mechanical parts of the robot.
        /*!
         \param jointName a name for the joint
         \param parentName a name of the parent link
         \param childName a name of the child link
         \param origin frame of the joint
         \param axis an axis of the joint
         \param positionLimits a pair of min and max limit of joint position (if min > max then joint has no limits)
         \param damping joint motion damping (works when there is no actuator attached to the joint)
         */
        void DefinePrismaticJoint(std::string jointName, std::string parentName, std::string childName, const Transform& origin, 
                                  const Vector3& axis, std::pair<Scalar, Scalar> positionLimits = std::make_pair(Scalar(1), Scalar(-1)), Scalar damping = Scalar(-1));
        
        //! A method used to define a fixed joint between two mechanical parts of the robot.
        /*!
         \param jointName a name for the joint
         \param parentName a name of the parent link
         \param childName a name of the child link
         \param origin frame of the joint
         */
        void DefineFixedJoint(std::string jointName, std::string parentName, std::string childName, const Transform& origin);
        
        //! A method which uses links and joints definitions to build the kinematic structure of the robot.
        virtual void BuildKinematicStructure() = 0;
        
        //PERCEPTION
        //! A method used to attach a sensor to a specified link of the robot.
        /*!
         \param s a pointer to a link sensor object
         \param monitoredLinkName a name of the link to which the sensor is attached
         \param origin a transfromation from the link origin to the sensor frame
         */
        void AddLinkSensor(LinkSensor* s, const std::string& monitoredLinkName, const Transform& origin);
        
        //! A method used to attach a sensor to a specified joint of the robot.
        /*!
         \param s a pointer to a joint sensor object
         \param monitoredJointName a name of the joint at which the sensor is attached
         */
        virtual void AddJointSensor(JointSensor* s, const std::string& monitoredJointName) = 0;
        
        //! A method used to attach a vision sensor to a specified link of the robot.
        /*!
         \param s a pointer to a vision sensor object
         \param attachmentLinkName a name of the link to which the sensor is attached
         \param origin a transformation from the link origin to the sensor frame
         */
        void AddVisionSensor(VisionSensor* s, const std::string& attachmentLinkName, const Transform& origin);
        
        //ACTUATORS
        //! A method used to attach an actuator to a specified link of the robot.
        /*!
         \param a a pointer to a link actuator object
         \param actuatedLinkName a name of the link which is to be actuated
         \param origin a transformation from the link origin to the actuator frame
         */
        virtual void AddLinkActuator(LinkActuator* a, const std::string& actuatedLinkName, const Transform& origin);
        
        //! A method used to attach an actuator to a specified joint of the robot.
        /*!
         \param a a pointer to a joint actuator object
         \param actuatedJointName a name of the joint which is to be driven
         */
        virtual void AddJointActuator(JointActuator* a, const std::string& actuatedJointName) = 0;
        
        //COMMUNICATION DEVICES
        //! A method used to attach a communication device to a specified link of the robot.
        /*!
         \param s a pointer to a comm object
         \param attachmentLinkName a name of the link to which the comm is attached
         \param origin a transfromation from the link origin to the comm frame
         */
        void AddComm(Comm* c, const std::string& attachmentLinkName, const Transform& origin);
        
        //GENERAL
        //! A method adding the robot to the simulation world (includes consistency checking).
        /*!
         \param sm a pointer to a simulation manager
         \param origin a trasformation from the world origin to the robot origin
         */
        virtual void AddToSimulation(SimulationManager* sm, const Transform& origin);
        
        //! A method respawning the robot in the simulation world.
        /*!
         \param sm a pointer to a simulation manager
         \param origin a trasformation from the world origin to the robot origin
         */
        virtual void Respawn(SimulationManager* sm, const Transform& origin);

        //! A method returning a pointer to the actuator with a given name.
        /*!
         \param aname the name of the actuator
         \return a pointer to the actuator object
         */
        Actuator* getActuator(std::string aname);
        
        //! A method returning a pointer to the actuator by index.
        /*!
         \param index the id of the actuator
         \return a pointer to the actuator object
         */
        Actuator* getActuator(size_t index);
        
        //! A method returning a pointer to the sensor with a given name.
        /*!
         \param sname the name of the sensor
         \return a pointer to the sensor object
         */
        Sensor* getSensor(std::string sname);
        
        //! A method returning a pointer to the sensor by index.
        /*!
         \param index the id of the sensor
         \return a pointer to the sensor object
         */
        Sensor* getSensor(size_t index);
        
        //! A method returning a pointer to the communication device with a given name.
        /*!
         \param cname the name of the communication device
         \return a pointer to the comm object
         */
        Comm* getComm(std::string cname);
        
        //! A method returning a pointer to the communication device by index.
        /*!
         \param index the id of the communication device
         \return a pointer to the comm object
         */
        Comm* getComm(size_t index);
        
        //! A method returning a pointer to the base link solid.
        SolidEntity* getBaseLink();
        
        //! A method returning a pointer to the link.
        /*!
         \param lname a name of the link
         \return a pointer to the link solid
         */
        SolidEntity* getLink(const std::string& lname);

        //! A method returning a pointer to the link by index.
        /*!
         \param index the id of the link
         \return a pointer to the link solid
         */
        SolidEntity* getLink(size_t index);
        
        //! A method returning the pose of the robot in the world frame.
        virtual Transform getTransform() const = 0;
        
        //! A method returning the name of the robot.
        std::string getName();

        //! A method returning type of algorithm used for the robot.
        virtual RobotType getType() const = 0;
        
    protected:
        struct JointData
        {
            JointType jtype;
            std::string name;
            std::string parent;
            std::string child;
            Transform origin;
            Vector3 axis;
            std::pair<Scalar, Scalar> posLim;
            Scalar damping;
        };
        std::vector<JointData> jointsData_; // For construction

        std::vector<SolidEntity*> detachedLinks_;
        std::vector<SolidEntity*> links_;
        std::vector<Sensor*> sensors_;
        std::vector<Actuator*> actuators_;
        std::vector<Comm*> comms_;
        std::string name_;
        bool fixed_;
    };
} // namespace sf