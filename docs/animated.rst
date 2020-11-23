.. _animated-bodies:

===============
Animated bodies
===============

The animated bodies are rigid bodies that move along a predefined trajectory in space, instead of following the laws of physics. Their position, orientation and velocities can also be set manually by the user. The physical body definition is not obligatory for an animated body as it can represent just a moving frame. Animated bodies can be useful in cases where body/robot dynamics are not important or solved externally, and the focus is put on simulation of the attached sensors. Using an animated body in this situation allows for a completely repeateable result. Another use of animated bodies is to represent moving parts of the environment, that the dynamic bodies can collide with. Using animated bodies instead of dynamic ones makes sense if the simulation of object dynamics would be unstable/inaccurate (very large mass/dimensions) or the user deserves a particular behaviour, for example, based on measured data. 

Animated bodies are defined in the XML syntax using the ``<animated> ... </animated>`` tags, in a following way:

.. code-block:: xml

    <animated name="{1}" type="{2}" collisions="{3}">
        <!-- type specific definitions -->
        <material name="{4}"/>
        <look name="{5}"/>
        <trajectory type="{6}" playback="{7}">
            <!-- key points -->
        </trajectory>
        <!-- sensor definitions (optional) -->
    </animated>

where

1) **Name**: unique string

2) **Type**: type of the animated body

3) **Collisions**: flag defining if body should collide with dynamic bodies

4) **Material name**: the name of the physical material

5) **Look name**: the name of the graphical material

6) **Trajectory type**: the type of the trajectory generator

7) **Playback mode**: the playback mode of the trajectory generator

.. note:: 

    In the following examples it is assumed that physical material called "Steel" and look called "Yellow" were defined.

Trajectory definition
=====================

Animated bodies require a definition of a trajectory to follow, when the simulation is started. The type of the generated trajectory and its playback mode are specified as attributes of the ``<trajectory>`` tag and the definition of the trajectory **key points** is placed inside ``<trajectory> ... </trajectory>``. The following **trajectory types** are available:

- **Manual** ``type="manual"`` - the body is not moved automatically, its transformation and velocities have to be specified manually by calling a function.

- **Piece-wise linear** ``type="pwl"`` - position and orientation of the body are interpolated linearly. Linear and angular velocities are computed based on time differences between the key points.

- **Spline** ``type="spline"`` - position of the body is interpolated using a Catmull-Rom spline, orientation of the body is interpolated linearly. Linear velocities are computed as 1st order derivatives of the Catmull-Rom spline, while angular velocities using simple differentation, both based on time differences between the key points.

When any of the trajectories, other than the manual, is selected, the body is animated automatically along it, with three possible **playback modes**:

- **One time** ``playback="onetime"`` - the animation plays one time from the start of the simulation.

- **Repeat** ``playback="repeat"`` - the animation is repeated indefinitely. When the body reaches the end of the trajectory it jumps to the beginning.

- **Boomerang** ``playback="boomerang"`` - the animation is reapeated indefinitely. When the body reaches the end of the trajectory it changes the direction of motion and goes back to the beginning.

Each trajectroy is built of key points. Each key point is characterised by the time at which it has to be reached and the position and orientation of the body in the world frame to be achieved. Defining the trajectory **key points** is done by using the following syntax:

.. code-block:: xml

    <keypoint time="{1}" xyz="{2a}" rpy="{2b}/>

where {1} specifies time since the beginning of the simulation and {2} is the pose of the body in the world frame. In case of the manual trajectory only the key point for ``time="0"`` is taken into account, as the starting pose of the body.

An exemplary trajectory can be defined using the following XML syntax:

.. code-block:: xml

    <trajectory type="spline" playback="repeat">
        <keypoint time="0.0" xyz="0.0 1.0 2.0" rpy="0.0 0.0 0.0"/>
        <keypoint time="5.0" xyz="10.0 1.0 2.0" rpy="0.0 0.0 1.57"/>
        <keypoint time="20.0" xyz="10.0 2.0 2.0" rpy="0.0 0.0 0.0"/>
    </trajectory>

The same can be achieved using the following code:

.. code-block:: cpp

    sf::CRTrajectory* traj = new sf::CRTrajectory(sf::PlaybackMode::REPEAT);
    traj->AddKeyPoint(0.0, sf::Transform(sf::IQ(), sf::Vector3(0.0, 1.0, 2.0)));
    traj->AddKeyPoint(5.0, sf::Transform(sf::Quaternion(1.57, 0.0, 0.0), sf::Vector3(10.0, 1.0, 2.0)));
    traj->AddKeyPoint(20.0, sf::Transform(sf::IQ(), sf::Vector3(10.0, 2.0, 2.0)));

Moving frame
============

