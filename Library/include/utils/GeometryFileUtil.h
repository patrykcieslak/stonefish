//
//  GeometryFileUtil.h
//  Stonefish
//
//  Created by Patryk Cieślak on 22/11/2018.
//  Copyright (c) 2018-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GeometryFileUtil__
#define __Stonefish_GeometryFileUtil__

#include "graphics/OpenGLDataStructs.h"

namespace sf
{
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
}

#endif
