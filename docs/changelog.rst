==========
Changelog
==========

1.0
===

- Fully GPU-based simulation of mechanical scanning imaging sonar (MSIS)
- Improvements in all sonar simulations
- Significant improvement to DVL performance when heightfield terrain is used
- Heightfield terrain now supports 16 bit heightmaps
- New syntax for loading ocean and atmosphere definitons using the XML parser
- Support for arguments passed to the included files
- New, complete, beautiful documentation generated with Sphinx

0.9
===

- Moved to the OpenGL 4.3 functionality (compute shaders)
- Complete rewrite of the ocean/underwater rendering pipeline
- Light absorption and scattering in water based on Jerlov measurements
- Full support of photo-reallistic sky and sunlight as well as point and spot lights
- New, linear tree based, automatic LOD algorithm
- New automatic exposure (histogram based) and anti-aliasing (FXAA) algorithms
- Logarythmic depth buffer for planet scale rendering without precision issues
- Fully GPU-based simulation of forward-looking sonar (FLS)
- Fully GPU-based simulation of side-scan sonnar (SSS)
- Normal mapping to enable high resolution surface details
- Faster download of data from the GPU memory
- Scheduling of the rendering of multiple views
- Reallistic measurement of the drawing time
- Interactive selection outline in 3D view
- OpenGL function handlers provided through GLAD (dropped outdated GLEW)
- General cleaning of code and refactoring
- Dozens of bug fixes

Origins
=======

This project started when I was writing my PhD thesis and needed a realtime simulator for a balancing mono-wheel robot. The simulator not only had to be fast but also deliver high fidelity results. After investigating commercial solutions I have reached the conculsion that I need to implement my own tool becasue simulation times were prohibitively long and no direct interaction with the robot was possible. I decided to use Bullet Physics library and build a simulator capable of computing multi-body dynamics with an analytic tyre-ground collision model, in realitime.
Thanks to this simulator I was able to implement my whole control system in a virtual environment and simulate the robot in an interactive way, which allowed me to finish my PhD thesis.

During my PhD studies I had a brief adventure with underwater robotics and after I finished my PhD I started working in this field. 
Being mostly interested in control design, I have realised that a modern simulator for underwater robots is missing. That is how I started exteding *Stonefish* with marine robotics features and regularily using it in my research. 
I saw that this work can be of benefit for the whole marine robotics community and decided to release it as open-source software.