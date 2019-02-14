# ***Stonefish***
## An advanced simulation tool developed for marine robotics.

Stonefish is a C++ library which wraps around Bullet Physics library to deliver intuitive simulation of marine robots in reallistic scenarios.   
It is directed towards researchers in the field of marine robotics but can be used as a general robot simulator. 

Stonefish can be used to create standalone applications or combined with a ROS package *stonefish_ros* which implements 
simulator templates and facilitates easy integration with ROS architecture.

Stonefish includes advanced hydrodynamics computation based on actual geometry of bodies to better approximate hydrodynamic forces and allow for effects not possible when using symbolic models.
The rendering pipeline, developed from the ground up, delivers reallistic rendering of atmosphere, ocean and underwater environment. 
Special focus was put on the latter, where effects of wavelength-dependent light absorption and scattering were considered (other simulators often use only blue fog). 

### Installation
1. Dependencies
    * Bullet Physics library (version 2.87, from source)
    * OpenGL Mathematics library (GLM, version >= 0.9.9.3, from source)
    * SDL2 library (from repository)
    * Eigen library (version >= 3.3.4)

2. Building
    1. clone stonefish repository
    2. cd stonefish
    3. mkdir build
    4. cd build
    5. cmake ..
    6. make
    7. sudo make install

3. Documentation
    1. go to stonefish directory
    2. doxygen doxygen
    3. open docs/html/index.html