===========
Environment
===========

The description of a simulation scenario starts with the definition of the environment to be simulated. The *Stonefish* library is prepared to be used as a general robot simulator, which implements crucial elements for marine robotics. The standard simulation medium is air, which can be used for the ground and flying robots (preliminary support). If the user is interested in simulating marine robots, a virtual ocean has to be enabled. Next, the world can be filled with static rigid bodies, which may include terrain and structures fixed to the world frame. If some parts of the environment are considered dynamic, they have to be created as dynamic bodies or robots, described in the subsequent parts of the documentation.

The parameters of the environment are specified between ``<environment> ... </environment>``, and they include position of the world frame origin, position of the sun in the sky and the ocean definitions explained in the next section:

.. code-block:: xml

    <environment>
        <ned latitude="40.0" longitude="3.0"/> <!-- geographic coordinates of the NED origin -->
        <sun azimuth="20.0" elevation="50.0"/> <!-- sun position -->
        <!-- ocean definitions -->
    </environment>

The same can be achieved in code by the following lines:

.. code-block:: cpp

    getNED()->Init(40.0, 20.0, 0.0);
    getAtmosphere()->SetupSunPosition(20.0, 50.0);

Ocean
=====

The ocean simulation is one of crucial parts of the *Stonefish* library. Obviously, marine robots can not be simulated well without reallistic hydrodynamics, interactions with the ocean surface and underwater currents. Moreover, ocean optics-based visuals are an important feature when trying to reproduce images from submerged cameras.

Waves
-----

The library implements an ocean surface simulation utilising the fast Fourier transform (FFT), following the ideas of Tessendorf. Multiple FFT layers are computed using a GPU-based algoritm, to simulate the spectrum of the ocean waves and transform it into the 3D space and time domain. Later, the GPU generated data can be used to simulate the interaction between the ocean water and the dynamic bodies. This interaction is still under development and should be disable if not needed. Therefore, there is two ways the ocean can be simulated: with geometrical waves or as a flat surface. The flat surface option is also better in terms of performance.

Currents
--------

Water currents have a significant impact on the operation of underwater robots. Therefore, the *Stonefish* library implements some basic forms of water currents, treated as water velocity fields. Currently implemented types of water currents include:

- ``Uniform`` the same velocity in the whole ocean
- ``Jet`` a velocity distribution coming from an underwater pipe outlet
- ``Pipe`` a velocity distrubution in a virtual pipe

Ocean optics
------------

As mentioned before, underwater rendering plays an important role in reallistic simulation of optical sensors. The *Stonefish* library implements optical effects encountered in ocean waters like light absorption, out-scattering, and in-scattering, also called the airlight.
The absorption and scattering coefficeints are computed for three wavelengths, corresponding to the red, green and blue channels of the rendering pipeline, based on the Jerlov measurements covering wide spectrum of the coastal water types. The water quality is defined with a single parameter ranging from 0.0 to 1.0, where the lower limit corresponds to the Jerlov type I water and the upper limit represents the Jerlov type 9C water.

Definitions
-----------

The ocean definitions have to be placed inside the environment node of the scenario file, between ``<ocean> ... </ocean>``. An example covering all of the implemented features is presented below:

.. code-block:: xml

    <ocean enabled = "true">
        <water density="1031.0" jerlov="0.2"/>
        <waves height="0.0"/>
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
    getOcean()->SetupWaterProperties(0.2);
    getOcean()->AddVelocityField(new sf::Uniform(sf::Vector3(1.0, 0.0, 0.0)));
    getOcean()->AddVelocityField(new sf::Jet(sf::Vector3(0.0, 0.0, 3.0), sf::Vector3(0.0, 1.0, 0.0), 0.2, 2.0));

Static bodies
=============

The static bodies are all elements of the simulation scenario that remain fixed to the world origin, for the whole duration of the simulation. These kind of objects are used only for collision and sensor simulation. Due to their fixed position in the world, they do not require computation of dynamics and can deliver optimised collision detection algoritms. An important feature is that static bodies can have arbitrary collision geometry, not requiring convexity. Static bodies include a simple plane, basic solids, meshes and terrain.

Plane
-----

A plane is the simplest static body, that is usually used as the ground plane or the sea bottom, if no complex terrain is needed.

