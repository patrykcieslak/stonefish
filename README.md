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
    * **Bullet Physics library** (version 2.87, from source)
        1. Download from https://github.com/bulletphysics/bullet3/releases/tag/2.87
        2. cd bullet3
        3. mkdir build
        4. cd build
        5. cmake -DBUILD_PYBULLET=OFF -DBUILD_SHARED_LIBS=ON -DUSE_DOUBLE_PRECISION=ON -DCMAKE_BUILD_TYPE=Release .. 
        6. make -j4
        7. sudo make install     
    * **OpenGL Mathematics library** (libglm-dev, version >= 0.9.9.3)
    * **SDL2 library** (libsdl2-dev, needs the following fix!)
        1. Install SDL2 library from the repository
        2. cd /usr/lib/x86_64-linux-gnu/cmake/SDL2/
        3. sudo vim sdl2-config.cmake
        4. Remove space after "-lSDL2"
        5. Save file
    * **Eigen3 library** (libeigen3-dev, version >= 3.3.4)

2. Building
    1. clone stonefish repository
    2. cd stonefish
    3. mkdir build
    4. cd build
    5. cmake ..
    6. make -j4
    7. sudo make install

3. Documentation
    1. go to stonefish directory
    2. doxygen doxygen
    3. open docs/html/index.html