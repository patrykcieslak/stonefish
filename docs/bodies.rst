==============
Dynamic bodies
==============

Two types of rigid bodies can be defined in the simulation scenario: static and dynamic. Each body is characterised by its geometry, material and look. Only dynamic bodies can be used as links of a multi-body chain (robot).

Algorithms
==========

The simulation scenario is composed of environment definition and dynamical objects definitions. The environment can include ocean simulation, which is one of the main features of the software.  

The geometry-based fluid dynamics computations (hydrodynamics and aerodynamics), which are a unique feature of the software, are based on an idea of using geometry of the rigid bodies to: a) approximate added mass and fluid dynamics coefficients; b) sum forces along the surfaces of bodies to enable realistic buoyancy calculation at the ocean surface, as well as drag forces depending on local fluid velocity (water jets, currents etc.). A type of required calculations is decided based upon a flag assigned to every rigid body in the simulation: a) ``SURFACE_BODY`` - no aerodynamics or hydrodynamics; 
b) ``FLOATING_BODY`` - hydrodynamics with buoyancy; c) ``SUBMERGED_BODY`` - hydrodynamics with buoyancy and added mass; d) ``AERODYNAMIC_BODY`` - aerodynamics.

The type of physics simulation does not only depend on the environment itself but also on the choice made for every dynamic body. While creating these bodies, as standalone or as links of a robot, a flag of type `sf::BodyPhysicsType` has to be set. The following options are possible, with their corresponding meaning:

- ``SURFACE_BODY`` - no aerodynamic or hydrodynamic forces computed
- ``FLOATING_BODY`` - buoyancy and hydrodynamic drag is computed, no added mass effect
- ``SUBMERGED_BODY`` - buoyancy and hydrodynamic forces including added mass effect are computed
- ``AERODYNAMIC_BODY`` - aerodynamic drag is computed (lift not supported for general bodies)

Common body properties
----------------------

Dynamic bodies are bodies which are not fixed to the global frame (subclasses of ``sf::SolidEntity``). These kind of bodies are affected by different forces, depending on the type of environment and the type of body physics simulation selected. Dynamic bodies can be created standalone or used as links of a robot. 

An important feature of the body creation process is that its mechanical properties are computed based on provided geometry: mass, moments of inertia, volume, location of the centre of gravity (CG) and location of the centre of buoyancy (CB). It is possible to override some of the properties by using following methods of the ``class SolidEntity``:

- ``void ScalePhysicalPropertiesToArbitraryMass(Scalar mass)`` - changes mass and scales moments of inertia accordingly
- ``void SetArbitraryPhysicalProperties(Scalar mass, const Vector3& inertia, const Transform& CG)`` - changes mass, moments of inertia and CG location to arbitrary values

The following diagram presents the coordinate frames defined for each of the dynamic bodies. The location of the frames is not realistic but only for illustration purposes. The shape of each body is described by: a graphical mesh (black), a mesh used for physics computations (green) and an approximation of the body geometry (dashed blue). There are 3 frames associated with these meshes: *frame G*, *frame C* and *frame H* respectively. The origin of the body is defined by *frame O*, which is used as a handle, to position the body in the *world frame W*.

.. image:: images/frames.svg
    :width: 400
    :alt: Drawing presenting coordinate frames used in the simulator.

Parametric solids
-----------------

The simplest rigid bodies that can be created are parametric solids, which include: box, sphere, cylinder, torus and wing. The physical geometry of parametric solids is the same as the graphical mesh. Besides the wing body, the collisions of parametric solids are computed analytically, with simple formulas, which makes them very efficient to use. 

- ``Sphere`` - a ball or sphere with a given radius
- ``Box`` - a box described by 3 lengths along X,Y,Z axes
- ``Cylinder`` - a cylinder parametrised with radius and height, aligned with the Z axis
- ``Torus`` - a torus parametrised with its major and minor radius, aligned with the Y axis
- ``Wing`` - a solid based on an extruded NACA profile (4-digit system), aligned with Y axis 

