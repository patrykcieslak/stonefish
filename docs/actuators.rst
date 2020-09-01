.. _actuators:

=========
Actuators
=========

A set of common actuator models is available in the *Stonefish* library, including the ones specific for the marine robotics. The implemented actuators can be divided into two groups: the joint actuators and the link actuators. Each actuator type is described below to understand its operation and the way to include it in the simulation scenario.

.. warning:: 

    Actuators can only be created in connection with a definition of a :ref:`robot <robots>`, because they have to be attached to a robot's joint or link.

.. note::

    In the following sections, description of each specific actuator implementation is accompanied with an example of actuator instantiation through the XML syntax and the C++ code. It is assumed that the XML snippets are located inside the definition of a robot. In case of C++ code, it is assumed that an object ``sf::Robot* robot = new sf::Robot(...);`` was created before the actuator definition. 


Joint actuators
===============

The joint actuators are attached to the robot's joints and they apply forces or torques between the links. They share a set of common properties:

1) **Name:** unique string

2) **Type:** type of the actuator

3) **Joint name**: the name of the robot joint that the actuator is attached to

.. code-block:: xml
    
    <actuator name="{1}" type="{2}">
       <!-- specific definitions here -->
       <joint name="{3}"/>
    </sensor>

Servomotor
----------

A servomotor is an electric motor connected with control and power circuits that allow for controlling it in different modes: torque, position or velocity.

.. code-block:: xml

    <actuator name="Servo" type="servo">
        <controller position_gain="1.0" velocity_gain="0.5" max_torque="10.0"/>
        <joint name="Joint1"/>
    </actuator>

.. code-block:: cpp

    #include <Stonefish/actuators/Servo.h>
    sf::Servo* srv = new sf::Servo("Servo", 1.0, 0.5, 10.0);
    robot->AddJointActuator(srv, "Joint1");

Link actuators
==============

The link actuators are attached to the robot's links and they apply forces or torques to the links. They share a set of common properties:

1) **Name:** unique string

2) **Type:** type of the actuator

3) **Origin:** the transformation from the link frame to the actuator frame

4) **Link name**: the name of the robot link that the actuator is attached to

.. code-block:: xml
    
    <actuator name="{1}" type="{2}">
       <!-- specific definitions here -->
       <origin xyz="{3a}" rpy="{3b}"/>
       <link name="{4}"/>
    </sensor>

Propeller
---------

A propeller is an actuator working in atmosphere, representing an airplane propeller driven by a motor. 

.. code-block:: xml

    <actuator name="Prop" type="propeller">
        <specs thrust_coeff="0.45" torque_coeff="0.02" max_rpm="1000" inverted="false"/>
        <propeller diameter="0.5" right="true">
            <mesh filename="propeller.obj" scale="1.0"/>
            <material name="Steel"/>
            <look name="Red"/>
        </propeller>
        <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        <link name="Link1"/>
    </actuator>

.. code-block:: cpp

    #include <Stonefish/actuators/Propeller.h>
    sf::Polyhedron* propMesh = new sf::Polyhedron("PropMesh", sf::GetDataPath() + "propeller.obj", 1.0, sf::I4(), "Steel", sf::BodyPhysicsType::AERODYNAMIC, "Red");
    sf::Propeller* prop = new sf::Propeller("Prop", propMesh, 0.5, 0.45, 0.02, 1000, true, false);
    robot->AddLinkActuator(prop, "Link1", sf::I4()); 

Thruster
--------

A thruster is an actuator working underwater, representing an underwater thruster with a propeller.

.. code-block:: xml

    <actuator name="Thruster" type="thruster">
        <specs thrust_coeff="0.45" thrust_coeff_backward="0.35" torque_coeff="0.02" max_rpm="1000" inverted="false"/>
        <propeller diameter="0.2" right="true">
            <mesh filename="propeller.obj" scale="1.0"/>
            <material name="Steel"/>
            <look name="Red"/>
        </propeller>
        <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        <link name="Link1"/>
    </actuator>

.. code-block:: cpp

    #include <Stonefish/actuators/Thruster.h>
    sf::Polyhedron* propMesh = new sf::Polyhedron("PropMesh", sf::GetDataPath() + "propeller.obj", 1.0, sf::I4(), "Steel", sf::BodyPhysicsType::SUBMERGED, "Red");
    sf::Thruster* th = new sf::Thruster("Thruster", propMesh, 0.2, std::make_pair(0.45, 0.35), 0.02, 1000, true, false);
    robot->AddLinkActuator(th, "Link1", sf::I4()); 

Variable buoyancy system (VBS)
------------------------------

A variable buoyancy system (VBS) is a container with an elastic wall, which can be filled with gas under pressure to change its volume and thus its buoyancy. It is used to control the depth of the robot. The VBS is defined by providing a set of meshes representing its states between minimum and maximum volume. Between these shapes the volume is interpolated linearly. In the current implementation, due to the limitations of the physics engine, the inertia of the water filling the container is not taken into account when computing dynamic forces.

.. code-block:: xml

    <actuator name="VBS" type="vbs">
        <volume initial="0.5">
            <mesh filename="empty.obj"/>
            <mesh filename="half.obj"/>
            <mesh filename="full.obj"/>
        </volume>
        <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        <link name="Link1"/>
    </actuator>

.. code-block:: cpp

    #include <Stonefish/actuators/VariableBuoyancy.h>
    std::vector<std::string> meshes;
    meshes.push_back(sf::GetDataPath() + "empty.obj");
    meshes.push_back(sf::GetDataPath() + "half.obj");
    meshes.push_back(sf::GetDataPath() + "full.obj");
    sf::VariableBuoyancy* vbs = new sf::VariableBuoyancy("VBS", meshes, 0.5);
    robot->AddLinkActuator(vbs, "Link1", sf::I4());

Light
-----

A light is a special actuator that does not generate any forces but represents an omnidirectional or spot light. It can be used to simulate artificial lighting in dark environments, light beacons etc.

.. code-block:: xml

    <actuator name="Light" type="light">
        <specs radius="0.1" cone_angle="30.0" illuminance="1000.0"/>
        <color temperature="5600.0"/>
        <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        <link name="Link1"/>
    </actuator>

.. code-block:: cpp

    #include <Stonefish/actuators/Light.h>
    sf::Light* lt = new sf::Light("Light", 0.1, 30.0, sf::Color::BlackBody(5600.0), 1000.0);
    robot->AddLinkActuator(lt, "Link1", sf::I4());