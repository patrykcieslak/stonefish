.. Stonefish documentation master file, created by
   sphinx-quickstart on Mon Jul 27 16:15:09 2020.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

====================
Welcome to Stonefish
====================

An advanced simulation tool developed for marine robotics
=========================================================

Stonefish is a *C++ library* which wraps around `Bullet Physics library <https://pybullet.org>`_ to deliver intuitive simulation of marine robots in realistic scenarios. It is directed towards researchers in the field of marine robotics but can as well be used as a general robot simulator.

Stonefish includes advanced hydrodynamics computation based on actual geometry of bodies to better approximate hydrodynamic forces and allow for effects not possible when using symbolic models. The rendering pipeline, developed from the ground up, delivers realistic rendering of atmosphere, ocean and underwater environment. Special focus was put on the latter, where effects of wavelength-dependent light absorption and scattering were considered.

Stonefish can be used to create standalone simulators or combined with a ROS package `stonefish_ros <https://github.com/patrykcieslak/stonefish_ros>`_, which implements a standard simulator node, loading the simulation world from an XML description file. This simulator node takes care of ROS interaction through messages and services. The XML parser function can as well be used in the custom standalone simulator.

Cite Me
=======

This software was written and is continuously developed by Patryk Cieślak. Small parts of the software based on code developed by other authors are marked clearly as such.

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

Funding
=======

This work was part of a project titled ”Force/position control system to enable compliant manipulation from a floating I-AUV”, which received funding from the European Community H2020 Programme, under the Marie Skłodowska-Curie grant agreement no. 750063. The work was continued under a project titled ”EU Marine Robots”, which received funding from the European Community H2020 Programme, grant agreement no. 731103.

.. toctree::
   :hidden:

   overview
   theory
   install
   building
   scenario
   materials
   environment
   bodies
   sensors
   actuators
   comms
   robots
   special
   changelog
   license