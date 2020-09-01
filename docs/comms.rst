.. _comms:

=====================
Communication devices
=====================

Communication devices were included in the *Stonefish* library to account for the delays and directional character of different communication interfaces, in different mediums.
They can be attached to the links of a robot, to a dynamic or static body, or to the world origin. 

The common properties of communication devices are:

1) **Name**: unique string

2) **Device ID**: unique number identifying the device

3) **Type**: type of the communication device

4) **Origin**: position and orientation of the device frame with respect to the parent frame

5) **Name of link**: name of the link of the robot the device is attached to (optional)

6) **Name of body**: name of the body the device is attached to (static or dynamic; optional)

.. code-block:: xml
    
    <comm name="{1}" device_id="{2}" type="{3}">
        <!-- specific definitions here -->
        <origin xyz="{4a}" rpy="{4b}"/>
        <link name="{5}"/>
        <body name="{6}"/>
    </comm>

.. warning::

    Only one of the options: 5 or 6, can be used. If neither ``link`` nor ``body`` tag is specified, the communication device is considered to be attached to the world origin.

.. note::

    In the following sections, description of each specific implementation of a communication device is accompanied with an example of its instantiation through the XML syntax and the C++ code. It is assumed that the XML snippets are located inside the definition of a robot. In case of C++ code, it is assumed that an object ``sf::Robot* robot = new sf::Robot(...);`` was created before the device definition. 

Acoustic modem
==============

An acoustic modem is an underwater communication device based on an acoustic transducer.

.. code-block:: xml

    <comm name="Modem" device_id="5" type="acoustic_modem">
        <specs horizontal_fov="360.0" vertical_fov="120.0" range="1000.0"/>
        <connect device_id="9"/>
        <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        <link name="Link1"/>
    </comm>
    
.. code-block:: cpp

    #include <Stonefish/comms/AcousticModem.h>
    sf::AcousticModem* modem = new sf::AcousticModem("Modem", 5, 360.0, 120.0, 1000.0);
    modem->Connect(9);
    robot->AddComm(modem, "Link1", sf::I4());

USBL
====

The ultra short baseline (USBL) is a device based on a tightly packed array of underwater acoustic transducers. It can be used for underwater communication as well as for localization of the signal source in 3D space. 
User can optionally define the standard deviation of the angle and range measurments, similarily to the sensor definitions.
Another feature of the USBL implementation is an automatic ping function used to update the measurements at a specified rate.

.. code-block:: xml    

    <comm name="USBL" device_id="5" type="usbl">
        <specs horizontal_fov="360.0" vertical_fov="160.0" range="1000.0"/>
        <connect device_id="9"/>
        <autoping rate="1.0"/>
        <noise range="0.05" angle="0.02"/>
        <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        <link name="Link1"/>
    </comm>

.. code-block:: cpp

    #include <Stonefish/comms/USBL.h>
    sf::USBL* usbl = new sf::USBL("USBL", 5, 360.0, 160.0, 1000.0);
    usbl->Connect(9);
    usbl->EnableAutoPing(1.0);
    usbl->setNoise(0.05, 0.02, 0.0, 0.0);
    robot->AddComm(usbl, "Link1", sf::I4());