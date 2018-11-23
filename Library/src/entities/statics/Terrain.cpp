//
//  Terrain.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/9/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "entities/statics/Terrain.h"

using namespace sf;

Terrain::Terrain(std::string uniqueName, unsigned int width, unsigned int length, btScalar size, btScalar minHeight, btScalar maxHeight, btScalar roughness, Material m, int lookId) : StaticEntity(uniqueName, m, lookId)
{
    size = UnitSystem::SetLength(size);
    minHeight = UnitSystem::SetLength(minHeight);
    maxHeight = UnitSystem::SetLength(maxHeight);
    
    terrainHeight = new btScalar[width*length];
    btVector3* terrainNormals = new btVector3[width*length];
    btScalar correl = 1000;
    btScalar amplitude = maxHeight;
    
    //heightfield generation
    for(unsigned int i=0; i<length; i++)
        for(unsigned int h=0; h<width; h++)
        {
            btScalar distance = sqrt((i*size)*(i*size)+(h*size)*(h*size))*10;
            btScalar height = amplitude*sin(i*size*0.5)*cos(h*size*0.5)*exp(-(distance/correl)*(distance/correl));   //amplitude*amplitude*exp(-fabs(distance/correl)*fabs(distance/correl))+minHeight;
            terrainHeight[i*width+h] = height > maxHeight ? maxHeight : height;
        }
    
    //normals calculation
    for(unsigned int i=0; i<length; i++)
        for(unsigned int h=0; h<width; h++)
            terrainNormals[i*width+h] = btVector3(0,1,0);
    
    for(unsigned int i=1; i<length-1; i++)
        for(unsigned int h=1; h<width-1; h++)
        {
            btVector3 c = btVector3(h*size, terrainHeight[i*width+h], i*size);
            btVector3 a1 = btVector3((h-1)*size, terrainHeight[i*width+(h-1)], i*size) - c;
            btVector3 a2 = btVector3((h+1)*size, terrainHeight[i*width+(h+1)], i*size) - c;
            btVector3 a3 = btVector3(h*size, terrainHeight[(i-1)*width+h], (i-1)*size) - c;
            btVector3 a4 = btVector3(h*size, terrainHeight[(i+1)*width+h], (i+1)*size) - c;
            terrainNormals[i*width+h] = (a1.cross(a3).normalized()+a3.cross(a2).normalized()+a2.cross(a4).normalized()+a4.cross(a1).normalized()).normalized()*-1;
        }
    
    //rigid body creation
    int upAxis = 1;
    btHeightfieldTerrainShape* shape = new btHeightfieldTerrainShape(width, length, terrainHeight, 1.0, minHeight, maxHeight, upAxis, PHY_FLOAT, false);
    btVector3 localScaling = btVector3(size, 1.0, size);
	shape->setLocalScaling(localScaling);
    shape->setUseDiamondSubdivision(true);
    
    BuildRigidBody(shape);
    
    delete [] terrainNormals;
}

StaticEntityType Terrain::getStaticType()
{
    return STATIC_TERRAIN;
}

/*
////////////////
static btScalar randomHeight(int step, int gridSize, btScalar gridSpacing)
{
	return (0.33 * gridSpacing * gridSize * step * (rand() - (0.5 * RAND_MAX))) / (1.0 * RAND_MAX * gridSize);
}

// creates a random, fractal heightfield
static void setFractal(btScalar *grid, int step, int gridSize, btScalar gridSpacing)
{
	int newStep = step / 2;
    
	// special case: starting (must set four corners)
	if (gridSize - 1 == step) {
		// pick a non-zero (possibly negative) base elevation for testing
		btScalar base = randomHeight(step/2, gridSpacing, gridSize);
        
		convertFromFloat(grid, base, type);
		convertFromFloat(grid + step * bytesPerElement, base, type);
		convertFromFloat(grid + step * s_gridSize * bytesPerElement, base, type);
		convertFromFloat(grid + (step * s_gridSize + step) * bytesPerElement, base, type);
	}
    
	// determine elevation of each corner
	float c00 = convertToFloat(grid, type);
	float c01 = convertToFloat(grid + step * bytesPerElement, type);
	float c10 = convertToFloat(grid + (step * s_gridSize) * bytesPerElement, type);
	float c11 = convertToFloat(grid + (step * s_gridSize + step) * bytesPerElement, type);
    
	// set top middle
	updateHeight(grid + newStep * bytesPerElement, 0.5 * (c00 + c01) + randomHeight(step), type);
    
	// set left middle
	updateHeight(grid + (newStep * s_gridSize) * bytesPerElement, 0.5 * (c00 + c10) + randomHeight(step), type);
    
	// set right middle
	updateHeight(grid + (newStep * s_gridSize + step) * bytesPerElement, 0.5 * (c01 + c11) + randomHeight(step), type);
    
	// set bottom middle
	updateHeight(grid + (step * s_gridSize + newStep) * bytesPerElement, 0.5 * (c10 + c11) + randomHeight(step), type);
    
	// set middle
	updateHeight(grid + (newStep * s_gridSize + newStep) * bytesPerElement, 0.25 * (c00 + c01 + c10 + c11) + randomHeight(step), type);
    
    //	std::cerr << "Computing grid with step = " << step << ": after\n";
    //	dumpGrid(grid, bytesPerElement, type, step + 1);
    
	// terminate?
	if (newStep < 2) {
		return;
	}
    
	// recurse
	setFractal(grid, bytesPerElement, type, newStep);
	setFractal(grid + newStep * bytesPerElement, bytesPerElement, type, newStep);
	setFractal(grid + (newStep * s_gridSize) * bytesPerElement, bytesPerElement, type, newStep);
	setFractal(grid + ((newStep * s_gridSize) + newStep) * bytesPerElement, bytesPerElement, type, newStep);
}
*/