Obstacles
---------

The obstacles are all static solids, created using parameteric definitions or loaded from geometry files.

The following code is an example of defining an environment and adding it to the simulation scenario. The example assumes that a physical material called "Rock" and a graphical look called "seabed" were defined.

.. code-block:: cpp

    sf::Plane* plane = new sf::Plane("Bottom", sf::Scalar(10000.0), "Rock", "seabed");
    AddStaticEntity(plane, sf::Transform(sf::IQ(), sf::Vector3(0.0,0.0,5.0)));
    sf::Obstacle* box = new sf::Obstacle("Box", sf::Vector3(1.0,1.0,1.0), "Rock", "seabed");
    AddStaticEntity(box, sf::Transform(sf::IQ(), sf::Vector3(0.0,0.0,4.5)));
    sf::Cylinder* cylinder = new sf::Cylinder("Cylinder", sf::Scalar(0.5), sf::Scalar(2.0), sf::Transform::getIdentity(), "Rock", sf::BodyPhysicsType::SUBMERGED_BODY, "seabed");
    AddSolidEntity(cylinder, sf::Transform(sf::IQ(), sf::Vector3(0.0,0.0,2.0)));

Elements of the environment, like terrain, structures, fixed obstacles, can be considered static bodies and should be defined as such, to improve performance of the simulation. Apart from the efficiency side, the geometry used for the static bodies can be arbitrary, i.e., does not have to be convex to achieve correct collisions.

A static body is defined using ``<static name="[1]" type="[2]">[3]</static>``. The name of the body [1] should be unique across the simulation scenario. The body type [2] has to be equal to one of the types specified below and it determines how the body will be created, i.e., which tags will have to be included in the description [3].
Some of the tags are common for all types of bodies like the material the body is made of [4], its look determining the rendering style [5] and its orientation [6] and position [7] in the world (NED) frame, which is fixed for a static body by definition. Therefore, a general structure of the definition of a static body is the following:

.. code-block:: xml

    <static name="[1]" type="[2]">
        [definitions specific for a selected body type]
        <material name="[4]"/>
        <look name="[5]"/>
        <world_transform rpy="[6]" xyz="[7]"/>
    </static>

Tags specific for each available body type are the following:

1. Plane ``type="plane"`` - an infinite plane: no additional tags.

2. Sphere ``type="sphere"`` - a sphere (ball) with a specified radius:  
``<dimensions radius="1.0"/>``

3. Cylinder ``type="cylinder"`` - a cylinder with a specified radius and height, with its axis aligned with the local Z axis:  
``<dimensions radius="1.0" height="2.0"/>``

4. Box ``type="box"`` - a box with specified width, height and length:  
``<dimensions xyz="0.5 1.0 2.0"/>``
   
5. Mesh ``type="model"`` - an arbitrary mesh made of triangles. The geometry can be specified separately for the physics computation and the rendering. If only physical geometry is specified it is also used for rendering. The geometry can be loaded from STL or OBJ files (ASCII format). The origin tag is used to apply local transformation to the geometry, i.e., transformation in the frame defined by the 3D software used to save the geometry.

.. code-block:: xml

    <physical>
        <mesh filename="statue_phy.obj" scale="1.0"/>
        <origin rpy="0.0 0.0 0.0" xyz="0.0 0.0 0.0"/> 
    </physical>
    <visual> <!-- optional -->
        <mesh filename="statue_gra.obj" scale="1.0"/>
        <origin rpy="0.0 0.0 0.0" xyz="0.0 0.0 0.0"/>
    </visual>


Terrain
-------

6. Terrain `type="terrain"` - a heightfield terrain built based on a bitmap. Scale of the terrain is defined in meters per pixel.

.. code-block:: xml

    <height_map filename="terrain.png"/>
    <dimensions scalex="0.1" scaley="0.1" height="10.0"/>

_Following the above instructions, an exemplary static cylinder can be defined as:_

.. code-block:: xml

    <static name="Cylinder1" type="cylinder">   
        <dimensions radius="1.0" height="2.0"/>
        <material name="Aluminium"/>
        <look name="yellow"/>
        <world_transform rpy="0.0 0.0 0.0" xyz="0.0 0.0 2.0"/>
    </static>



