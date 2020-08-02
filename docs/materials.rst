=========
Materials
=========

There are two kinds of materials that can be defined in the simulation scenario. T


Physical materials
==================

There are two kinds of materials that can be defined: physical and graphical. Physical materials define properties of bodies such as density and restitution and thus allow for automatic calculation of mass and inertia, based on geometry. There is also a mechanism to define friction character between bodies, by setting static and dynamic friction coefficients between all possible material pairs. On the other hand, graphical materials define how the objects are rendered, thus they only exist in graphical mode simulations. All materials are rendered with a Cook-Torrance model, parametrised by reflectance, roughness, metallicity and reflection factor (0.0 = no reflections, 1.0 = mirror). Additionally, it is possible to attach a texture to a material, which will be used as reflectance (multiplied by the reflectance colour). It should be noticed that physical and graphical materials are not connected, i.e., an arbitrary pair can be applied to each body.

Use the following code to define physical materials:

.. code-block:: cpp

    CreateMaterial("Aluminium", sf::Scalar(2791), sf::Scalar(0.7));
    CreateMaterial("Steel", sf::Scalar(7891), sf::Scalar(0.9));

The above definition of materials requires the following definition of friction interaction:

.. code-block:: cpp

    SetMaterialsInteraction("Aluminium", "Aluminium", sf::Scalar(0.7), sf::Scalar(0.4));
    SetMaterialsInteraction("Steel", "Steel", sf::Scalar(0.4), sf::Scalar(0.1));
    SetMaterialsInteraction("Aluminium", "Steel", sf::Scalar(0.8), sf::Scalar(0.5));

.. code-block:: xml

    <materials>
        <material name="Fiberglass" density="1500.0" restitution="0.3"/> <!-- definition of a material -->
        <material name="Aluminium" density="2710.0" restitution="0.7"/>
        <friction_table> <!-- definition of friction coefficients between all material pairs -->
        <friction material1="Fiberglass" material2="Fiberglass" static="0.5" dynamic="0.2"/>
        <friction material1="Fiberglass" material2="Aluminium" static="0.3" dynamic="0.05"/>
        <friction material1="Aluminium" material2="Aluminium" static="0.8" dynamic="0.5"/>
        </friction_table>
    </materials>

Looks
=====

Use the following code to define a graphical material:

.. code-block:: cpp

    CreateLook("red", sf::Color::RGB(1.f,0.f,0.f), 0.1f, 0.0f, 0.f, "texture.png");

.. code-block:: xml

    <looks>
        <look name="yellow" rgb="1.0 0.9 0.0" roughness="0.3"/>
        <look name="gray" gray="0.3" roughness="0.4" metalness="0.5"/>
        <look name="textured" gray="1.0" roughness="0.3" texture="tex.png"/>
    </looks>

When the obligatory elements are defined, the description should be filled with definitions of static bodies, dynamic bodies and robots. When defining these elements, the names of the created materials and looks have to be used (case sensitive).
