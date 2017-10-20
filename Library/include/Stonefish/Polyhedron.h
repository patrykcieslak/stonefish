//
//  Polyhedron.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Polyhedron__
#define __Stonefish_Polyhedron__

#include "SolidEntity.h"

class Polyhedron : public SolidEntity
{
public:
    Polyhedron(std::string uniqueName, std::string modelFilename, btScalar scale, Material m, int lookId = -1, bool smoothNormals = true);
	~Polyhedron();
    
    SolidType getSolidType();
    btCollisionShape* BuildCollisionShape();
	
private:
    btTriangleIndexVertexArray* triangleArray;
    btVector3 aabb[2];
};


#endif
