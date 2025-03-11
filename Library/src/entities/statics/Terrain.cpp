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
//  Terrain.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 09/01/2013.
//  Copyright (c) 2013-2020 Patryk Cieslak. All rights reserved.
//

#include "entities/statics/Terrain.h"

#include "stb_image.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Terrain::Terrain(std::string uniqueName, std::string pathToHeightmap, Scalar scaleX, Scalar scaleY, Scalar height, std::string material, std::string look, float uvScale) 
    : StaticEntity(uniqueName, material, look)
{
    //Load heightmap data
    int w, h, ch;
    GLfloat* heightmap;

    if(stbi_is_16_bit(pathToHeightmap.c_str())) //16 bit image
    {
        stbi_us* data = stbi_load_16(pathToHeightmap.c_str(), &w, &h, &ch, 1);
        if(data == NULL) cCritical("Failed to load heightmap from file '%s'!", pathToHeightmap.c_str());
        heightmap = new GLfloat[w*h];
        for(int i=0; i<h; ++i)
            for(int j=0; j<w; ++j)
                heightmap[i*w+j] = (1.f - data[i*w+j]/(GLfloat)(__UINT16_MAX__)) * height;
        stbi_image_free(data);
    }
    else //8 bit image
    {
        stbi_uc* data = stbi_load(pathToHeightmap.c_str(), &w, &h, &ch, 1);
        if(data == NULL) cCritical("Failed to load heightmap from file '%s'!", pathToHeightmap.c_str());
        heightmap = new GLfloat[w*h];
        for(int i=0; i<h; ++i)
            for(int j=0; j<w; ++j)
                heightmap[i*w+j] = (1.f - data[i*w+j]/(GLfloat)(__UINT8_MAX__)) * height;
        stbi_image_free(data);
    }
    
    //Calculate max height
    maxHeight = Scalar(0);
    heightfield = new Scalar[w*h];
    for(int i=0; i<h; ++i)
        for(int j=0; j<w; ++j)
        {
            heightfield[i*w+j] = Scalar(heightmap[i*w+j]);
            maxHeight = heightfield[i*w+j] > maxHeight ? heightfield[i*w+j] : maxHeight;
        }

    //Generate graphical mesh
    phyMesh = OpenGLContent::BuildTerrain(heightmap, w, h, scaleX, scaleY, (GLfloat)maxHeight, uvScale);
    delete [] heightmap;

    //Generate collision mesh
    btHeightfieldTerrainShape* shape = new btHeightfieldTerrainShape(w, h, heightfield, Scalar(1), Scalar(0), maxHeight, 2, PHY_FLOAT, false);
    shape->setLocalScaling(Vector3(scaleX, scaleY, 1.0));
    shape->setUseDiamondSubdivision(true);
    shape->setMargin(0);
    BuildRigidBody(shape);
}

Terrain::~Terrain()
{
    delete [] heightfield;
}

StaticEntityType Terrain::getStaticType()
{
    return StaticEntityType::TERRAIN;
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
        sm->getDynamicsWorld()->addRigidBody(rigidBody, MASK_STATIC, MASK_DYNAMIC);
    }
}

}
