//
//  GeometryFileUtil.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/23/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GeometryFileUtil__
#define __Stonefish_GeometryFileUtil__

#include "GeometryUtil.h"

TriangleMesh* LoadModel(const char* filename, btScalar scale, bool smooth);
TriangleMesh* LoadSTL(const char* filename, btScalar scale, bool smooth);
TriangleMesh* LoadOBJ(const char* filename, btScalar scale, bool smooth);

#endif