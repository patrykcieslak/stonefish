/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  GeometryFileUtil.h
//  Stonefish
//
//  Created by Patryk Cieslak on 22/11/2018.
//  Copyright (c) 2018-2026 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GeometryFileUtil__
#define __Stonefish_GeometryFileUtil__

#include "StonefishCommon.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    inline void hashCombine(std::size_t& seed, std::size_t value) 
    {
        // 0x9e3779b9 is the golden ratio; it prevents sequential numbers from colliding
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    struct MeshProperties
    {
        Scalar mass;
        Vector3 CG;
        Scalar volume;
        Scalar surface;
        Vector3 Ipri;
        Matrix3 Irot;
    };

    struct IndexedVertex
    {
        GLuint index;
        Vertex vertex;

        bool operator==(const IndexedVertex& other) const 
        {
            return vertex == other.vertex;
        }

        bool operator<(const IndexedVertex& other) const 
        {
            return index < other.index;
        }
    };

    struct IndexedVertexHash
    {
        std::size_t operator()(const IndexedVertex& s) const
        {
            std::size_t h1 = std::hash<GLfloat>{}(s.vertex.pos.x);
            std::size_t h2 = std::hash<GLfloat>{}(s.vertex.pos.y);
            std::size_t h3 = std::hash<GLfloat>{}(s.vertex.pos.z);
            std::size_t h4 = std::hash<GLfloat>{}(s.vertex.normal.x);
            std::size_t h5 = std::hash<GLfloat>{}(s.vertex.normal.y);
            std::size_t h6 = std::hash<GLfloat>{}(s.vertex.normal.z);
            std::size_t seed = 0;
            hashCombine(seed, h1);
            hashCombine(seed, h2);
            hashCombine(seed, h3);
            hashCombine(seed, h4);
            hashCombine(seed, h5);
            hashCombine(seed, h6);
            return seed;
        }
    };

    struct IndexedTexturableVertex
    {
        GLuint index;
        TexturableVertex vertex;

        bool operator==(const IndexedTexturableVertex& other) const 
        {
            return vertex == other.vertex;
        }

        bool operator<(const IndexedTexturableVertex& other) const
        {
            return index < other.index;
        }
    };

    struct IndexedTexturableVertexHash
    {
        std::size_t operator()(const IndexedTexturableVertex& s) const
        {
            std::size_t h1 = std::hash<GLfloat>{}(s.vertex.pos.x);
            std::size_t h2 = std::hash<GLfloat>{}(s.vertex.pos.y);
            std::size_t h3 = std::hash<GLfloat>{}(s.vertex.pos.z);
            std::size_t h4 = std::hash<GLfloat>{}(s.vertex.normal.x);
            std::size_t h5 = std::hash<GLfloat>{}(s.vertex.normal.y);
            std::size_t h6 = std::hash<GLfloat>{}(s.vertex.normal.z);
            std::size_t h7 = std::hash<GLfloat>{}(s.vertex.uv.x);
            std::size_t h8 = std::hash<GLfloat>{}(s.vertex.uv.y);
            std::size_t seed = 0;
            hashCombine(seed, h1);
            hashCombine(seed, h2);
            hashCombine(seed, h3);
            hashCombine(seed, h4);
            hashCombine(seed, h5);
            hashCombine(seed, h6);
            hashCombine(seed, h7);
            hashCombine(seed, h8);
            return seed;
        }
    };

    //! A function to load geometry from a file.
    /*!
     \param path a path to the file
     \param scale a scale to apply to the data
     \return a pointer to an allocated mesh structure
     */
    Mesh* LoadGeometryFromFile(const std::string& path, GLfloat scale);
    
    //! A function to load geometry from a STL file.
    /*!
     \param path a path to the file
     \param scale a scale to apply to the data
     \return a pointer to an allocated mesh structure
     */
    Mesh* LoadSTL(const std::string& path, GLfloat scale);
    
    //! A function to load geometry from an OBJ file.
    /*!
     \param path a path to the file
     \param scale a scale to apply to the data
     \return a pointer to an allocated mesh structure
     */
    Mesh* LoadOBJ(const std::string& path, GLfloat scale);

    //! A function to compute all physical properties of a mesh.
    /*!
     \param mesh a pointer to the mesh structure
     \param thickness a value of the wall thickness [m]
     \param desity the density of the material the mesh is made of [kg/m3]
     \param mass output of the computed mass [kg]
     \param CG output of the computed location of the centre of gravity [m]
     \param volume output of the computed volume [m3]
     \param surface output of the computed surface area [m2]
     \param Ipri output of the computed moments of inertia [kgm2]
     \param Irot output of the rotation matrix between mesh origin and the computed principal inertial axes
     */
    void ComputePhysicalProperties(const Mesh* mesh, Scalar thickness, Scalar density, Scalar& mass, Vector3& CG, Scalar& volume, Scalar& surface, Vector3& Ipri, Matrix3& Irot);
    
    //! A function to compute all physical properties of a mesh.
    /*!
     \param mesh a pointer to the mesh structure
     \param thickness a value of the wall thickness [m]
     \param desity the density of the material the mesh is made of [kg/m3]
     \return a structure containing properties of the mesh
     */
    MeshProperties ComputePhysicalProperties(const Mesh* mesh, Scalar thickness, Scalar density);
    
    //! A function to compute inertial axis for a given moment of inertia.
    /*!
     \param I the inertia tensor
     \param value the moment of inertia for which to find inertial axis
     \return versor of the inertial axis
     */
    Vector3 FindInertialAxis(const Matrix3& I, Scalar value);

    //! A fuction that checks if a matrix is diagonal.
    /*!
     \param A matrix
     \return is diagonal
    */
    bool IsDiagonal(const Matrix3& A);
}

#endif
