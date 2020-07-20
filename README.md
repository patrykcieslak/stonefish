![Stonefish logo](https://github.com/patrykcieslak/stonefish/blob/master/Library/shaders/logo_64.png)
# ***Stonefish***
### An advanced simulation tool developed for marine robotics.

Stonefish is a C++ library which wraps around Bullet Physics library to deliver intuitive simulation of marine robots in realistic scenarios. It is directed towards researchers in the field of marine robotics but can as well be used as a general robot simulator. 

Stonefish includes advanced hydrodynamics computation based on actual geometry of bodies to better approximate hydrodynamic forces and allow for effects not possible when using symbolic models. The rendering pipeline, developed from the ground up, delivers realistic rendering of atmosphere, ocean and underwater environment. Special focus was put on the latter, where effects of wavelength-dependent light absorption and scattering were considered (other simulators often use only blue fog). 

Stonefish can be used to create standalone applications or combined with a ROS package [_stonefish_ros_](https://github.com/patrykcieslak/stonefish_ros), which implements 
simulator templates and facilitates easy integration with ROS architecture.

There are two sources of documentation for the library: a [repository Wiki](https://github.com/patrykcieslak/stonefish/wiki) and a doxygen generated website, based on comments in the code (instructions below).

### Requirements

The simulation is CPU heavy and requires a recent GPU. The minimum requirement is the support for *OpenGL 4.3*. 

Install official manufacturer drivers for your graphics card before using _Stonefish_!

The software is developed and tested on *Linux Ubuntu*. It should work on any Unix based platform. A version for Windows is not available at this time.

### Installation
1. Dependencies
    * **Bullet Physics library** (version 2.89, from source)
        1. `git clone https://github.com/bulletphysics/bullet3.git -b 2.89`
        2. `cd bullet3`
        3. `mkdir build`
        4. `cd build`
        5. `cmake -DBUILD_PYBULLET=OFF -DBUILD_SHARED_LIBS=ON -DUSE_DOUBLE_PRECISION=ON -DCMAKE_BUILD_TYPE=Release ..`
        6. `make -j4`
        7. `sudo make install`
    * **OpenGL Mathematics library** (libglm-dev, version >= 0.9.9.0)
    * **SDL2 library** (libsdl2-dev, may need the following fix!)
        1. Install SDL2 library from the repository.
        2. `cd /usr/lib/x86_64-linux-gnu/cmake/SDL2/`
        3. `sudo vim sdl2-config.cmake`
        4. Remove space after "-lSDL2".
        5. Save file.
    * **Freetype library** (libfreetype6-dev)

2. Building
    1. Clone _stonefish_ repository.
    2. `cd stonefish`
    3. `mkdir build`
    4. `cd build`
    5. `cmake ..`
    6. `make -jX` (where X is the number of threads)
    7. `sudo make install`

3. Documentation
    1. Go to "stonefish" directory.
    2. `doxygen doxygen`
    3. Open "docs/html/index.html".
    
### Credits
This software was written and is continuously developed by Patryk Cieślak. Small parts of the software based on code developed by other authors are marked clearly as such.

If you find this software useful in your research, please cite:

*Patryk Cieślak, "Stonefish: An Advanced Open-Source Simulation Tool Designed for Marine Robotics, With a ROS Interface", In Proceedings of MTS/IEEE OCEANS 2019, June 2019, Marseille, France*
```
@inproceedings{stonefish,
   author = {Cie{\'s}lak, Patryk},
   booktitle = {OCEANS 2019 - Marseille},
   title = {{Stonefish: An Advanced Open-Source Simulation Tool Designed for Marine Robotics, With a ROS Interface}},
   month = jun,
   year = {2019},
   doi={10.1109/OCEANSE.2019.8867434}}
```
### Funding
This work was part of a project titled ”Force/position control system to enable compliant manipulation from a floating I-AUV”, which received funding from the European Community H2020 Programme, under the Marie Sklodowska-Curie grant agreement no. 750063. The work was continued under a project titled ”EU Marine Robots”, which received funding from the European Community H2020 Programme, grant agreement no. 731103.

### License
This is free software, published under the General Public License v3.0.

### Change log

*Version 0.9*
- Requires support for OpenGL 4.3+ (compute shaders)
- Complete rewrite of the ocean/underwater rendering pipeline
- New automatic exposure and anti-aliasing algorithms
- Logarythmic depth buffer for planet scale rendering
- Fully GPU-based simulation of forward-looking sonar (FLS)
- Fully GPU-based simulation of side-scan sonnar (SSS)
- Normal mapping to enable high resolution surface details
- Faster download of data from the GPU memory
- Scheduling of multiple view rendering
- OpenGL function handlers provided through GLAD