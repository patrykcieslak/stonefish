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
         \param modelFilename a path to the 3d model file
         \param scale a scale factor to be used when reading the mesh file
         \param originTrans a transformation of the mesh origin
         \param m a material of which the body is made
         \param lookId a graphical material defining how the mesh is rendered
         \param smoothNormals defines if the model normals should be smoothed after loading
         \param thickness defines the thickness of the model walls, if higher than zero the mesh is considered a shell
         \param isBuoyant defines if buoyancy forces should be calculated for the body
         \param geoProxy defines what time of hydrodynamic approximation of body shape should be used
        */
        Polyhedron(std::string uniqueName, std::string modelFilename, Scalar scale, const Transform& originTrans,
                   Material m, int lookId = -1, bool smoothNormals = true, Scalar thickness = Scalar(-1), bool isBuoyant = true, HydrodynamicProxyType geoProxy = HYDRO_PROXY_ELLIPSOID);
        
        Polyhedron(std::string uniqueName,
                   std::string geometryFilename, Scalar geometryScale, const Transform& geometryOrigin,
                   std::string collisionFilename, Scalar collisionScale, const Transform& collisionOrigin,
                   Material m, int lookId = -1, bool smoothGeometryNormals = false, Scalar thickness = Scalar(-1),
                   bool isBuoyant = true, HydrodynamicProxyType geoProxy = HYDRO_PROXY_ELLIPSOID);
        
        
        //! A method that returns the type of solid.
        SolidType getSolidType();
        
        //! A method that returns the collision shape.
        btCollisionShape* BuildCollisionShape();
        
    private:
        btTriangleIndexVertexArray* triangleArray;
        Vector3 aabb[2];
    };
}

#endif
