==========
Changelog
==========

The changelog of the library code is presented below. **Breaking changes** were marked with *italics*.

1.6
===

- Implemented soft body cables
- Added support for glueing dynamic bodies to robot links
- *Renamed multiple symbols in the library*

1.5
===

-  Implemented an event-based camera
-  Implemented an optical flow sensor
-  Implemented a segmentation camera
-  Implemented a thermal camera
-  *Implemented an optical modem*
-  *Improved processing of messages of all communication devices*
-  Extended look definition to support temperature maps
-  Added water temperature
-  Added air temperature, pressure, and humidity
-  Added a test application for all camera types
-  Updated marine snow rendering to use the same particle system for vision sensors attached to the same body
-  Removed failing framerate limitting and added option to enable vertical synchronisation
-  Fixed application of hydrodynamic drag coefficients to compound bodies
-  Fixed problems with vision sensor framerate not consistent with settings
-  Fixed switching on/off lights

1.4
===

-  *Reimplemented the thruster actuator to support combinations of different mathematical models*
-  *Rewritten computation of hydrodynamic drag*
-  *Simple thruster is now a new actuator class and displays a rotating propeller*
-  Implemented new trajectory generator for animated bodies utilising B-splines (now default)
-  Extended glue to support joining links of two Featherstone robots together
-  Extended fixed joint class to support updates of joint definition (dynamic activation/deactivation)
-  Added a watchdog timer to the actuators, including parser support
-  Added access to the viscous and quadratic hydrodynamic drag coefficients, including parser support
-  Added an option to set internal parts of a compound body as always visible
-  Added access to the computed wetted surface area and submerged volume
-  Added maximum angular rate of change of the rudder actuator angle, to represent the actuator's dynamics
-  Added an option to specify fluid dynamics computation prescaler, including parser support
-  *Fixed loading sRGB and linear textures (fixes normal map issues)*
-  Fixed ocean rendering error when switching between different views 
-  Fixed calculation and rendering of the ellipsoidal approximation used for added mass estimation
-  Fixed buoyancy force calculation for flat ocean (floating bodies are not rotating or moving anymore!)
-  Fixed IMU readings, adding the missing gravitational and centrifugal accelerations
-  Fixed rendering of vision sensor outputs for debug purposes
-  Fixed getting robot transform
-  Fixed acoustic modem implementation eliminating problem with modems not seeing each other
-  Fixed sonar update frequency implementation to allow for slow updates
-  Fixed Stonefish logo and icon

1.3
===

-  *Restructured the SimulationApp class and its children to support the new ROS2 interface*
-  *Reimplemented Robot definition to allow for two different algorithms: the Featherstone's algorithm and a general constraint solving algorithm; the general algorithm allows for kinematic loops in the robot structure*
-  *Added origin definition to standard obstacle solids to enable local transformations*
-  *Updated definition of accelerometer, gyroscope and IMU sensors, including parser support*
-  *Extended DVL model, with water layer velocity measurement and new noise models, including parser support*
-  Added easy access to the parameters of the constraint solver, including parser support
-  Implemented an INS combining internal gyroscopes and accelerometers with external sensors like DVL and GPS, including parser support
-  Implemented methods which enable live updates of sensor and actuator frames
-  Implemented magnetic interaction between materials to enable simulation of permanent magnets
-  Implemented parsing of mathematical expressions in scenario files
-  Improved support for console simulations
-  Improved support for non-realitime simulations
-  Separated underwater and above water rendering paths
-  Eliminated precomputation of atmospheric scattering (loaded from resources)
-  Improved ocean reflections
-  Reimplemented XML parser logging mechanism
-  Significantly improved XML parser error and warning messages (easier location of errors)
-  Extended implementation of velocity fields to facilitate online updates
-  Added optional functionality to embed internal resources in the library binary
-  Fixed spline interpolation of trajectories with subsequent overlapping points
-  Fixed measurement of accelerations
-  Fixed unstable multibody joint position control
-  Fixed computation of moments of inertia
-  Fixed trackball implementation - better zoom and translation of the main 3D view
-  Fixed mouse issues in the main 3D view

