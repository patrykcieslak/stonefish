=====================
Communication devices
=====================

Class header: ``Stonefish\comms\Comm.h``

Communication devices were included in *Stonefish* to account for the delays and directional character of different communication interfaces, in different mediums.
They can be attached to the links of a robot, to a dynamic or static body, or to the world origin. 

The common properties of communication devices are:

1) **Name**: unique string

2) **Device ID**: unique number identifying the device

3) **Type**: type of the communication device

4) **Origin**: position and orientation of the device frame with respect to the parent frame

5) [Optional] **Name of link**: name of the link of the robot the device is attached to

6) [Optional] **Name of body**: name of the body the device is attached to (static or dynamic)

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
    
    In the following sections only the tags specific to each type of the communication device will be defined.

Acoustic modem
==============

Class header: ``Stonefish\comms\AcousticModem.h``

Acoustic modem is an underwater communication device based on an acoustic transducer. It has the following properties:

1) **Horizontal FOV**: horizontal field of view of the transducer [deg]

2) **Vertical FOV**: vertical field of view of the transducer [deg]

3) **Range**: operating range [m]

4) **Conneted device ID**: unique number specifying the conneted device

.. code-block:: xml

    <comm name="..." device_id="..." type="acoustic_modem">
        <specs horizontal_fov="{1}" vertical_fov="{2}" range="{3}"/>
        <connect device_id="{4}"/>
        <!-- common definitions here -->
    </comm>
    
USBL
====

Class header: ``Stonefish\comms\USBL.h``

Ultra short base line (USBL) is a device based on a tightly packed array of underwater acoustic transducers. It can be used for underwater communication as well as for localization of the signal source in 3D space. It has the following properties:

1) **Horizontal FOV**: horizontal field of view of the transducer [deg]

2) **Vertical FOV**: vertical field of view of the transducer [deg]

3) **Range**: operating range [m]

4) **Conneted device ID**: unique number specifying the conneted device

5) [Optional] **Auto-ping rate**: rate at which the device automatically pings the connected device [Hz]

6) [Optional] **Range noise**: standard deviation of the range measurement [m]

7) [Optional] **Angle noise**: standard deviation of the angle measurement noise [deg]

.. code-block:: xml    

    <comm name="..." device_id="..." type="usbl">
        <specs horizontal_fov="{1}" vertical_fov="{2}" range="{3}"/>
        <connect device_id="{4}"/>
        <autoping rate="{5}"/>
        <noise range="{6}" angle="{7}"/>
        <!-- common definitions here -->
    </comm>