Models loaded from geometry files
---------------------------------

Another way of creating a rigid body is by loading its geometry from a Wavefront OBJ file or an STL file (only text files supported), which is possible by using ``class Polyhedron``. The user can provide different geometry for rendering and physics computation, to deliver plausible visualisation and efficient simulation at the same time. It should be noticed that the collision geometry is always a convex hull around the physical geometry. Assuming that a physical material called "Steel" and a graphical look called "grey" where defined before, a rigid body based on a geometry file can be created writing:

.. code-block:: cpp

    sf::Polyhedron* poly = new sf::Polyhedron("Poly", "geometry_file_name.obj", sf::Scalar(1), sf::I4(), "Steel", sf::BodyPhysicsType::SUBMERGED_BODY, "grey");

Compound bodies
---------------

Sometimes it is useful to define a rigid body as a combination of multiple rigid bodies, e.g., when creating a base of a mobile robot. It is also a way of creating a concave mesh which will respond to collisions correctly. In the _Stonefish_ library such body can be defined using ``class Compound``. User can define a rigid body composed of external and internal parts, which influences the performed physics computations, e.g., drag forces are only computed for external parts. 

Example:

.. code-block:: cpp

    //Define components of the compound body
    sf::Cylinder* cylinder1 = new sf::Cylinder("Cylinder1", 0.4, 0.5, sf::I4(), "Steel", sf::BodyPhysicsType::SUBMERGED_BODY, "grey");
    sf::Cylinder* cylinder2 = new sf::Cylinder("Cylinder2", 0.3, 1.0, sf::I4(), "Steel", sf::BodyPhysicsType::SUBMERGED_BODY, "grey");
    sf::Cylinder* cylinder3 = new sf::Cylinder("Cylinder3", 0.1, 0.3, sf::I4(), "Steel", sf::BodyPhysicsType::SUBMERGED_BODY, "grey");

    //Build compound body
    sf::Compound* comp = new sf::Compound("Compound", cylinder1, sf::I4(), sf::BodyPhysicsType::SUBMERGED_BODY);
    comp->AddExternalPart(cylinder2, sf::Transform(sf::IQ(), sf::Vector3(0.0,0.6,0.0)));
    comp->AddInternalPart(cylinder3, sf::I4());

Two different types of rigid bodies can be defined in the simulation scenario, that is *static* and *dynamic* bodies.


Xml
===

All bodies that should move and do not constitute a multi-body system have to be defined as dynamic bodies. For these type of bodies a convex hull is built for collision computation. Therefore, to achieve correct collisions for a non-convex body, it is necessary to compose a it from multiple convex bodies, using the compound body type. Whenever possible, one of the basic solid shapes should be used to improve collision detection performance and enable analytical computation of collision points, i.e., achieve smooth collision with curved surfaces.

A dynamic body is defined using ``<dynamic name="[1]" type="[2]" physics="[3]" buoyant="[4]">[5]</dynamic>``. The name of the body [1] should be unique across the simulation scenario. The body type [2] has to be equal to one of the types specified below and it determines how the body will be created, i.e., which tags will have to be included in the description [5]. Moreover, the type of physics computations [3] as well as a boolean flag determining if the body is buoyant [4] have to be defined, to ensure proper behaviour of the body. Some of the tags in the body description [5] are common for all types of bodies, like the material the body is made of [6], its look determining the rendering style [7] and its initial orientation [8] and position [9] in the world (NED) frame. Therefore, a general structure of the definition of a dynamic body is the following:

.. code-block:: xml

    <dynamic name="[1]" type="[2]" physics="[3]" buoyant="[4]">
        [definitions specific for a selected body type]
        <material name="[6]"/>
        <look name="[7]"/>
        <world_transform rpy="[8]" xyz="[9]"/>
    </dynamic>

Tags specific for each available body type are the following:

1. Sphere ``type="sphere"`` - a sphere (ball) with a specified radius:

