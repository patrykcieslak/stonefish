.. Stonefish documentation master file, created by
   sphinx-quickstart on Mon Jul 27 16:15:09 2020.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

====================
Welcome to Stonefish
====================

An advanced simulation tool developed for marine robotics
=========================================================

Stonefish is a C++ library combining a physics engine and a lightweight rendering pipeline. The physics engine is based on the core functionality of the `Bullet Physics <https://pybullet.org>`_ library, extended to deliver realistic simulation of marine robots. It is directed towards researchers in the field of marine robotics but can as well be used as a general purpose robot simulator.

Stonefish includes advanced hydrodynamics computation based on actual geometry of bodies to better approximate hydrodynamic forces and allow for effects not possible when using symbolic models. The rendering pipeline, developed from the ground up, delivers realistic rendering of atmosphere, ocean and underwater environment. Special focus was put on the latter, where effects of wavelength-dependent light absorption and scattering were considered.

Stonefish can be used to create standalone simulators or combined with a ROS package `stonefish_ros <https://github.com/patrykcieslak/stonefish_ros>`_, which implements a standard simulator node, loading the simulation world from an XML description file. This simulator node takes care of ROS interaction through messages and services. The XML parser function can as well be used in the custom standalone simulator.

Cite Me
=======

This software was written and is continuously developed by Patryk Cieślak. Parts of the software based on code developed by other authors are clearly marked as such.

If you find this software useful in your research, please cite:

*Patryk Cieślak, "Stonefish: An Advanced Open-Source Simulation Tool Designed for Marine Robotics, With a ROS Interface", In Proceedings of MTS/IEEE OCEANS 2019, June 2019, Marseille, France*

.. code-block:: bib

   @inproceedings{stonefish,
      author = "Cie{\'s}lak, Patryk",
      booktitle = "OCEANS 2019 - Marseille",
      title = "Stonefish: An Advanced Open-Source Simulation Tool Designed for Marine Robotics, With a ROS Interface",
      month = "jun",
      year = "2019",
      doi = "10.1109/OCEANSE.2019.8867434"
   }


Support
=======

I offer paid support on setting up the simulation of your own systems, including necessary 3D modelling (simplification of CAD models for physics, preparation of accurate visualisations, etc.), setup of simulation scenarios, development of new sensors, actuators, and custom features that do not require significant changes to the code base. Please contact me at patryk.cieslak@udg.edu.

Funding
=======

Currently there is no funding of this work. It is developed by the author following his needs and requests from other users. The work was started during his PhD studies and was mainly developed in his free time. Parts of this work were developed in the context of the project titled ”Force/position control system to enable compliant manipulation from a floating I-AUV”, which received funding from the European Community H2020 Programme, under the Marie Sklodowska-Curie grant agreement no. 750063. The work was also extended under a project titled ”EU Marine Robots”, which received funding from the European Community H2020 Programme, grant agreement no. 731103. 

.. toctree::
   :hidden:

   overview
   theory
   install
   building
   scenario
   materials
   environment
   animated
   bodies
   robots
   sensors
   actuators
   comms
   special
   changelog
   license