1.2
===

-  Animated bodies - bodies moving according to a predefined trajectory
-  Trajectory generators for animated bodies (piece-wise linear and spline interpolation)
-  Sensors can now be attached to all kinds of bodies, as well as the world frame
-  *New implementation of the 3-axis gyroscope, with a measurement bias*
-  *IMU implementation extended with yaw angle drift and per channel characteristics*
-  Noise definition for sonars and the depth camera
-  Sonar output reduced to 8 bit, to better reflect real sensors
-  *Lights can now be easily attached to any kind of body, as well as the world frame*
-  *New XML syntax for defining lights*
-  *Communication devices can now be attached to all kinds of bodies, as well as the world frame*
-  Fixed beam occlusion testing for acoustic comms and introduced option to disable it
-  *New implementation of the USBL, including measurement resolution*
-  Looks are now parsed from the included files
-  "Shift" key can be used to move the main window camera faster
-  Display of keymap in the GUI (press 'K')
-  Sun light shadows on ocean surface
-  Screen-space reflections quality settings
-  Fixed reflections on ocean surface
-  Fixed horizon rendering problems
-  Fixed particle motion
-  Fixed cascaded shadow mapping
-  Fixed depth camera minimum range

1.1
===

-  Removed external dependence on the Bullet Physics Library and included necessary parts in the source tree
-  Updated the mathematical models of the thruster and the propeller actuators
-  Optimised computation of the geometry-based hydrodynamics/aerodynamics
-  Implemented new visualisation of underwater currents (water velocity field)
-  Fixed crashes when trying to create marine actuators in a simulation without ocean
 
1.0
===

-  Fully GPU-based simulation of mechanical scanning imaging sonar (MSIS)
-  Improvements in all sonar simulations
-  Significant improvement to DVL performance when heightfield terrain is used
-  Heightfield terrain now supports 16 bit heightmaps
-  New syntax for loading ocean and atmosphere definitons using the XML parser
-  Support for arguments passed to the included files
-  New, complete, beautiful documentation generated with Sphinx

0.9
===

-  Moved to the OpenGL 4.3 functionality (compute shaders)
-  Complete rewrite of the ocean/underwater rendering pipeline
-  Light absorption and scattering in water based on Jerlov measurements
-  Full support of photo-reallistic sky and sunlight as well as point and spot lights
-  New, linear tree based, automatic LOD algorithm
-  New automatic exposure (histogram based) and anti-aliasing (FXAA) algorithms
-  Logarythmic depth buffer for planet scale rendering without precision issues
-  Fully GPU-based simulation of forward-looking sonar (FLS)
-  Fully GPU-based simulation of side-scan sonnar (SSS)
-  Normal mapping to enable high resolution surface details
-  Faster download of data from the GPU memory
-  Scheduling of the rendering of multiple views
-  Reallistic measurement of the drawing time
-  Interactive selection outline in 3D view
-  OpenGL function handlers provided through GLAD (dropped outdated GLEW)
-  General cleaning of code and refactoring
-  Dozens of bug fixes

Origins
=======

This project started when I was writing my PhD thesis and needed a realtime simulator for a balancing mono-wheel robot. The simulator not only had to be fast but also deliver high fidelity results. After investigating commercial solutions I have reached the conculsion that I need to implement my own tool becasue simulation times were prohibitively long and no direct interaction with the robot was possible. I decided to use Bullet Physics library and build a simulator capable of computing multi-body dynamics with an analytic tyre-ground collision model, in realitime.
Thanks to this simulator I was able to implement my whole control system in a virtual environment and simulate the robot in an interactive way, which allowed me to finish my PhD thesis.

During my PhD studies I had a brief adventure with underwater robotics and after I finished my PhD I started working in this field. 
Being mostly interested in control design, I have realised that a modern simulator for underwater robots is missing. That is how I started exteding *Stonefish* with marine robotics features and regularily using it in my research. 
I saw that this work can be of benefit for the whole marine robotics community and decided to release it as open-source software.