.. code-block:: xml

    <dimensions radius="1.0"/>    
    <origin rpy="0.0 0.0 0.0" xyz="0.0 0.0 0.0"/>

2. Cylinder ``type="cylinder"`` - a cylinder with a specified radius and height, with its axis aligned with the local Z axis:  

.. code-block:: xml

    <dimensions radius="1.0" height="2.0"/>
    <origin rpy="0.0 0.0 0.0" xyz="0.0 0.0 0.0"/>

3. Box ``type="box"`` - a box with specified width, height and length:  

.. code-block:: xml

    <dimensions xyz="0.5 1.0 2.0"/>
    <origin rpy="0.0 0.0 0.0" xyz="0.0 0.0 0.0"/>

4. Torus ``type="torus"`` - a torus with a specified major and minor radius:

.. code-block:: xml

    <dimensions major_radius="1.0" minor_radius="0.1"/>
    <origin rpy="0.0 0.0 0.0" xyz="0.0 0.0 0.0"/>

5. Mesh ``type="model"`` - an arbitrary mesh made of triangles. The geometry can be specified separately for the physics computation and the rendering. If only physical geometry is specified it is also used for rendering. The geometry can be loaded from STL or OBJ files (ASCII format). The origin tag is used to apply local transformation to the geometry, i.e., transformation in the frame defined by the 3D software used to save the geometry.

.. code-block:: xml

    <physical>
        <mesh filename="pipe_phy.obj" scale="1.0"/>
        <origin rpy="0.0 0.0 0.0" xyz="0.0 0.0 0.0"/> 
    </physical>
    <visual> <!-- optional -->
        <mesh filename="pipe_gra.obj" scale="1.0"/>
        <origin rpy="0.0 0.0 0.0" xyz="0.0 0.0 0.0"/>
    </visual>

*Following the above instructions, an exemplary dynamic torus can be defined as:*

.. code-block:: xml

    <dynamic name="Torus1" type="torus" physics="submerged" buoyant="true">   
        <dimensions major_radius="1.0" minor_radius="0.2"/>
        <origin rpy="0.0 0.0 0.0" xyz="0.0 0.0 0.0"/>
        <material name="Aluminium"/>
        <look name="yellow"/>
        <world_transform rpy="0.0 0.0 0.0" xyz="0.0 0.0 2.0"/>
    </dynamic>

Compound bodies
^^^^^^^^^^^^^^^

A special type of body, called *compound*, can be created, to allow for intuitive construction of a group of rigidly connected elements and/or enable correct collision with non-convex geometry. A compound body is composed of external and internal parts, with at least one obligatory external part. The general structure of a compound body definition is following:

.. code-block:: xml

    <dynamic name="[1]" type="[2]" physics="[3]" buoyant="[4]">
        <external_part name="[1.1]" type="[1.2]" physics="[1.3]" buoyant="[1.4]">
            [definitions specific for the type of part selected]
            <material name="[1.5]"/>
            <look name="[1.6]"/>
            <compound_transform rpy="[1.7]" xyz="[1.8]"/>
        </external_part>

        <internal_part name="[2.1]" type="[2.2]" physics="[2.3]" buoyant="[2.4]">
            [definitions specific for the type of part selected]
            <material name="[2.5]"/>
            <look name="[2.6]"/>
            <compound_transform rpy="[2.7]" xyz="[2.8]"/>
        <internal_part/>
        <world_transform rpy="[5]" xyz="[6]"/>
    </dynamic>

It can be noticed that the definition of each compound body part, internal or external, is the same as the definition of a single dynamic body, i.e., each part can be one of the five types of dynamic bodies defined above. There are two differences though: instead of the *world transform* we have a *compound transform* [1.7-1.8, 2.7-2.8], which positions the part in the origin frame of the compound body, and the physics computed for the parts change depending if they are defined as internal or external, e.g., fluid damping forces do not act on internal parts. The *world transform* of the whole body is defined outside the definition of the parts [5-6]. There is no definition of material or look for the whole body because these properties can change for each part.


