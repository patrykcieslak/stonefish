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
//  RayTest.hpp
//  Stonefish
//
//  Created by Patryk Cieslak on 28/06/2022.
//  Copyright (c) 2022 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_RayTest__
#define __Stonefish_RayTest__

#include "BulletCollision/CollisionDispatch/btCollisionWorld.h"

struct DetailedRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
    DetailedRayResultCallback(const btVector3& rayFromWorld, const btVector3& rayToWorld)
        : ClosestRayResultCallback(rayFromWorld, rayToWorld), m_childShapeIndex(-1)
    {
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
    {
        btScalar hitFraction = ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);

        if(rayResult.m_localShapeInfo != nullptr)
            m_childShapeIndex = rayResult.m_localShapeInfo->m_triangleIndex; 
              
        return hitFraction;
    }

    int m_childShapeIndex;
};

#endif