.. _environment:

===========
Environment
===========

The description of a simulation world starts with the definition of the environment to be simulated. The *Stonefish* library is prepared to be used as a general robot simulator, which implements crucial elements for marine robotics. The standard simulation medium is air, which can be used for the ground and flying robots (preliminary support). If the user is interested in simulating marine robots, a virtual ocean has to be enabled. Next, the world can be filled with static rigid bodies, which may include terrain and structures fixed to the world frame. If some parts of the environment are considered dynamic, they have to be created as dynamic bodies or robots, described in the subsequent parts of the documentation.

The parameters of the environment are specified between ``<environment> ... </environment>``, inside the root node of the XML file, and they include position of the world frame origin (NED origin), the ocean definitions and the atmosphere definitions, explained in the following sections. If no ocean definitions are present the ocean simulation is disabled.

The basic structure of the XML tags used to define the environment is:

.. code-block:: xml

    <environment>
        <ned latitude="40.0" longitude="3.0"/> <!-- geographic coordinates of the NED origin -->
        <!-- ocean definitions -->
        <!-- atmosphere definitions -->
    </environment>

The same can be achieved in code by the following line:

.. code-block:: cpp

    getNED()->Init(40.0, 20.0, 0.0);

Ocean
=====

The ocean simulation is one of crucial parts of the *Stonefish* library. Obviously, marine robots can not be simulated well without reallistic hydrodynamics, interactions with the ocean surface and underwater currents. Moreover, ocean optics-based visuals are an important feature when trying to reproduce images from submerged cameras.

Waves
-----

The library implements an ocean surface simulation utilising the fast Fourier transform (FFT), following the ideas of Tessendorf. Multiple FFT layers are computed using a GPU-based algoritm, to simulate the spectrum of the ocean waves and transform it into the 3D space and time domain. Later, the GPU generated data can be used to simulate the interaction between the ocean water and the dynamic bodies. This interaction is still under development and should be disable if not needed. Therefore, there is two ways the ocean can be simulated: with geometrical waves or as a flat surface. The flat surface option is also better in terms of performance.

Currents
--------

Water currents have a significant impact on the operation of underwater robots. Therefore, the *Stonefish* library implements some basic forms of water currents, treated as water velocity fields. Currently implemented types of water currents include:

-  ``Uniform`` the same velocity in the whole ocean
-  ``Jet`` a velocity distribution coming from an circular underwater outlet
-  ``Pipe`` a velocity distrubution resambling a virtual pipe submerged in the ocean

Ocean optics
------------

As mentioned before, underwater rendering plays an important role in realistic simulation of optical sensors. The *Stonefish* library implements optical effects encountered in ocean waters like light absorption, out-scattering, and in-scattering, also called the airlight.
The absorption and scattering coefficeints are computed for three wavelengths, corresponding to the red, green and blue channels of the rendering pipeline, based on the Jerlov measurements covering wide spectrum of the coastal water types. The water quality is defined with a single parameter ranging from 0.0 to 1.0, where the lower limit corresponds to the Jerlov type I water and the upper limit represents the Jerlov type 9C water.

Suspended particles
-------------------

An additional visual effect included in the ocean rendering is suspended particles, sometimes referred to as underwater snow. It is enabled by default but can easily be disabled from the scenario file or the standard GUI.

Definitions
-----------

The ocean definitions have to be placed inside the environment node of the scenario file, between ``<ocean> ... </ocean>``. An example covering all of the implemented features is presented below:

.. code-block:: xml

    <ocean>
        <water density="1031.0" jerlov="0.2"/>
        <waves height="0.0"/>
        <particles enabled="true"/>
        <current type="uniform">
            <velocity xyz="1.0 0.0 0.0"/>
        </current>
        <current type="jet">
            <center xyz="0.0 0.0 3.0"/>
            <outlet radius="0.2"/>
            <velocity xyz="0.0 2.0 0.0"/>
        </current>
    </ocean>

The following lines of code can be used to achieve the same:

.. code-block:: cpp

    getMaterialManager()->CreateFluid("OceanWater", 1031.0, 0.002, 1.33);
    EnableOcean(0.0, getMaterialManager()->getFluid("OceanWater"));
    getOcean()->setWaterType(0.2);
    getOcean()->AddVelocityField(new sf::Uniform(sf::Vector3(1.0, 0.0, 0.0)));
    getOcean()->AddVelocityField(new sf::Jet(sf::Vector3(0.0, 0.0, 3.0), sf::Vector3(0.0, 1.0, 0.0), 0.2, 2.0));

Atmosphere
==========

The atmosphere simulation is another component of the virtual environment. It enables realistic motion of aerodynamic bodies, taking into account winds. In the current state only air drag is simulated. An important feature of the atmosphere simulation is the photo-realistic rendering of sky and Sun.  

Winds
-----

Winds have a significant impact on the motion of flying robots. Therefore, the *Stonefish* library implements some basic forms of wind, treated as air velocity fields. Currently implemented types of wind include:

-  ``Uniform`` the same velocity in the whole atmosphere
-  ``Jet`` a velocity distribution coming from an circular outlet
-  ``Pipe`` a velocity distrubution resambling a virtual pipe submerged in the atmosphere

Sky and Sun
-----------

The sky and Sun rendering is based on precomputed atmospheric scattering algorithm. It takes into account multiple layers of Earth's atmosphere, including the ozone layer, to generate photo-realistic image of the sky. Sun's position on the sky can be changed dynamically during the simulation.  

Definitions
-----------

The atmosphere definitions have to be placed inside the environment node of the scenario file, between ``<atmosphere> ... </atmosphere>``. An example covering all of the implemented features is presented below:

.. code-block:: xml

    <atmosphere>
        <sun azimuth="20.0" elevation="50.0"/>
        <wind type="uniform">
            <velocity xyz="1.0 0.0 0.0"/>
        </wind>
        <wind type="jet">
            <center xyz="0.0 0.0 3.0"/>
            <outlet radius="0.2"/>
            <velocity xyz="0.0 2.0 0.0"/>
        </wind>
    </atmosphere>

The following lines of code can be used to achieve the same:

.. code-block:: cpp

    getAtmosphere()->SetupSunPosition(20.0, 50.0);
    getAtmosphere()->AddVelocityField(new sf::Uniform(sf::Vector3(1.0, 0.0, 0.0)));
    getAtmosphere()->AddVelocityField(new sf::Jet(sf::Vector3(0.0, 0.0, 3.0), sf::Vector3(0.0, 1.0, 0.0), 0.2, 2.0));

Static bodies
=============

The static bodies are all elements of the simulation scenario that remain fixed to the world origin, for the whole duration of the simulation. These kind of objects are used only for collision and sensor simulation. Due to their fixed position in the world, they do not require computation of dynamics and can deliver optimised collision detection algoritms. An important feature is that static bodies can have arbitrary collision geometry, not requiring convexity. Static bodies include a simple plane, basic solids, meshes and terrain. They are defined in the XML syntax using the ``<static> ... </static>`` tags, in a following way:

.. code-block:: xml

    <static name="{1}" type="{2}">
        <!-- specific definitions here -->
        <material name="{3}"/>
        <look name="{4}"/>
        <world_transform xyz="{5a}" rpy="{5b}"/>
    </static>

where

1) **Name**: unique string

2) **Type**: type of the static body

3) **Material name**: the name of the physical material

4) **Look name**: the name of the graphical material

5) **World transform**: position and orientation of the body with respect to the world frame.

Depending on the type of the static body the specific definitions change.

.. note:: 

    In the following examples it is assumed that physical materials called "Steel" and "Rock", as well as looks called "Yellow" and "Gray", were defined.

Plane
-----

A plane is the simplest static body, that is usually used as the ground plane or the sea bottom, if no complex terrain is needed.

In the XML syntax the plane does not have any additional parameters. It is only needed to define the type of the static body as "plane". An exemplary plane can be defined as follows:

.. code-block:: xml

    <static name="Floor" type="plane">
        <material name="Steel"/>
        <look name="Yellow"/>
        <world_transform xyz="0.0 0.0 1.0" rpy="0.0 0.0 0.0"/>
    </static>

The same can be achieved in code:

