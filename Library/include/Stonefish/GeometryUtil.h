//
//  GeometryUtil.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GeometryUtil__
#define __Stonefish_GeometryUtil__

#include "OpenGLPipeline.h"

#define GOLDEN_RATIO ((btScalar)1.61803398875)

struct TriangleFace
{
    uint32_t vertexIndex[3];
    uint32_t normalIndex;
    int32_t neighbourIndex[3];
    
    uint32_t glnormalIndex[3];
    uint32_t uvIndex[3];
    
    friend bool operator==(const TriangleFace& lhs, const TriangleFace& rhs)
    {
        if(lhs.normalIndex != rhs.normalIndex)
            return false;
        
        if(lhs.vertexIndex[0] != rhs.vertexIndex[0] || lhs.vertexIndex[1] != rhs.vertexIndex[1] || lhs.vertexIndex[2] != rhs.vertexIndex[2])
            return false;
        
        return true;
    };
};

struct OpenGLNormal
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct OpenGLUV
{
    GLfloat u;
    GLfloat v;
};

struct TriangleMesh
{
    //geometry
    btAlignedObjectArray<btVector3> vertices;
    btAlignedObjectArray<btVector3> normals;
    btAlignedObjectArray<TriangleFace> faces;
    
    //graphics
    btAlignedObjectArray<OpenGLNormal> glnormals;
    btAlignedObjectArray<OpenGLUV> uvs;
};

void CalculateNeighbours(TriangleMesh* m, bool smooth);
void SubdivideFace(TriangleMesh* mesh, uint32_t faceId, unsigned int n);
void SubdivideFace(TriangleMesh* mesh, uint32_t faceId, unsigned int n, uint32_t& newFacesStart, uint32_t& newFacesCount);
void AABB(TriangleMesh* mesh, btVector3& min, btVector3& max);
void AABS(TriangleMesh* mesh, btScalar& bsRadius, btVector3& bsCenterOffset);
btScalar signedVolumeOfTetrahedron6(const btVector3& v1, const btVector3& v2, const btVector3& v3);
btScalar distanceFromCenteredPlane(const btVector3& planeN, const btVector3& v);
void SetFloatvFromMat(const btMatrix3x3 &mat, GLfloat* fv);

#endif