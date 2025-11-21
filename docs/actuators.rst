.. _actuators:

=========
Actuators
=========

A set of common actuator models is available in the *Stonefish* library, including the ones specific for the marine robotics. The implemented actuators can be divided into two groups: the joint actuators and the link actuators. Each actuator type is described below to understand its operation and the way to include it in the simulation scenario.

.. warning:: 

    Actuators can only be created in connection with a definition of a :ref:`robot <robots>`, because they have to be attached to a robot's joint or link.

Common properties
=================

All of the actuators share a few common properties. Each actuator has a **name** and a **type**.

Optionally, the user can enable a watchdog timer that will reset the actuator if no new setpoint was received for a specified timeout time. This can be achieved by using the following syntax:

.. code-block:: xml
    
    <actuator name="Thruster" type="thruster">
       <!-- thruster definitions here -->
       <watchdog timeout="1.0"/>
    </actuator>

.. code-block:: cpp

    sf::Thruster* th = new sf::Thruster(...);  
    th->setWatchdog(Scalar(1));

Not all of the actuators are going to use the watchdog as it is limited to the ones that have a clear zero state.

.. note::

    In the following sections, description of each specific actuator implementation is accompanied with an example of actuator instantiation through the XML syntax and the C++ code. It is assumed that the XML snippets are located inside the definition of a robot. In case of C++ code, it is assumed that an object ``sf::Robot* robot = new sf::Robot(...);`` was created before the actuator definition. 

.. _joint-actuators:

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

Motor
-----

A motor is a simple actuator that applies desired torque to the joint.

.. code-block:: xml

    <actuator name="Motor" type="motor">
        <joint name="Joint1"/>
    </actuator>

.. code-block:: cpp

    #include <Stonefish/actuators/Motor.h>
    sf::Motor* motor = new sf::Motor("Motor");
    motor->setCommand(1.0);
    robot->AddJointActuator(motor, "Joint1");

Servomotor
----------

A servomotor is an electric motor connected with control and power circuits that allow for controlling it in two modes: position or velocity.
It is possible to define an initial position of the joint that will be achieved at the beginning of the simulation.

.. code-block:: xml

    <actuator name="Servo" type="servo">
        <controller position_gain="1.0" velocity_gain="1.0" max_torque="10.0" max_velocity="0.1"/>
        <joint name="Joint1"/>
        <initial position="0.5"/>
    </actuator>

.. code-block:: cpp

    #include <Stonefish/actuators/Servo.h>
    sf::Servo* srv = new sf::Servo("Servo", 1.0, 0.5, 10.0);
    srv->setControlMode(sf::ServoControlMode::POSITION_CTRL);
    srv->setMaxVelocity(0.1);
    srv->setDesiredPosition(0.5);
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

Push
----

A push actuator is a virtual actuator that applies a given force to the attached body.

.. code-block:: xml

    <actuator name="Push" type="push">
        <specs lower_limit="-10.0" upper_limit="10.0" inverted="false"/>
        <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        <link name="Link1"/>
    </actuator>

.. code-block:: cpp

    #include <Stonefish/actuators/Push.h>
    sf::Push* push = new sf::Push("Push", false, false);
    push->setForceLimits(-10.0, 10.0);
    robot->AddLinkActuator(push, "Link1", sf::I4()); 

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
    sf::Polyhedron* propMesh = new sf::Polyhedron("PropMesh", sf::BodyPhysicsType::AERODYNAMIC, sf::GetDataPath() + "propeller.obj", 1.0, sf::I4(), "Steel", "Red");
    sf::Propeller* propeller = new sf::Propeller("Prop", propMesh, 0.5, 0.45, 0.02, 1000, true, false);
    robot->AddLinkActuator(propeller, "Link1", sf::I4()); 

Simple thruster
---------------

A simple thruster is an extension of the *push* actuator that functions only underwater. It can be used, for example, in development of control systems that use idealised actuators for preliminary implementation or when the thruster model is supplied externally. 

.. code-block:: xml

    <actuator name="SimpleThruster" type="simple_thruster">
        <specs lower_thrust_limit="-10.0" upper_thrust_limit="10.0" inverted="false"/>
        <propeller right="true">
            <mesh filename="propeller.obj" scale="1.0"/>
            <material name="Steel"/>
            <look name="Red"/>
        </propeller>
        <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        <link name="Link1"/>
    </actuator>

.. code-block:: cpp

    #include <Stonefish/actuators/SimpleThruster.h>
    sf::Polyhedron* propMesh = new sf::Polyhedron("PropMesh", sf::BodyPhysicsType::SUBMERGED, sf::GetDataPath() + "propeller.obj", 1.0, sf::I4(), "Steel", "Red");
    sf::SimpleThruster* thruster = new sf::SimpleThruster("SimpleThruster", propMesh, true, false);
    robot->AddLinkActuator(thruster, "Link1", sf::I4()); 

Thruster
--------

A thruster actuator represents an underwater actuator based on a rotating propeller. Being the most common type of actuator for the underwater and surface vehicles it was given special attention. The mathematical model of the thruster is modular and combines two models: the model of rotor dynamics and the model of thrust (and torque) generation. The available models can be used in any combination, giving a very flexible setup, which should fulfill requirements of most of the users.