This kind of animated body ``type="empty"`` does not represent any physical body and as a result it is not colliding by definition. Consequently, it does not require material or look definitions as well. It can be viewed as a moving coordinate frame. No specific definitions are needed here.

The definition of a moving frame using the XML syntax can look like this:

.. code-block:: xml

    <animated name="Frame" type="empty">
        <!-- trajectory definition (look up) -->
    </animated>

The same using the code:

.. code-block:: cpp

    sf::AnimatedEntity* anim = new sf::AnimatedEntity("Frame", traj);
    AddAnimatedEntity(anim);

Parametric solids
=================

When the user is interested in displaying a simple physical representation of the animated body, with the option to enable efficient collisions, one of the types belonging to the group of parametric solids can be used. The following type-specific definitions have to be used:

- **Sphere** ``type="sphere"`` - ball with a specified radius {1}:
 
.. code-block:: xml
  
    <dimensions radius="{1}"/>

- **Cylinder** ``type="cylinder"`` - cylinder along Z axis, with a specified radius {1} and height {2}:

.. code-block:: xml

    <dimensions radius="{1}" height="{2}"/>

- **Box** ``type="box"`` - box with specified width {1}, length {2} and height {3}: 

.. code-block:: xml
    
    <dimensions xyz="{1} {2} {3}"/>

Moreover, a definition of the body origin frame is required:

.. code-block:: xml

    <origin xyz="{1}" rpy="{2}"/>

where {1} and {2} represent position and orientation of the body origin frame, with respect to the natural frame of the solid.

A few examples of creating solid animated objects using the XML syntax follow:

.. code-block:: xml

    <animated name="AnimSphere" type="sphere" collisions="false">
        <dimensions radius="2.0"/>
        <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        <material name="Steel"/>
        <look name="Yellow"/>
        <!-- trajectory definition (look up) -->
    </animated>

    <animated name="AnimBox" type="box" collisions="true">
        <dimensions xyz="1.0 0.5 0.1"/>
        <origin xyz="0.5 0.0 0.1" rpy="0.0 0.0 0.0"/>
        <material name="Steel"/>
        <look name="Yellow"/>
        <!-- trajectory definition (look up) -->
    </animated>

The same can be achieved through the following code:

.. code-block:: cpp

    sf::AnimatedEntity* anim1 = new sf::AnimatedEntity("AnimSphere", traj, 2.0, sf::I4(), "Steel", "Yellow");
    AddAnimatedEntity(anim1);
    sf::AnimatedEntity* anim2 = new sf::AnimatedEntity("AnimBox", traj, sf::Vector3(1.0, 0.5, 0.1), sf::Transform(sf::IQ(), sf::Vector3(0.5, 0.0, 0.1)), "Steel", "Yellow", true);
    AddAnimatedEntity(anim2);

Arbitrary meshes
================

The animated body can also have an arbitrary geometry ``type="model"``, loaded from a geometry file. Its geometry can be specified separately for the computation of physics ``<physical> .. </physical>`` and the rendering ``<visual> ... </visual>``. The physics mesh should be optimised to improve collision performance.  If only physics geometry is specified, it is also used for rendering. Moreover, the physics mesh is used when simulating operation of :ref:`link sensors <link-sensors>` and the graphics mesh is used for the :ref:`vision sensors <vision-sensors>`. For more details on preparing mesh data check :ref:`preparing-geometry`.

Instantiation of an animated body based on mesh data can be done with the following XML syntax:

.. code-block:: xml

    <animated name="AnimMesh" type="model" collisions="false">
        <physical>
            <mesh filename="vehicle_phy.obj" scale="1.0"/>
            <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        </physical>
        <visual>
            <mesh filename="vehicle_vis.obj" scale="1.0"/>
            <origin xyz="0.0 0.0 0.0" rpy="0.0 0.0 0.0"/>
        </visual>
        <material name="Steel"/>
        <look name="Yellow"/>
        <!-- trajectory definition (look up) -->
    </animated>

The ``<origin>`` tag is used to apply local transformation to the geometry, i.e., to change the position of the mesh origin and rotate the mesh, before placing it in the world. It is especially useful in case of geometry exported from 3D software in a wrong frame.

The same definition in the code looks like this:

.. code-block:: cpp

    sf::AnimatedEntity* anim = new sf::AnimatedEntity("AnimMesh", traj, sf::GetDataPath() + "vehicle_vis.obj", 1.0, sf::I4(), sf::GetDataPath() + "vehicle_phy.obj", 1.0, sf::I4(), "Steel", "Yellow");
    AddAnimatedEntity(anim);

.. note::

    Function ``std::string sf::GetDataPath()`` returns a path to the directory storing simulation data, specified during the construction of the ``sf::SimulationApp`` object.