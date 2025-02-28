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
//  Copyright (c) 2018-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GeometryFileUtil__
#define __Stonefish_GeometryFileUtil__

#include "StonefishCommon.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    struct MeshProperties
    {
        Scalar mass;
        Vector3 CG;
        Scalar volume;
        Scalar surface;
        Vector3 Ipri;
        Matrix3 Irot;
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