To properly define a thruster one has to supply a few common parameters, as well as, fill three blocks: propeller definition, rotor dynamics, and thrust model. The common parameters specify the range of the internal setpoint used to control the thruster, as well as, information about the setpoint values supplied by the user, which may be inverted and/or normalized (range [-1,1]) for convenience.

The following XML syntax presents the structue of the definition:

.. code-block:: xml

    <actuator name="Thruster" type="thruster">
        <specs max_setpoint="1000.0" inverted_setpoint="false" normalized_setpoint="true"/>
        <propeller diameter="0.2" right="true">
            <mesh filename="propeller.obj" scale="1.0"/>
            <material name="Steel"/>
            <look name="Red"/>
        </propeller>
        <rotor_dynamics type="...">
            <!-- rotor dynamics parameters here -->
        </rotor_dynamics>
        <thrust_model type="...">
            <!-- thrust model parameters here -->
        </thrust_model>
        <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        <link name="Link1"/>
    </actuator>

.. note:: 

    The limit of the thruster setpoint, called ``max_setpoint``, is an absolute value. The setpoints will be limited symmetrically. The quantity represented by the setpoints depends on the type of the input of the selected ``rotor_dynamics`` model.

The following rotor dynamics models are implemented, with their respective parameters and example XML syntax. The output of all of the models is the angular velocity of the propeller and the input quantity depends on the model of choice.

1. ``zero_order`` a simple passthrough (no dynamics), no parameters, input is angular velocity [rad/s].
   
.. code-block:: xml

    <rotor_dynamics type="zero_order"/>

2. ``first_order`` first order system, input is angular velocity [rad/s].
  - ``time_constant``

.. code-block:: xml

    <rotor_dynamics type="first_order">
        <time_constant value="1.0"/>
    </rotor_dynamics>

3. ``yoerger`` Yoerger's model, input is motor torque [Nm].
  - ``alpha``
  - ``beta``

.. code-block:: xml

    <rotor_dynamics type="yoerger">
        <alpha value="0.1"/>
        <beta value="0.5"/>
    </rotor_dynamics>

4. ``bessa`` Bessa's model, input is voltage [V].
  - ``jmsp`` inertia of the rotor
  - ``kv1`` linear thruster constant
  - ``kv2`` quadratic thruster constant
  - ``kt`` torque constant
  - ``rm`` resistance of motor windings 

.. code-block:: xml

    <rotor_dynamics type="bessa">
        <jmsp value="1.0"/>
        <kv1 value="100.0"/>
        <kv2 value="200.0"/>
        <kt value="1.0"/>
        <rm value="10.0"/>
    </rotor_dynamics>

1. ``mechanical_pi`` mechanical model of a rotating propeller, controlled using PI controller, input is angular velocity [rad/s].
  - ``rotor_inertia`` combined inertia of the propeller and the added intertia of the accelerated fluid
  - ``kp`` proportional gain
  - ``ki`` integral gain
  - ``ilimit`` integral limit (anti-windup)

.. code-block:: xml

    <rotor_dynamics type="mechanical_pi">
        <propeller_inertia value="1.0"/>
        <kp value="8.0"/>
        <ki value="5.0"/>
        <ilimit value="10.0"/>
    </rotor_dynamics>

The following thrust models are implemented, with their respective parameters and example XML syntax. The input to all of the models is the angular velocity of the propeller and the outputs are the generated thrust and the induced torque.

1. ``quadratic``
  - ``thrust_coeff`` symmetrical thrust coeffcient

.. code-block:: xml

    <thrust_model type="quadratic">
        <thrust_coeff value="1.0"/>
    </thrust_model>
  
2. ``deadband``
  - ``thrust_coeff`` (``forward`` and ``reverse``) asymmetrical thrust coefficient
  - ``deadband`` (``lower`` and ``upper``) deadband limits (tested on input)
  
.. code-block:: xml

    <thrust_model type="deadband">
        <thrust_coeff forward="0.5" reverse="0.3"/>
        <deadband lower="-10.0" upper="10.0"/>
    </thrust_model>

3. ``linear_interpolation`` velocity to thrust transformation based on linearly interpolated tabulated data
  - ``input`` space separated list of angular velocity values
  - ``output`` space separated list of thrust values (length eqal to input!)
  
.. code-block:: xml

    <thrust_model type="linear_interpolation">
        <input value="-100.0 -20.0 0.0 20.0 100.0"/>
        <output value="-7.0 -1.0 0.0 2.0 10.0"/>
    </thrust_model>

4. ``fluid_dynamics`` model based on advanced fluid dynamics equations, taking into account incomming fluid velocity
  - ``thrust_coeff`` (``forward`` and ``reverse``) asymmetrical thrust coefficient
  - ``torque_coeff`` induced torque coeffcient

.. code-block:: xml

    <thrust_model type="fluid_dynamics">
        <thrust_coeff forward="0.5" reverse="0.3"/>
        <torque_coeff value="0.1"/>
    </thrust_model>

