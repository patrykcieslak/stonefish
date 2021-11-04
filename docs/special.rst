=====================
Special functionality
=====================

A list of special functions of the *Stonefish* library is gathered below. It includes tools, which can be useful in the analysis of the behaviour of simulated systems, not representing real world devices.

Contacts
========

The contact recording function enables capturing the history of contact points between two selected bodies (dynamic or static). The recorded contact points can optionally be displayed in the visualisation window, using one of the selected representations: path, slip (velocity) or force. Moreover, the visualisation is defined separately for each of the bodies.

An example of using a contact recording feature is presented below:

.. code-block:: xml

    <contact name="Contact1">
        <bodyA name="Body1" display="path"/>
        <bodyB name="Body2"/>
        <history points="1000"/>
    </contact>

.. code-block:: cpp

    #include <Stonefish/sensors/Contact.h>
    sf::SolidEntity* body1 = ...;
    sf::StaticEntity* body2 = ...;
    sf::Contact* cnt = new sf::Contact("Contact1", body1, body2, 1000);
    cnt->setDisplayMask(CONTACT_DISPLAY_PATH_A);
    AddContact(cnt); 

Soft collision
==============

Collisions of all bodies defined in the simulation scenario are considered rigid by default. Sometimes it is important to improve the stability of the collision response, e.g., in case of high impacts, or introduce softness, which can simulate linearly deformable materials. This can be achieved by extending the definition of a selected dynamic body as follows:

.. code-block:: xml

    <dynamic>
        <!-- all standard definitions -->
        <contact stiffness="1000.0" damping="0.5"/>
    </dynamic>

.. code-block:: cpp

    sf::SolidEntity* solid = ...;
    solid->SetContactProperties(true, 1000.0, 0.5);

Magnetic materials
==================

A special feature of the material system allows for defining magnetic properties of bodies. 
This can be used, e.g., for a simple simulation of permanent magnet grippers. 
Magnetic properties are defined by extending the standard material definition with a factor specifying the type of magnetic behaviour and its strengt, according to the following syntax:

.. code-block:: xml

    <material name="Aluminium" density="2710.0" restitution="0.7" magnetic="1.0"/>

.. code-block:: cpp

    CreateMaterial("Aluminium", sf::Scalar(2791), sf::Scalar(0.7), sf::Scalar(1.0));

The "magnetic" factor has different meaning depending on its value:

-  **<0** - ferromagnetic material
-  **=0** - paramagnetic material
-  **>0** - permanent magnet.

The absolute value of the factor defines the strength of magnetic interactions. Only a pair of bodies, first made of a ferromagnetic material and second being a permanent magnet, will interact. Interaction between two magnets is not implemented due to the lack of information about polarity.