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
#include <glm/glm.hpp>

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

inline glm::mat4 glMatrixFromBtTransform(const btTransform& T)
{
#ifdef BT_USE_DOUBLE_PRECISION
    btScalar glmatrix[16];
    T.getOpenGLMatrix(glmatrix);
    glm::mat4 M((GLfloat)glmatrix[0],(GLfloat)glmatrix[1],(GLfloat)glmatrix[2],(GLfloat)glmatrix[3],
				(GLfloat)glmatrix[4],(GLfloat)glmatrix[5],(GLfloat)glmatrix[6],(GLfloat)glmatrix[7],
				(GLfloat)glmatrix[8],(GLfloat)glmatrix[9],(GLfloat)glmatrix[10],(GLfloat)glmatrix[11],
				(GLfloat)glmatrix[12],(GLfloat)glmatrix[13],(GLfloat)glmatrix[14],(GLfloat)glmatrix[15]);
#else
    GLfloat glmatrix[16];
    T.getOpenGLMatrix(glmatrix);
    glm::mat4 M = glm::make_mat4(glmatrix);
	//OR
	//glm::mat4 M;
	//T.getOpenGLMatrix(&M[0].x);
#endif
	return M;
}

#endif