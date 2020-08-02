======
Theory
======

Unit system
===========

The `International System of Units (SI) <https://en.wikipedia.org/wiki/International_System_of_Units>`_ is used for the definitions and computations throughout the simulation library. For convenience some of the angular quantities are specified in degrees; the computations are always done using radians.

Coordinate frames
=================

Multiple coordinate frames are defined in marine craft theory.
At this stage, the *Stonefish* library is intended for small scale simulations where Earth curvature can be neglected. Therefore, the **North-East-Down (NED) coordinate frame** is used throughout the simulation, as the world reference frame. The NED frame is a Cartesian coordinate frame, tangent to the Earth surface at a chosen geographic location. This location is called the home and defined by latitude and longitude.

Rigid body dynamics and collision
=================================

The *Stonefish* library utilises algoritms implemented in the *Bullet Physics* library for the computation of the rigid body kinematics, dynamics and collision. The simulation engine is implementing velocity-based dynamics with semi-implicit Euler integration. The dynamics are computed using the Sequential Impulse algorithm for separate dynamical bodies. Kinematic trees are handled through an implementation of the Featherstone multi-body algorithm. Rigid collisions are computed using an impulse-based approach. Soft collisions are possible by defining collision stiffeness and damping factors. Frictional forces are computed based on static and dynamic friction coefficients defined between materials. The Stribeck function is used for the transition between sticking and sliding phases.

Hydrodynamics
=============

The *Stonefish* library delivers a novel approach to simulating hydrodynamics and aerodynamics, by performing geometry based computations. The main focus is put on hydrodynamics as the library is directed towards marine robotics. The simulated effects include: added mass, buoyancy and drag. The drag is composed of 3 elements: potential drag (linear), form drag (quadratic) and skin drag. 

Added mass
----------

The added mass effect is encountered during the acceleration of bodies submerged in liquid. It is manifested by the dynamical response of the body as if it was heavier than it actually is, because the surrounding liquid has to be moved together with it. The added mass is formulated as a 6x6 matrix, which describes how acceleration in every direction is affected by the fluid. It is a common practice to use a diagonal representation of the matrix, which is also the case here. Moreover, due to the fact that the underlying physics library is not capable of accepting different values of mass for each axis, a mean value is used for all of the axes. In terms of the added inertia tensor, a vector of 3 inertia moments is used. The values of the added mass matrix are computed based on an automatic approximation of the body geometry using one of the 3 solids: sphere, cylinder or ellipsoid.

Buoyancy
--------

The buoyancy force is calculated based on the sum of hydrostatic forces acting on the body surface. The actual geometry is used to compute force at each face of the mesh, depending on the depth of the face centre. It allows for simulating realistic buoyancy force at the surface of the ocean, with and without geometrical waves. When the body is completely submerged the buoyancy force is based on the volume of the mesh, computed automatically during loading.

Drag
----

The drag forces are calculated as a sum of forces acting on each face of the body surface. To obtain precise values of these forces it is required to solve Navier-Stokes equations, which is not possible for a general 3D case in realtime. Therefore, the computations implemented in the *Stonefish* library have to be based on the local velocity of fluid as if there was no body. The result is not quantitively correct but it gives a good approximation and allows for effects not possible when using simple formulas, e.g., a water current acting on a part of the body.