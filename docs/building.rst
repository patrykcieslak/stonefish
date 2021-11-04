====================
Building a simulator
====================

Types of simulators
===================

The *Stonefish* library is designed to build simulators for specific scenarios, by subclassing a minimal number of classes and overriding as few methods as possible. Depending on the functionality that is requested it can be as little as one class and one method. Moreover, there are two different kinds of simulators that can be built: a *console mode* simulator and a *graphical mode* simulator. A *console mode* simulator does not provide any functionality that requires graphics, which includes not only visualisation of the simulated scenario but also simulation of cameras, lights, depth map based sensors and waves. This kind of simulators can run on platforms which do not conform to the minimum requirements of the rendering pipeline. The normal mode of operation of the simulators is graphical.

.. note::
    
    The *Stonefish* repository contains examples of different types of simulators in the ``Tests`` directory. These examples range from simple dynamic bodies falling on a groud plane to a full model of an autonomous underwater vehicle (AUV).

Building a simple graphical simulator
=====================================

The following steps have to be completed to create a simple graphical simulator:

1) Create a new class ``MySimulationManager`` being a subclass of ``sf::SimulationManager``.
2) Override method ``void BuildScenario()`` of the base class, **creating the whole simulation scenario inside**.
3) Create a .cpp file with the ``int main(int argc, char **argv)`` function.
4) Include the header files: ``#include <Stonefish/core/GraphicalSimulationApp.h>`` and ``#include "MySimulationManager.h"``.
5) Create and fill two structures of types ``sf::RenderSettings`` and ``sf::HelperSettings``.
6) Create a new object of class ``MySimulationManager``.
7) Create a new object of class ``sf::GraphicalSimulationApp``, passing the previous one to the constructor, together with the structures defined before.
8) Use method ``void Run()`` from the second object.
9) Build application and link with libraries.
10) Run the simulator from the terminal.

Following the steps above should result in 3 new files: *main.cpp*, *MySimulationManager.h* and *MySimulationManager.cpp*. The exemplary contents of these files are attached below. The simulated scenario is a ball bouncing on a flat surface.

*main.cpp*:

.. code-block:: cpp

    #include <Stonefish/core/GraphicalSimulationApp.h>
    #include "MySimulationManager.h"

    int main(int argc, char **argv)
    {
        //Using default settings
        sf::RenderSettings s;
        sf::HelperSettings h;
        
        MySimulationManager manager(500.0);
        sf::GraphicalSimulationApp app("Simple simulator", "path_to_data", s, h, &manager);
        app.Run();

        return 0;
    }

*MySimulationManager.h*:

.. code-block:: cpp

    #include <Stonefish/core/SimulationManager.h>

    class MySimulationManager : public sf::SimulationManager
    {
    public:
        MySimulationManager(sf::Scalar stepsPerSecond);
        void BuildScenario();
    };

*MySimulationManager.cpp*

.. code-block:: cpp

    #include "MySimulationManager.h"
    #include <Stonefish/entities/statics/Plane.h>
    #include <Stonefish/entities/solids/Sphere.h>

    MySimulationManager::MySimulationManager(sf::Scalar stepsPerSecond) : SimulationManager(stepsPerSecond)
    {
    }

    void MySimulationManager::BuildScenario()
    {
        //Physical materials
        CreateMaterial("Aluminium", 2700.0, 0.8);
        CreateMaterial("Steel", 7810.0, 0.9);
        SetMaterialsInteraction("Aluminium", "Aluminium", 0.7, 0.5);
        SetMaterialsInteraction("Steel", "Steel", 0.4, 0.2);
        SetMaterialsInteraction("Aluminium", "Steel", 0.6, 0.4);

        //Graphical materials (looks)
        CreateLook("gray", sf::Color::Gray(0.5f), 0.3f, 0.2f);
        CreateLook("red", sf::Color::RGB(1.f,0.f,0.f), 0.1f, 0.f);
        
        //Create environment
        sf::Plane* plane = new sf::Plane("Ground", 10000.0, "Steel", "gray");
        AddStaticEntity(plane, sf::I4());

        //Create object
        sf::Sphere* sph = new sf::Sphere("Sphere", 0.1, sf::I4(), "Aluminium", sf::BodyPhysicsType::SURFACE, "red");
        AddSolidEntity(sph, sf::Transform(sf::IQ(), sf::Vector3(0.0,0.0,-1.0)));
    }

Interacting with the simulator
==============================

Physical controllers
--------------------

