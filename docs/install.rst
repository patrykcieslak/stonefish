============
Installation
============

Requirements
============

The *Stonefish* library requires a **modern multi-core processor** to run the physics calculations in realtime. A **powerful GPU** with the support for **OpenGL 4.3 or higher** is required to run graphical simulations. If the GPU is not fulfilling the requirements it is still possible to run simulation in console mode, with limitted functionality.

Dependencies
============

The following dependencies have to be installed prior to building the library:

-  `OpenGL Mathematics <https://glm.g-truc.net/>`_ (libglm-dev, version >= 0.9.9.0)
-  `SDL2 <https://www.libsdl.org/>`_ (libsdl2-dev)

.. note::
    
    SDL2 library may need a small fix to the CMake configuration file, to avoid build errors. Remove a space after ``-lSDL2`` in ``/usr/lib/x86_64-linux-gnu/cmake/SDL2/sdl2-config.cmake``.

-  `Freetype <https://www.freetype.org>`_ (libfreetype6-dev)

Building
========

The easiest way to build the library is to let `CMake <https://cmake.org>`_ do its job at configuring the build process.
The standard build configuration includes building the dynamic library for system-wide use and creating 
the *install* target for make. The installation includes the library binary, header files and internal resources. 
It is possible to define the install location by modifying the standard variable ``CMAKE_INSTALL_PREFIX``, through the command line or the *cmake-gui* tool.

There are two special build options defined for CMake:

1) ``BUILD_TESTS``
    -  build dynamic library for local use, without an option for system-wide installation
    -  set path of internal resources to the source code location
    -  build tests/examples of simulators
2) ``EMBED_RESOURCES``
    -  generate C++ code from all internal resources
    -  compile the resources and embed them inside the library binary file
    -  no need to install resources as files in the shared system location
    -  useful for a binary release

The following terminal commands are necessary to clone, build and install the library with a standard configuration (*X* number of cores to use):
 
.. code-block:: console
    
    $ git clone "https://github.com/patrykcieslak/stonefish.git"
    $ cd stonefish
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make -jX
    $ sudo make install

Generating code documentation
=============================

The following steps are needed to generate and open the documentation of the library code. It is assumed that `Doxygen <https://www.doxygen.nl>`_ is already installed in the system.

1) Go to "stonefish" directory.
2) ``$ doxygen doxygen``
3) Open "docs/html/index.html".

3rd party code
==============

The following 3rd party code is included in the source of the library and will be updated manually by the author: 

-  `Bullet Physics <https://pybullet.org/wordpress/>`_ (C++ library, parts)
-  `TinyXML-2 <http://www.grinninglizard.com/tinyxml2/>`_ (C++ library)
-  `stb_image <https://github.com/nothings/stb>`_ (C library)
-  `stb_image_write <https://github.com/nothings/stb>`_ (C library)
-  OpenGL 4.6 functions loader generated with `GLAD2 <https://gen.glad.sh>`_
-  `ResourceManager <https://github.com/Johnnyborov/ResourceManager>`_ (C++ class and tool)