//
//  GeometryUtil.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GeometryUtil__
#define __Stonefish_GeometryUtil__

#include "common.h"

inline btScalar distanceFromCenteredPlane(const btVector3& planeN, const btVector3& v)
{
    btScalar distance = planeN.dot(v);
    return distance == 0.0 ? 10e-9 : distance;
}

inline void SetFloatvFromMat(const btMatrix3x3 &mat, float* fv)
{
	fv[0] = btScalar(mat.getRow(0).x());
    fv[1] = btScalar(mat.getRow(1).x());
    fv[2] = btScalar(mat.getRow(2).x());
    fv[3] = btScalar(mat.getRow(0).y());
    fv[4] = btScalar(mat.getRow(1).y());
    fv[5] = btScalar(mat.getRow(2).y());
    fv[6] = btScalar(mat.getRow(0).z());
    fv[7] = btScalar(mat.getRow(1).z());
    fv[8] = btScalar(mat.getRow(2).z());
}

#endif