The *graphical mode* simulators can be interacted with using physical controllers - keyboard and/or joystick/gamepad. 
To implement this physical interaction it is necessary to subclass ``sf::GraphicalSimulationApp``. Some of the base class methods need to be overridden. When keyboard is to be used as a controller the methods to override are called ``void KeyDown(SDL_Event* event)`` and ``void KeyUp(SDL_Event* event)``. Care has to be taken when overriding the ``KeyDown()`` method, because its base class version implements some default keyboard functionality. If this functionality is to be retained the base class method has to be called in the subclass.
If the joystick/gamepad support is to be implemented, the methods to override are called ``void JoystickDown(SDL_Event* event)``, ``void JoystickUp(SDL_Event* event)`` and ``void ProcessInputs()``. The former two are used to implement joystick button functionality while the latter can be used to read values of the joystick axes.

.. note::
    If the standard keyboard handling was not overridden, the ``w s a d z x`` keys can be used to manipulate the viewport. The mouse can be used to select objects (left button), rotate the viewport (right button) and move the rotation centre (middle button). 

Internal or external code
-------------------------

Any type of simulator will probably require some interaction with internal or external code. This can be a control algorithm implemented inside the simulator application or another application that requests data from the simulator, like sensor readings, and/or wants to modify actuator setpoints. To ensure consistency of the simulation results this data can only be read and written at specific moments in time. To facilitate easy interaction the class ``sf::SimulationManager`` provides a virtual method ``void SimulationStepCompleted(Scalar timeStep)``, which is called by the physics engine after a single simulation step is completed. Since the base class has to be subclassed to build a simulation scenario, it is easy to override another method for the interaction purposes.

Robot Operating System (ROS)
----------------------------

One of the anticipated uses of the simulators built with the *Stonefish* library, is their use in combination with ROS. The simulator can substitute the real robot during the research and development phases, transparently hooking up to the complex, ROS-based, control and navigation architectures. The interfacing is done using the same basic idea of overriding the ``void SimulationStepCompleted(Scalar timeStep)`` method. The author provides a ROS package called `stonefish_ros <https://github.com/patrykcieslak/stonefish_ros>`_, delivering a standard simulator node, to simplify the integration.

Graphical user interface (GUI)
==============================

The *Stonefish* library uses the concept of an immediate-mode GUI (IMGUI), which is displayed in the simulation window. It is a non-retained user interface, which means that the programmer is responsible for all data management, and the GUI is always rendered based on the current data. The IMGUI offers a few basic widgets to enable displaying and manipulating simulation parameters as well as plotting sensor measurements. There is a standard implementation of the IMGUI, displayed on the left side of the window, which can be overwriten or enhanced. Apart from the standard widgets, a text console is implemented which is always displayed during the simulator startup and can be used to check for errors and warnings.

.. note::

    If the standard keyboard handling was not overridden, the ``h`` key can be used to show/hide the IMGUI and the ``c`` key can be used to show/hide the console. The console can be scrolled using the mouse.

.. note::

    If the standard GUI was not overridden, a keymap of the standard keyboard commands can be displayed hitting the ``k`` key, in the right bottom corner of the simulation window.

Customising the IMGUI
---------------------

To customise the IMGUI, the class ``sf::GraphicalSimulationApp`` has to be subclassed and the method ``void DoHUD()`` has to be overridden. Every widget has to use a unique ``sf::ui_id``, which allows for identification of active IMGUI elements.

The available widgets include: 

- ``Panel`` a box that may be used to group items visually.
- ``Label`` a static text
- ``ProgressBar`` an indicator which shows progress in form of a partially filled rectangle
- ``Button`` a momentary button that can be pressed
- ``Slider`` slider that can be used to set a continuous parameter
- ``CheckBox`` a checkbox that can be used to enable/disable options
- ``ComboBox`` a box with multiple selectable options
- ``TimePlot`` a plot which can display data from one or more sensor channels, with a common time axis
- ``XYPlot`` a plot which can display a relation between two sensor channels

Example of creating a button widget (*note:* ``MySimulationApp`` is a subclass of ``sf::GraphicalSimulationApp``):

.. code-block:: cpp

    void MySimulationApp::DoHUD()
    {
        GraphicalSimulationApp::DoHUD(); //Keep standard GUI

        sf::ui_id button;
        button.owner = 1; //e.g. id of a panel
        button.index = 0; //e.g. id of a widget on the panel
        button.item = 0; //e.q. id of an option on a list

        if(getGUI()->DoButton(button, 200, 10, 200, 50, "Press me"))
            code_to_execute;
    }