An example of a full thruster definition utilising the XML syntax and the C++ code are shown below.

.. code-block:: xml
    
    <actuator name="Thruster" type="thruster">
        <specs max_setpoint="400.0" inverted_setpoint="false" normalized_setpoint="true"/>
        <propeller diameter="0.18" right="true">
            <mesh filename="propeller.obj" scale="1.0"/>
            <material name="Steel"/>
            <look name="Red"/>
        </propeller>
        <rotor_dynamics type="mechanical_pi">
            <propeller_inertia value="1.0"/>
            <kp value="10.0"/>
            <ki value="5.0"/>
            <ilimit value="5.0"/>
        </rotor_dynamics>
        <thrust_model type="fluid_dynamics">
            <thrust_coeff forward="0.88" reverse="0.48"/>
            <torque_coeff value="0.05"/>
        </thrust_model>
        <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        <link name="Link1"/>
    </actuator>

.. code-block:: cpp

    #include <Stonefish/actuators/Thruster.h>
    sf::Polyhedron* prop = new sf::Polyhedron("PropMesh", sf::BodyPhysicsType::SUBMERGED, sf::GetDataPath() + "propeller.obj", 1.0, sf::I4(), "Steel", "Red");
    std::shared_ptr<sf::MechanicalPI> rotorDynamics;
    rotorDynamics = std::make_shared<sf::MechanicalPI>(1.0, 10.0, 5.0, 5.0);
    std::shared_ptr<sf::FDThrust> thrustModel;
    thrustModel = std::make_shared<sf::FDThrust>(0.18, 0.88, 0.48, 0.05, true, 1000.0);
    sf::Thruster* th = new sf::Thruster("Thruster", prop, rotorDynamics, thrustModel, 0.18, true, 400.0, false, true);
    robot->AddLinkActuator(thruster, "Link1", sf::I4()); 

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

Rudder (control surface)
------------------------

A rudder, or a control surface in general, is an actuated hydrofoil that can be used to change the direction of motion of a floating or underwater vehicle.
The forces generated by this actuator include hydrodynamic lift and drag. The models are based on quadratic approximations with lift and drag coefficients. Moreover, the angle of attack is compared with the stall angle to account for rapid change in forces when the latter is exceeded.

.. code-block:: xml

    <actuator name="Rudder" type="rudder">
        <specs lift_coeff="0.5" drag_coeff="0.1" max_angle="1.0" area="0.05" stall_angle="0.9" max_angular_rate="0.2" inverted="false"/>
        <visual>
            <mesh filename="rudder.obj" scale="1.0"/>
            <material name="Steel"/>
            <look name="Red"/>
            <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        </visual>
        <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        <link name="Link1"/>
    </actuator>

.. code-block:: cpp

    #include <Stonefish/actuators/Rudder.h>
    sf::PhysicsSettings phy;
    phy.mode = sf::PhysicsMode::SUBMERGED;
    phy.collisions = false;
    phy.buoyancy = false;
    sf::Polyhedron* rudderMesh = new sf::Polyhedron("RudderMesh", phy, sf::GetDataPath() + "rudder.obj", 1.0, sf::I4(), "Steel", "Red");
    sf::Rudder* rudder = new sf::Rudder("Rudder", rudderMesh, 0.05, 0.5, 0.1, 0.9, 1.0, false, 0.2);        
    robot->AddLinkActuator(rudder, "Link1", sf::I4());

Lights
======

The *Stonefish* library delivers high quality, physically based rendering, to enable testing of computer vision algorithms on reallistic synthetic images. Lighting is one of the most important components to be considered. The library implements omnidirectional and spot lights, with physically correct illuminance and attenuation model, and multiple options to specify color. The spot lights are created automatically when the user specifies the cone angle. The color can be defined as black body temperature in Kelvins, RGB triplet or HSV triplet. Lights can be attached to any kind of body, as well as directly to the world frame (like :ref:`vision sensors <vision-sensors>`). 

.. code-block:: xml

    <light name="Omni">
        <specs radius="0.2" illuminance="10000.0"/>
        <color rgb="0.2 0.3 1.0"/>
        <world_transform xyz="1.0 5.0 2.0" rpy="0.0 0.0 0.0"/>
    </light>
    <light name="Spot">
        <specs radius="0.1" cone_angle="30.0" illuminance="2000.0"/>
        <color temperature="5600.0"/>
        <origin xyz="1.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        <link name="Link1"/>
    </light>

.. code-block:: cpp

    #include <Stonefish/actuators/Light.h>
    sf::Light* l1 = new sf::Light("Omni", 0.2, sf::Color::RGB(0.2, 0.3, 1.0), 10000.0);
    AddActuator(l1, sf::Transform(sf::IQ(), sf::Vector3(1.0, 5.0, 2.0)));
    sf::Light* l2 = new sf::Light("Spot", 0.1, 30.0, sf::Color::BlackBody(5600.0), 2000.0);
    robot->AddLinkActuator(l2, "Link1", sf::Transform(sf::IQ(), sf::Vector3(1.0, 0.0, 0.0)));