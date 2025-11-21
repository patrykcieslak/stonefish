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

*Michele Grimaldi, Patryk Cieślak, Eduardo Ochoa, Vibhav Bharti, Hayat Rajani, Ignacio Carlucho, Maria Koskinopoulou, Yvan R. Petillot, and Nuno Gracias, "Stonefish: Supporting Machine Learning Research in Marine Robotics", In Proceedings of IEEE ICRA 2025, May 2025, Atlanta, USA*

.. code-block:: bib

   @inproceedings{stonefish_ml,
      author = "Michele Grimaldi and Patryk Cieslak and Eduardo Ochoa and Vibhav Bharti and Hayat Rajani and Ignacio Carlucho and Maria Koskinopoulou and Yvan R. Petillot and Nuno Gracias",
      title = "Stonefish: Supporting Machine Learning Research in Marine Robotics",
      booktitle = "Proceedings of the IEEE International Conference on Robotics and Automation",
      month = "may",
      year = "2025",
      eprint = "2502.11887",
      archivePrefix = "arXiv",
      url = "https://arxiv.org/abs/2502.11887",
      organization = "IEEE"
   }

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
   cables
   robots
   sensors
   actuators
   comms
   special
   changelog
   license
