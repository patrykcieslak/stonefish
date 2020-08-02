======
Robots
======

Every robot is composed of links connected in a kinematic tree structure, actuators and sensors. The dynamics of a robot are computed using the Featherstone's multi-body algorithm. 
All multi-body systems, with rigid-bodies connected by joints, are considered robots. This does not necessary mean actual robots but for example a panel with rotating valves. Robots are built from links, joints, sensors and actuators.

Links
^^^^^

Joints
^^^^^^

Sensors
^^^^^^^

Actuators
^^^^^^^^^


Building a robot
----------------

The following steps have to be completed in order to build a robot using the *Stonefish* library:
1. Defining links
2. Defining sensors
3. Defining actuators
4. Building the structure
5. Attaching sensors and actuators
6. Adding robot to the simulation scenario

An example code is attached below, which presents how a simple robot can be built. In the code it is assumed that a physical material called "Steel" and a graphical look called "green" were defined.

.. code-block:: cpp

    //Defining links
    sf::Sphere* base = new sf::Sphere("Base", sf::Scalar(0.2), sf::I4(), "Steel", sf::BodyPhysicsType::SURFACE_BODY, "green");
    sf::Box* link1 = new sf::Box("Link1", sf::Vector3(0.1,0.02,0.5), sf::Transform(sf::Quaternion(M_PI_2,0,0), sf::Vector3(0.0,0.0,-0.2)), "Steel", sf::BodyPhysicsType::SURFACE_BODY, "green");
    sf::Box* link2 = new sf::Box("Link2", sf::Vector3(0.1,0.02,0.5), sf::Transform(sf::Quaternion(M_PI_2,0,0), sf::Vector3(0.0,0.0,-0.2)), "Steel", sf::BodyPhysicsType::SURFACE_BODY, "green");

    std::vector<sf::SolidEntity*> links;
    links.push_back(link1);
    links.push_back(link2);

    //Defining sensors
    sf::IMU* imu = new sf::IMU("IMU", sf::Scalar(-1), 1000);
    sf::RotaryEncoder* enc = new sf::RotaryEncoder("Encoder", sf::Scalar(-1), 1000);
    
    //Defining actuators
    sf::ServoMotor* srv = new sf::ServoMotor("Servo", sf::Scalar(1.0), sf::Scalar(1.0), sf::Scalar(10.0));
    
    //Building the structure
    sf::Robot* robot = new sf::Robot("Robot", false);
    robot->DefineLinks(base, links);
    robot->DefineRevoluteJoint("Joint1", "Base", "Link1", sf::Transform(sf::IQ(), sf::Vector3(0.0,0.25,-0.2)), sf::Vector3(0.0,1.0,0.0), std::make_pair(1.0, -1.0));
    robot->DefineRevoluteJoint("Joint2", "Base", "Link2", sf::Transform(sf::IQ(), sf::Vector3(0.0,-0.25,-0.2)), sf::Vector3(0.0,1.0,0.0), std::make_pair(1.0, -1.0));

    //Attaching sensors and actuators   
    robot->AddLinkSensor(imu, "Link2", sf::I4());
    robot->AddJointSensor(enc, "Joint2");
    robot->AddJointActuator(srv, "Joint1");
        
    //Adding robot to the simulation scenario
    AddRobot(robot, sf::Transform(sf::IQ(), sf::Vector3(0.0,0.0,-2.0)));
