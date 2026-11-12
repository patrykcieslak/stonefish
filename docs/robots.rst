.. _robots:

======
Robots
======

In the *Stonefish* library every kinematic tree is called a robot. The robots can thus represent vehicles, stationary manipulators, as well as, other structures with moving parts.
The kinematic tree is built of links (rigid bodies) connected by joints. When this mechanical structure is defined, then it is possible to attach :ref:`actuators <actuators>`, :ref:`sensors <sensors>` and :ref:`communication devices <comms>` to its joints and links. The dynamics of the tree can be solved using one of two algorithms: the Roy Featherstone's multi-body algorithm or a general sequential impulse algorithm. The former is highly preferred for its precision, stability, and performance. The latter can be used when support for kinematic loops is necessary.
Robots are defined using the ``<robot>`` tag. The base link of a robot can either be fixed to the world frame or floating. Robot definitions have to be placed in the root node of the XML file or after the definition of materials and looks in the C++ code.

.. _robot-links:

Links
=====

Links are the mechanical parts of the robot, joined together to create the kinematic tree. Links are considered :ref:`dynamic bodies <dynamic-bodies>` and are defined in the same way. To differentiate links from free dynamic bodies the XML syntax introduces the ``<base_link>`` and ``<link>`` tags, which replace the ``<dynamic>`` tag. A definition of a robot has to contain at least one link, called the base link. Only one base link can exist in a kinematic tree.

Joints
======

Joints are connections between links that define the relative position and orientation of links, as well as the allowed directions of motion (degrees of freedom). The ``<joint>`` tag is used to define joints in the XML syntax. There are three types of joints implemented in the *Stonefish* library: 

1. Fixed ``type="fixed"`` - all degrees of freedom are locked
2. Prismatic ``type="prismatic"`` - one linear degree of freedom
3. Revolute ``type="revolute"`` - one angular degree of freedom

Defining a robot
================

The definition of the robot comprises multiple links, joints and attached devices. If the robot is defined using the XML syntax, the order of the definitions of these components does not matter. However, using the C++ code, there are few steps which have to be completed in the following order:

1. Defining links
2. Defining joints (building the structure)
3. Creating actuators, sensors and communication devices
4. Attaching actuators, sensors and communication devices
5. Adding robot to the simulation scenario.

Below, an example of defining a complete robot is presented, first using the XML syntax and later using its C++ twin. It is assumed that the material "Steel" and the look "Green" were defined before.

.. code-block:: xml

    <robot name="Robot" fixed="false" self_collisions="false" algorithm="featherstone">
        <base_link name="Base" type="sphere" physics="surface">
            <dimensions radius="0.2"/>
            <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
            <material name="Steel"/>
            <look name="Green"/>
        </base_link>
        <link name="Link1" type="box" physics="surface">
            <dimensions xyz="0.1 0.02 0.5"/>
            <origin xyz="0.0 0.0 -0.2" rpy="0.0 0.0 3.14159"/>
            <material name="Steel"/>
            <look name="Green"/>
        </link>
        <link name="Link2" type="box" physics="surface">
            <dimensions xyz="0.1 0.02 0.5"/>
            <origin xyz="0.0 0.0 -0.2" rpy="0.0 0.0 3.14159"/>
            <material name="Steel"/>
            <look name="Green"/>
        </link>
        <joint name="Joint1" type="revolute">
            <parent name="Base"/>
            <child name="Link1"/>
            <origin xyz="0.0 0.25 -0.2" rpy="0.0 0.0 0.0"/>
            <axis xyz="0.0 1.0 0.0"/>
        </joint>
        <joint name="Joint2" type="revolute">
            <parent name="Base"/>
            <child name="Link2"/>
            <origin xyz="0.0 -0.25 -0.2" rpy="0.0 0.0 0.0"/>
            <axis xyz="0.0 1.0 0.0"/>
        </joint>
        <actuator name="Servo" type="servo">
            <controller position_gain="1.0" velocity_gain="1.0" max_torque="10.0"/>
            <joint name="Joint1"/>
        </actuator>
        <sensor name="Encoder" type="encoder">
            <history samples="1000"/>
            <joint name="Joint2"/>
        </sensor>
        <sensor name="IMU" type="imu" rate="10.0">
            <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
            <link name="Link2"/>
        </sensor>
        <world_transform xyz="0.0 0.0 -2.0" rpy="0.0 0.0 0.0"/>
    </robot>

.. code-block:: cpp
    
    #include <Stonefish/core/FeatherstoneRobot.h>
    #include <Stonefish/entities/solids/Sphere.h>
    #include <Stonefish/entities/solids/Box.h>
    #include <Stonefish/actuators/Servo.h>
    #include <Stonefish/sensors/scalar/RotaryEncoder.h>
    #include <Stonefish/sensors/scalar/IMU.h>

    //1. Defining links
    std::unique_ptr<sf::Sphere> base = std::make_unique<sf::Sphere>("Base", 0.2, sf::I4(), "Steel", sf::BodyPhysicsType::SURFACE, "Green");

    std::vector<std::unique_ptr<sf::SolidEntity>> links;
    links.push_back(std::make_unique<sf::Box>("Link1", sf::Vector3(0.1, 0.02, 0.5), sf::Transform(sf::Quaternion(M_PI_2, 0.0, 0.0), sf::Vector3(0.0, 0.0, -0.2)), "Steel", sf::BodyPhysicsType::SURFACE, "Green"));
    links.push_back(std::make_unique<sf::Box>("Link2", sf::Vector3(0.1, 0.02, 0.5), sf::Transform(sf::Quaternion(M_PI_2, 0.0, 0.0), sf::Vector3(0.0, 0.0, -0.2)), "Steel", sf::BodyPhysicsType::SURFACE, "Green"));

    //2. Building the structure
    std::unique_ptr<sf::FeatherstoneRobot> robot = std::make_unique<sf::FeatherstoneRobot>("Robot", false);
    robot->DefineLinks(std::move(base), std::move(links));
    robot->DefineRevoluteJoint("Joint1", "Base", "Link1", sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.25, -0.2)), sf::Vector3(0.0, 1.0, 0.0), std::make_pair(1.0, -1.0));
    robot->DefineRevoluteJoint("Joint2", "Base", "Link2", sf::Transform(sf::IQ(), sf::Vector3(0.0, -0.25, -0.2)), sf::Vector3(0.0, 1.0, 0.0), std::make_pair(1.0, -1.0));
    robot->BuildKinematicStructure();

    //3. Creating and attaching actuators and sensors
    robot->AddJointActuator(std::make_unique<sf::Servo>("Servo", 1.0, 1.0, 10.0), "Joint1");
    robot->AddJointSensor(std::make_unique<sf::RotaryEncoder>("Encoder", -1.0, 1000), "Joint2");
    robot->AddLinkSensor(std::make_unique<sf::IMU>("IMU", 10.0), "Link2", sf::I4());
    
    //4. Adding robot to the simulation scenario
    AddRobot(std::move(robot), sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, -2.0)));