.. code-block:: cpp

    sf::Plane* floor = new sf::Plane("Floor", 1000.f, "Steel", "Yellow");
    AddStaticEntity(floor, sf::Transform(sf::Quaternion(0.0, 0.0, 0.0), sf::Vector3(0.0, 0.0, 1.0));

.. note::

    Plane definition has one special functionality. It is possible to scale the automatically generated texture coordinates, to tile the textures associated with the look. In the XML syntax the ``<look>`` tag has to be augmented to include attribute ``uv_scale="#.#"`` and in the C++ code the scale can be passed as the last argument in the object constructor.

Obstacles
---------

The obstacles are static solids, created using parameteric definitions: spheres, cylinders and boxes, or loaded from geometry files. 

In case of the **parameteric solids** the specific definitions are reduced to their dimensions. Both the physical and the graphical mesh have the same complexity. Depending on the shape a different set of dimensions has to be specified:

- **Sphere** ``type="sphere"`` - ball with a specified radius {1}:
 
.. code-block:: xml
  
    <dimensions radius="{1}"/>

- **Cylinder** ``type="cylinder"`` - cylinder along Z axis, with a specified radius {1} and height {2}:

.. code-block:: xml

    <dimensions radius="{1}" height="{2}"/>

- **Box** ``type="box"`` - box with specified width {1}, length {2} and height {3}: 

.. code-block:: xml
    
    <dimensions xyz="{1} {2} {3}"/>

.. note::

    Box definition has one special functionality. It is possible to choose from 3 automatically generated texture coordinate schemes: scheme 0 (default) assumes that the texture is in a cubemap format and applies it to the box faces accordingly, scheme 1 applies the whole texture to each face of the box, and scheme 2 tiles the whole texture along each of the box faces, based on face dimensions. In the XML syntax the ``<look>`` tag has to be augmented to include attribute ``uv_mode="#"`` and in the C++ code the mode can be passed as the last argument in the object constructor.
   
Definition of arbitrary **triangle meshes** ``type="model"``, loaded from geometry files, is more complex. Their geometry can be specified separately for the physics computations ``<physical> .. </physical>`` and the rendering ``<visual> ... </visual>``. If no visual geometry is specified, the physical geometry is used for rendering. Moreover, the physical mesh is used when simulating operation of the :ref:`link sensors <link-sensors>` while the visual mesh is used by the :ref:`vision sensors <vision-sensors>`. Finally, the collision response, which depends on the physical mesh, can be significantly improved, by enabling the convex hull approximation ``<mesh ... convex="true"/>``. Without this option, the collision mesh is treated as a concave triangle mesh, which severly impacts collision performance and in most cases can be avoided, by partitioning the concave body into multiple convex bodies. For more details on preparing mesh data check :ref:`preparing-geometry`.

An example of creating obstacles, including triangle meshes, is presented below:

.. code-block:: xml

    <static name="Ball" type="sphere">
        <dimensions radius="0.5"/>
        <material name="Steel"/>
        <look name="Yellow"/>
        <world_transform xyz="2.0 0.0 5.0" rpy="0.0 0.0 0.0"/>
    </static>
    
    <static name="Wall" type="box">
        <dimensions xyz="10.0 0.2 5.0"/>
        <material name="Steel"/>
        <look name="Gray"/>
        <world_transform xyz="0.0 5.0 2.0" rpy="0.0 0.0 0.0"/>
    </static>

    <static name="Canyon" type="model">
        <physical>
            <mesh filename="canyon_phy.obj" scale="1.0" convex="true"/>
            <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/> 
        </physical>
        <visual>
            <mesh filename="canyon_vis.obj" scale="1.0"/>
            <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        </visual>
        <material name="Rock"/>
        <look name="Gray"/>
        <world_transform xyz="0.0 0.0 10.0" rpy="0.0 0.0 0.0"/>
    </static>

The ``<origin>`` tag is used to apply local transformation to the geometry, i.e., to change the position of the mesh origin and rotate the mesh, before placing it in the world. It is especially useful in case of geometry exported from 3D software in a wrong frame.

The same can be achieved using the following code:

.. code-block:: cpp

    sf::Obstacle* ball = new sf::Obstacle("Ball", 0.5, "Steel", "Yellow");
    AddStaticEntity(ball, sf::Transform(sf::Quaternion(0.0, 0.0, 0.0), sf::Vector3(2.0, 0.0, 5.0)));
    sf::Obstacle* wall = new sf::Obstacle("Wall", sf::Vector3(10.0, 0.2, 5.0), "Steel", "Gray");
    AddStaticEntity(wall, sf::Transform(sf::Quaternion(0.0, 0.0, 0.0), sf::Vector3(0.0, 5.0, 2.0)));
    sf::Obstacle* canyon = new sf::Obstacle("Canyon", sf::GetDataPath() + "canyon_vis.obj", 1.0, sf::I4(), sf::GetDataPath() + "canyon_phy.obj", 1.0, sf::I4(), true, "Rock", "Gray");
    AddStaticEntity(canyon, sf::Transform(sf::Quaternion(0.0, 0.0, 0.0), sf::Vector3(0.0, 0.0, 10.0)));

.. note::

    Function ``std::string sf::GetDataPath()`` returns a path to the directory storing simulation data, specified during the construction of the ``sf::SimulationApp`` object. Function ``sf::Transform sf::I4()`` creates an identity transformation matrix.

Terrain
-------

Currently the *Stonefish* library implements one type of easily defined terrain mesh which is a heightmap based terrain ``type="terrain"``. This kind of terrain mesh is generated from a planar grid displaced in the Z direction, based on the values of the heightmap pixels. Scale of the terrain is defined in meters per pixel and the height is defined by providing value correspondinng to a fully saturated pixel.
The heightmap has to be a single channel (grayscale) image, with an 8 bit or 16 bit precision. The latter allows for much higher height resolution.

The following example presents the definition of a heightmap based terrain:

.. code-block:: xml

    <static name="Bottom" type="terrain">
        <height_map filename="terrain.png"/>
        <dimensions scalex="0.1" scaley="0.2" height="10.0"/>
        <material name="Rock"/>
        <look name="Gray"/>
        <world_transform xyz="0.0 0.0 15.0" rpy="0.0 0.0 0.0"/>
    </static>

.. code-block:: cpp

    sf::Terrain* bottom = new sf::Terrain("Bottom", sf::GetDataPath() + "terrain.png", 0.1, 0.2, "Rock", "Gray");
    AddStaticEntity(bottom, sf::Transform(sf::Quaternion(0.0, 0.0, 0.0), sf::Vector3(0.0, 0.0, 15.0)));

.. note::

    Terrain definition has one special functionality. It is possible to scale the automatically generated texture coordinates, to tile the textures associated with the look. In the XML syntax the ``<look>`` tag has to be augmented to include attribute ``uv_scale="#.#"`` and in the C++ code the scale can be passed as the last argument in the object constructor.
