//
//  Terrain.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 09/01/2013.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#include "entities/statics/Terrain.h"

#include "core/Console.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLContent.h"
#include "utils/stb_image.h"

namespace sf
{

Terrain::Terrain(std::string uniqueName, std::string pathToHeightmap, Scalar scaleX, Scalar scaleY, Scalar height, Material m, int lookId) : StaticEntity(uniqueName, m, lookId)
{
    int w,h,ch;
    unsigned char* dataBuffer = stbi_load(pathToHeightmap.c_str(), &w, &h, &ch, 1);
    
    if(dataBuffer == NULL)
        cCritical("Failed to load heightmap from file '%s'!", pathToHeightmap.c_str());
    
    maxHeight = Scalar(0);
    terrainHeight = new Scalar[w*h];
    GLfloat* heightfield = new GLfloat[w*h];
    
    for(int i=0; i<h; ++i)
        for(int j=0; j<w; ++j)
        {
            Scalar localHeight = (Scalar(1)-(Scalar)dataBuffer[i*w+j]/Scalar(255)) * height;
            terrainHeight[i*w+j] = localHeight;
            heightfield[i*w+j] = (GLfloat)localHeight;
            maxHeight = localHeight > maxHeight ? localHeight : maxHeight;
        }

    phyMesh = OpenGLContent::BuildTerrain(heightfield, w, h, scaleX, scaleY, maxHeight);
    
    delete [] dataBuffer;
    delete [] heightfield;
    
    btHeightfieldTerrainShape* shape = new btHeightfieldTerrainShape(w, h, terrainHeight, Scalar(1), Scalar(0), maxHeight, 2, PHY_FLOAT, false);
    Vector3 localScaling = Vector3(scaleX, scaleY, 1.0);
    shape->setLocalScaling(localScaling);
    shape->setUseDiamondSubdivision(true);
    
    BuildRigidBody(shape);
}

Terrain::~Terrain()
{
    delete [] terrainHeight;
}

StaticEntityType Terrain::getStaticType()
{
    return STATIC_TERRAIN;
}
    
void Terrain::getAABB(Vector3 &min, Vector3 &max)
{
        //Terrain shouldn't affect shadow calculation
        min.setValue(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
        max.setValue(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
}

void Terrain::AddToSimulation(SimulationManager* sm, const Transform& origin)
{
    if(rigidBody != NULL)
    {
        btDefaultMotionState* motionState = new btDefaultMotionState(origin*Transform(IQ(), Vector3(0,0,-maxHeight/Scalar(2))));
        rigidBody->setMotionState(motionState);
        sm->getDynamicsWorld()->addRigidBody(rigidBody, MASK_STATIC, MASK_STATIC | MASK_DEFAULT);
    }
}

}
