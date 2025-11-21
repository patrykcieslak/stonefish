.. _cables:

======
Cables
======

Cables are soft bodies that can be used to simulate lines, ropes, tethers, etc. 
They can be defined as elastic or inelastic, and can be attached to the world or to dynamic bodies, at both ends, only at one end, or left completely free.

Properties
==========

A cable is created between two specified points in the world frame, and the resulting distance is divided into a specified number of segments. 
Cable diameter and physical material are used to compute the distributed mass. Moreover, a stretch factor can be specified to allow for elastic materials.

.. warning::

    The stretch factor is not a physical quantity, neither it represents the stretch fraction, although its range is limitted to [0, 1]. It is passed directly to the soft body configuration of the physics engine. 
    The user needs to experiment with its value to obtain the desired behaviour, which will depend on the update rate of the simulation and the mass of any objects attached to the cable.

Collisions
==========

Cables can collide with dynamic bodies, static bodies, and robot links. However, collision between cables, as well as, self-collisions are not supported.

Anchoring
=========

Cables can be free or anchored at one or both ends. An anchor can be fixed to the world or to a dynamic body. Anchoring cables directly to the robot links is not supported by the physics engine.
There are three anchor types implemented: "free", "world", and "dynamic". Creating a dynamic anchor requires specifying the dynamic body name.

.. note::

    Anchoring a cable to a robot link can be achieved by creating a small dynamic body attached to the link with a fixed constraint, and anchoring the cable to that body. 
    However, the results of this trick will depend on the mass of the small body and the cable.

Instantiation
=============

Cables can be instantiated as follows:

.. code-block:: xml

     <dynamic name="Sphere" type="sphere" physics="submerged" buoyant="true">
        <dimensions radius="0.5"/>    
        <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>    
        <material name="Steel"/>
        <look name="Yellow"/>
        <world_transform xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
    </dynamic>
    <cable name="Cable" physics="submerged" buoyant="true" collisions="true">
        <geometry diameter="0.01" number_of_segments="100"/>
        <material name="Steel" stretch_factor="0.0"/>
        <look name="Yellow" uv_scale="10.0"/>
        <first_end position="0.0 0.0 -5.0" anchor="world"/>
        <second_end position="0.0 0.0 -1.0" anchor="dynamic">
            <body name="Sphere"/>
        </second_end>
    </cable>

.. code-block:: cpp

    #include <Stonefish/entities/solids/Sphere.h>
    #include <Stonefish/entities/CableEntity.h>
    sf::Sphere* sph = new sf::Sphere("Sphere", phy, 0.5, sf::I4(), "Steel", "Yellow");
    AddSolidEntity(sph, sf::I4());
    sf::CableEntity* cable = new sf::CableEntity("Cable", phy, sf::Vector3(0.0, 0.0, -5.0), sf::Vector3(0.0, 0.0, -1.0), 100, 0.01, "Steel", "Yellow", 0.0, 10.f);
    cable->AttachToWorld(sf::CableEnds::FIRST);
    cable->AttachToSolid(sf::CableEnds::SECOND, sph);
    AddCableEntity(cable);
