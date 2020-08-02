==============================
Defining a simulation scenario
==============================

The simulation scenario can be defined in two ways: by writing code or by using a scenario file parser. Both ways can also be combined, by first loading the scenario from a file and then modifying it using code. This gives access to the full functionality of the library, allows for non-standard definitions, as well as opens the possibility of using extensions developed for the library. In all cases, the definitions have to be placed inside the ``void BuildScenario()`` method of your subclass of ``sf::SimulationManager``.

Using the scenario file parser
==============================

For the convenience of the users, the *Stonefish* library implements a parser class for loading simulation scenarios from specially formulated scenario files. It is the preferred method of setting up the simulation scenarios, without the need of writing code. Loading a scenario from a file can be achieved through the following lines of code:

.. code-block:: cpp

    sf::ScenarioParser parser(this);
    parser.Parse("path_to_scenario_file");

Scenario file syntax
--------------------

Scenario files are `XML <https://www.w3.org/XML/>`_ files, with a syntax similar to the `URDF <http://wiki.ros.org/urdf>`_. Due to the unique features of the *Stonefish* library, some of the tags used in the scenario files are specific to it, and different from other formats. Every scenario file has to contain a root node called ``<scenario>``, i.e., all definitions have to be written between the tags ``<scenario> ... </scenario>``. It is possible to include one file in another to allow for the reuse of common definitions. The includes can only be used at the root level, by writing ``<include file="path_to_file"/>``. All paths defined in the scenario files are automatically recognised as absolute if they begin with ``/`` or ``~``, or as relative to the data directory passed to the constructor of the ``sf::SimulationApp`` otherwise. Whenever an attribute requires passing multiple values, these values have to be separated by spaces, e.g., ``<world_transform rpy="0.0 3.1415 0.0" xyz="1.0 2.0 3.0"/>``.

A properly defined scenario file has to contain a set of obligatory tags, specifying the type of the simulated environment as well as the list of the materials and looks used throguhout the scenario. The rest of the definitions are used to actually create the simulated static bodies, dynamics bodies and robots. A skeleton of a scenario file is shown below:

.. code-block:: xml

    <?xml version="1.0"?>
    <scenario>
        <environment>
            <!-- Parameters of simulated environment -->
        </environment>
        <materials>
            <!-- Definitions of materials -->
            <friction_table>
                <!-- Interaction between materials -->
            </friction_table>
        </materials>
        <looks>
            <!-- Definitions of looks -->
        </looks>
        <!-- Definitions used to build the simulated world -->
    </scenario>

Using the code
==============

Using the code to create simulation scenarios has a few possible benefits. First of all, it is necessary in case of extending the functionality of the *Stonefish* library, e.g., with new sensors, actuators or communication devices. This necessity can be dropped if the parser class is extended to include this new functionality. The other option is to load the standard definitions from a scenario file and add the missing elements with code. Secondly, the library code might expose properties and functions not supported by the parser, which may happen due to the difficulty in defining a particular functionality through the scenario file syntax. Finally, using code allows for implementing dynamically created simulation worlds, possibly with parametric functions, random distribution of bodies, generated terrain, etc.

When no scenario file is used, all of the obligatory definitions have to be written with code, in a specific order. Naturally, the materials and looks have to be defined before they can be used, which is not the case with the scenario file, in which the order of the tags does not matter. Moreover, the programmer is fully responsible for the correctness of the defined scenario, as any error checking mechanisms, implemented inside the parser, are not working anymore.


.. note::

    The rest of the documentation describes in detail how to define the obligatory properties of a simulation world, as well as every implemented object, that can be used in a simulation scenario. Each of the descriptions is accompanied by an XML snippet and its C++ twin, showing how to create objects using the scenario file syntax or the code.