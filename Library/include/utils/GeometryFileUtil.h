//
//  GeometryFileUtil.h
//  Stonefish
//
//  Created by Patryk Cieślak on 22/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GeometryFileUtil__
#define __Stonefish_GeometryFileUtil__

#include "graphics/OpenGLDataStructs.h"

namespace sf
{

Mesh* LoadGeometryFromFile(const std::string& path, GLfloat scale);
Mesh* LoadSTL(const std::string& path, GLfloat scale);
Mesh* LoadOBJ(const std::string& path, GLfloat scale);

}

#endif
