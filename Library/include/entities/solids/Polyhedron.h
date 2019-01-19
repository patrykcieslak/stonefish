//
//  Polyhedron.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright (c) 2012-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Polyhedron__
#define __Stonefish_Polyhedron__

#include "entities/SolidEntity.h"

namespace sf
{
    //! A class representing a rigid body of any shape described by a polyhedron (mesh with triangle faces).
    class Polyhedron : public SolidEntity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the body
         \param graphicsFilename a path to the 3d model used for rendering
         \param graphicsScale a scale factor to be used when reading the mesh file
         \param graphicsOrigin a pose of the mesh with respect to the body origin frame
         \param physicsFilename a path to the 3d model used for physics computation (collisions, fluid dynamics)
         \param physicsScale a scale factor to be used when reading the mesh file
         \param physicsOrigin a pose of the mesh with respect to the body origin frame
         \param m a material of which the body is made
         \param lookId a graphical material defining how the mesh is rendered
         \param smoothGraphicsNormals defines if the graphics mesh normals should be smoothed after loading
         \param thickness defines the thickness of the physics geometry walls, if higher than zero the mesh is considered a shell
         \param enableHydrodynamicForces a flag to enable computation of hydrodynamic forces
         \param isBuoyant defines if buoyancy forces should be calculated for the body
         \param proxy defines what time of hydrodynamic approximation of body shape should be used
         */
        Polyhedron(std::string uniqueName,
                   std::string graphicsFilename, Scalar graphicsScale, const Transform& graphicsOrigin,
                   std::string physicsFilename, Scalar physicsScale, const Transform& physicsOrigin,
                   Material m, int lookId = -1, bool smoothGraphicsNormals = false, Scalar thickness = Scalar(-1),
                   bool enableHydrodynamicForces = true, bool isBuoyant = true, HydrodynamicProxyType proxy = HYDRO_PROXY_ELLIPSOID);
        
        //! A constructor.
        /*!
         \param uniqueName a name for the body
         \param modelFilename a path to the 3d model used for both physics and rendering
         \param scale a scale factor to be used when reading the mesh file
         \param origin a pose of the mesh with respect to the body origin frame
         \param m a material of which the body is made
         \param lookId a graphical material defining how the mesh is rendered
         \param smoothNormals defines if the model normals should be smoothed after loading
         \param thickness defines the thickness of the model walls, if higher than zero the mesh is considered a shell
         \param enableHydrodynamicForces a flag to enable computation of hydrodynamic forces
         \param isBuoyant defines if buoyancy forces should be calculated for the body
         \param proxy defines what time of hydrodynamic approximation of body shape should be used
         */
        Polyhedron(std::string uniqueName, std::string modelFilename, Scalar scale, const Transform& origin,
                   Material m, int lookId = -1, bool smoothNormals = true, Scalar thickness = Scalar(-1),
                   bool enableHydrodynamicForces = true, bool isBuoyant = true, HydrodynamicProxyType proxy = HYDRO_PROXY_ELLIPSOID);
        
        //! A destructor.
        ~Polyhedron();
        
        //! A method that returns the type of solid.
        SolidType getSolidType();
        
        //! A method that returns the collision shape.
        btCollisionShape* BuildCollisionShape();
        
        //! A method used to build the graphical representation of the body.
        void BuildGraphicalObject();
        
    private:
        Mesh *graMesh; //Mesh used for rendering
        //Vector3 aabb[2];
    };
}

#endif
