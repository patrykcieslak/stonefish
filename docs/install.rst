============
Installation
============

Requirements
============

The *Stonefish* library requires a modern multi-core processor to run the physics calculations in realtime.

A discrete graphics card with the support for **OpenGL 4.3 or higher** is required to run graphical simulations.

Dependencies
============

The following dependencies have to be installed prior to building the library:

* `Bullet Physics <https://pybullet.org>`_ (version 2.89, from source)

.. code-block:: console

    $ git clone "https://github.com/bulletphysics/bullet3.git" -b 2.89
    $ cd bullet3
    $ mkdir build
    $ cd build
    $ cmake -DBUILD_PYBULLET=OFF -DBUILD_SHARED_LIBS=ON -DUSE_DOUBLE_PRECISION=ON -DCMAKE_BUILD_TYPE=Release ..
    $ make -jX
    $ sudo make install

* `OpenGL Mathematics <https://glm.g-truc.net/>`_ (libglm-dev, version >= 0.9.9.0)

* `SDL2 <https://www.libsdl.org/>`_ (libsdl2-dev)

.. note::
    
    SDL2 library may need a small fix to the cmake configuration file, to avoid build errors. Remove a space after ``-lSDL2`` in ``/usr/lib/x86_64-linux-gnu/cmake/SDL2/sdl2-config.cmake``.

* `Freetype <https://www.freetype.org>`_ (libfreetype6-dev)

Building
========

The following steps are needed to build the library:
 
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

The following steps are needed to generate and open the documentation of the library code.

1. Go to "stonefish" directory.
2. ``$ doxygen doxygen``
3. Open "docs/html/index.html".