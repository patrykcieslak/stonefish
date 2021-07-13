.. _comms:

=====================
Communication devices
=====================

Communication devices were included in the *Stonefish* library to account for the delays and directional character of different communication interfaces, in different mediums.
They can be attached to all kinds of bodies, as well as directly to the world frame. All communication devices share the following properties:

1) **Name**: unique string

2) **Device ID**: unique number identifying the device

3) **Type**: type of the communication device

4) **Origin**: position and orientation of the device frame with respect to the parent frame

5) **Link name**: name of the link of the robot the device is attached to (for robots)

.. code-block:: xml
    
    <comm name="{1}" device_id="{2}" type="{3}">
        <!-- specific definitions here -->
        <origin xyz="{4a}" rpy="{4b}"/>
        <link name="{5}"/>
    </comm>

.. note::

    In the following sections, description of each specific implementation of a communication device is accompanied with an example of its instantiation through the XML syntax and the C++ code. It is assumed that the XML snippets are located inside the definition of a robot. In case of C++ code, it is assumed that an object ``sf::Robot* robot = new sf::Robot(...);`` was created before the device definition. 

Acoustic modem
==============

An acoustic modem is an underwater communication device based on an acoustic transducer. When creating an acoustic modem it is required to specify an id of the acoustic node it will be connected to.
During the acoustic communication the directional characteristics of both the sender and the receiver are used to determine if both nodes can see each other. 
Moreover, an occlusion test is performed as default, to take into account obstacles in the path of the acoustic beam. The occlusion test can be disabled (it has to be done for both communicating nodes).

.. code-block:: xml

    <comm name="Modem" device_id="5" type="acoustic_modem">
        <specs min_vertical_fov="0.0" max_vertical_fov="220.0" range="1000.0"/>
        <connect device_id="9" occlusion_test="true"/>
        <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        <link name="Link1"/>
    </comm>
    
.. code-block:: cpp

    #include <Stonefish/comms/AcousticModem.h>
    sf::AcousticModem* modem = new sf::AcousticModem("Modem", 5, 0.0, 220.0, 1000.0);
    modem->Connect(9);
    modem->setOcclusionTest(true);
    robot->AddComm(modem, "Link1", sf::I4());

USBL
====

The ultra short baseline (USBL) is a device based on a tightly packed array of underwater acoustic transducers. It shares the same properties as the acoustic modem and extends upon them.
It can be used for underwater communication as well as for localization of the signal source in 3D space. User can optionally define the standard deviation of the measurements of slant range, horizontal angle and vertical angle. Moreover, the resolution of the range and angle measurements can be set.
Another feature of the USBL implementation is an automatic ping function used to update the measurements at a specified rate.

.. code-block:: xml    

    <comm name="USBL" device_id="5" type="usbl">
        <specs min_vertical_fov="0.0" max_vertical_fov="220.0" range="1000.0"/>
        <connect device_id="9" occlusion_test="false"/>
        <autoping rate="1.0"/>
        <noise range="0.05" horizontal_angle="0.2" vertical_angle="0.5"/>
        <resolution range="0.1" angle="0.1"/>
        <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        <link name="Link1"/>
    </comm>

.. code-block:: cpp

    #include <Stonefish/comms/USBL.h>
    sf::USBL* usbl = new sf::USBL("USBL", 5, 0.0, 220.0, 1000.0);
    usbl->Connect(9);
    usbl->EnableAutoPing(1.0);
    usbl->setOcclusionTest(false);
    usbl->setNoise(0.05, 0.2, 0.5);
    usbl->setResolution(0.1, 0.1);
    robot->AddComm(usbl, "Link1", sf::I4());