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
//  Copyright (c) 2013-2026 Patryk Cieslak. All rights reserved.
//

#include "entities/statics/Terrain.h"

#include <algorithm>
#include "stb_image.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Terrain::Terrain(const std::string& uniqueName, const std::string& pathToHeightmap, Scalar scaleX, Scalar scaleY, Scalar height, const std::string& material, const std::string& look, float uvScale) 
    : StaticEntity(uniqueName, material, look)
{
    //Load heightmap data
    int w, h, ch;
    
    if(stbi_is_16_bit(pathToHeightmap.c_str())) //16 bit image
    {
        stbi_us* data = stbi_load_16(pathToHeightmap.c_str(), &w, &h, &ch, 1);
        if(data == nullptr) 
            cCritical("Failed to load heightmap from file '%s'!", pathToHeightmap.c_str());

        heightfield_.resize(w*h);
        for(size_t i=0; i<h; ++i)
            for(size_t j=0; j<w; ++j)
                heightfield_[i*w+j] = (Scalar(1) - data[i*w+j]/(Scalar)(__UINT16_MAX__)) * height;
        
        stbi_image_free(data);
    }
    else //8 bit image
    {
        stbi_uc* data = stbi_load(pathToHeightmap.c_str(), &w, &h, &ch, 1);
        if(data == nullptr) 
            cCritical("Failed to load heightmap from file '%s'!", pathToHeightmap.c_str());
        
        heightfield_.resize(w*h);
        for(size_t i=0; i<h; ++i)
            for(size_t j=0; j<w; ++j)
                heightfield_[i*w+j] = (Scalar(1) - data[i*w+j]/(Scalar)(__UINT8_MAX__)) * height;

        stbi_image_free(data);
    }
    
    //Calculate maximum height
    auto it = std::max_element(heightfield_.begin(), heightfield_.end());
    maxHeight_ = *it;

    //Generate graphical mesh
    phyMesh_ = OpenGLContent::BuildTerrain(heightfield_, w, h, scaleX, scaleY, (GLfloat)maxHeight_, uvScale);
    
    //Generate collision mesh
    btHeightfieldTerrainShape* shape = new btHeightfieldTerrainShape(w, h, heightfield_.data(), Scalar(1), Scalar(0), maxHeight_, 2, PHY_FLOAT, false);
    shape->setLocalScaling(Vector3(scaleX, scaleY, 1.0));
    shape->setUseDiamondSubdivision(true);
    shape->setMargin(0);
    BuildRigidBody(shape);
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
    if(rigidBody_ != NULL)
    {
        btDefaultMotionState* motionState = new btDefaultMotionState(origin*Transform(IQ(), Vector3(0,0,-maxHeight_/Scalar(2))));
        rigidBody_->setMotionState(motionState);
        sm->getDynamicsWorld()->addRigidBody(rigidBody_, MASK_STATIC, MASK_DYNAMIC);
    }
}

}
