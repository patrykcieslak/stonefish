=========
Materials
=========

Before the environment and the robots can be created, two groups of materials have to be defined in the simulation scenario. These groups are: the physical materials, defining the physical properties of the bodies, and the graphical materials (looks), defining how the bodies will look. The physical materials and the looks are defined separately to allow for arbitrary mixing between them, when creating the bodies. Moreover, bodies made of the same physical material may require distinctive looks. 

.. note::
    
    The names of the defined physical materials and looks have to be used when creating the objects in the simulation scenario (case-sensitive). Therefore, the materials have to be defined prior to the definition of any bodies.

Physical materials
==================

Physical materials are characterised by physical properties: the density and the restitution factor. This allows for automatic calculation of mass and inertia of the bodies, based on their geometry. The restitution factor defines the fraction of momentum preserved during rigid collisions. It also affects the way that the body reflects the sonar waves, as they are mechanical in nature. There is also a mechanism to define frictional interaction between different materials, by setting static and dynamic friction coefficients between all possible material pairs. 

An example of the definition of two physical materials, using the XML syntax, is presented below:

.. code-block:: xml

    <materials>
        <material name="Aluminium" density="2710.0" restitution="0.7"/>
        <material name="Steel" density="7891.0" restitution="0.9"/>
        <friction_table> 
            <friction material1="Aluminium" material2="Aluminium" static="0.7" dynamic="0.4"/>
            <friction material1="Steel" material2="Steel" static="0.4" dynamic="0.1"/>
            <friction material1="Aluminium" material2="Steel" static="0.8" dynamic="0.5"/>
        </friction_table>
    </materials>

The same can be achieved using the following code:

.. code-block:: cpp

    CreateMaterial("Aluminium", sf::Scalar(2791), sf::Scalar(0.7));
    CreateMaterial("Steel", sf::Scalar(7891), sf::Scalar(0.9));
    SetMaterialsInteraction("Aluminium", "Aluminium", sf::Scalar(0.7), sf::Scalar(0.4));
    SetMaterialsInteraction("Steel", "Steel", sf::Scalar(0.4), sf::Scalar(0.1));
    SetMaterialsInteraction("Aluminium", "Steel", sf::Scalar(0.8), sf::Scalar(0.5));    

Looks
=====

The graphical materials, called looks, define how the objects are rendered, thus they only exist in graphical mode simulations. All looks are rendered with a Cook-Torrance model, parametrised by reflectance (color), roughness, metalness and reflection factor (0.0 = no reflections, 1.0 = mirror). Additionally, it is possible to attach two textures to a material: an albedo texture and a normal texture. The albedo texture is multiplied by the reflectance to define the color of the material. The normal texture (map) is used to introduce surface details which would otherwise require complex geometry (bump mapping). The normal vectors computed from the geometry are rotated locally, based on the normal map, before the lighting calculations are performed. The normal map also affects the normals seen by the sonar, which allows for capturing realistic surface textures in the virtual sonar images.

Example of defining different looks, using the XML syntax, is presented below:

.. code-block:: xml

    <looks>
        <look name="Yellow" rgb="1.0 0.9 0.0" roughness="0.3"/>
        <look name="Gray" gray="0.3" roughness="0.4" metalness="0.5"/>
        <look name="Textured" gray="1.0" roughness="0.1" texture="tex.png"/>
        <look name="Bump" rgb="1.0 0.0 0.0" roughness="0.2" normal_map="normal.png"/>
    </looks>

The same can be achieved using the following code:

.. code-block:: cpp

    CreateLook("Yellow", sf::Color::RGB(1.f, 0.9f, 0.f), 0.3f);
    CreateLook("Gray", sf::Color::Gray(0.3f), 0.4f, 0.5f);
    CreateLook("Textured", sf::Color::Gray(1.f), 0.1f, 0.f, 0.f, sf::GetDataPath() + "tex.png");
    CreateLook("Bump", sf::Color::RGB(1.f, 0.f, 0.f), 0.2f, 0.f, 0.f, "", sf::GetDataPath() + "normal.png");
    
.. note::

    Function ``std::string sf::GetDataPath()`` returns a path to the directory storing simulation data, specified during the construction of the ``sf::SimulationApp`` object. 