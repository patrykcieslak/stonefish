//
//  Robot.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/11/2018.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Robot__
#define __Stonefish_Robot__

#include <utility>
#include "StonefishCommon.h"

namespace sf
{
    class SimulationManager;
    class SolidEntity;
    class FeatherstoneEntity;
    class Sensor;
    class Actuator;
    class LinkSensor;
    class JointSensor;
    class VisionSensor;
    class LinkActuator;
    class JointActuator;
    
    //! A class inplementing a robotic system composed of a dynamic rigid-body tree, actuators and sensors.
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
        void DefineLinks(SolidEntity* baseLink, std::vector<SolidEntity*> otherLinks = std::vector<SolidEntity*>(0), bool selfCollision = false);
        
        //! A method used to define a revolute joint between two mechanical parts of the robot.
        /*!
         \param jointName a name for the joint
         \param parentName a name of the parent link
         \param childName a name of the child link
         \param origin frame of the joint
         \param axis an axis of the joint
         \param positionLimits a pair of min and max limit of joint position (if min > max then joint has no limits)
         */
        void DefineRevoluteJoint(std::string jointName, std::string parentName, std::string childName, const Transform& origin, 
                                 const Vector3& axis, std::pair<Scalar, Scalar> positionLimits = std::make_pair(Scalar(1), Scalar(-1)));
        
        //! A method used to define a prismatic joint between two mechanical parts of the robot.
        /*!
         \param jointName a name for the joint
         \param parentName a name of the parent link
         \param childName a name of the child link
         \param origin frame of the joint
         \param axis an axis of the joint
         \param positionLimits a pair of min and max limit of joint position (if min > max then joint has no limits)
         */
        void DefinePrismaticJoint(std::string jointName, std::string parentName, std::string childName, const Transform& origin, 
                                  const Vector3& axis, std::pair<Scalar, Scalar> positionLimits = std::make_pair(Scalar(1), Scalar(-1)));
        
        //! A method used to define a fixed joint between two mechanical parts of the robot.
        /*!
         \param jointName a name for the joint
         \param parentName a name of the parent link
         \param childName a name of the child link
         \param origin frame of the joint
         */
        void DefineFixedJoint(std::string jointName, std::string parentName, std::string childName, const Transform& origin);
        
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
        void AddJointSensor(JointSensor* s, const std::string& monitoredJointName);
        
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
        void AddLinkActuator(LinkActuator* a, const std::string& actuatedLinkName, const Transform& origin);
        
        //! A method used to attach an actuator to a specified joint of the robot.
        /*!
         \param a a pointer to a joint actuator object
         \param actuatedJointName a name of the joint which is to be driven
         */
        void AddJointActuator(JointActuator* a, const std::string& actuatedJointName);
        
        //GENERAL
        //! A method adding the robot to the simulation world (includes consistency checking).
        /*!
         \param sm a pointer to a simulation manager
         \param origin a trasformation from the world origin to the robot origin
         */
        void AddToSimulation(SimulationManager* sm, const Transform& origin);
        
        //! A method returning a pointer to the actuator with a given name.
        Actuator* getActuator(std::string name);
        
        //! A method returning a pointer to the sensor with a given name.
        Sensor* getSensor(std::string name);
        
        //! A method returning the pose of the robot in the world frame.
        virtual Transform getTransform() const;
        
        //! A method returning the name of the robot.
        std::string getName();
        
    private:
        void getFreeLinkPair(const std::string& parentName, const std::string& childName, unsigned int& parentId, unsigned int& childId);
        SolidEntity* getLink(const std::string& name);
        int getJoint(const std::string& name);
        
        FeatherstoneEntity* dynamics;
        bool fixed;
        std::vector<SolidEntity*> detachedLinks;
        std::vector<SolidEntity*> links;
        std::vector<Sensor*> sensors;
        std::vector<Actuator*> actuators;
        std::string name;
    };
}

#endif
