//
//  GeometryUtil.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "GeometryUtil.h"

void CalculateNeighbours(TriangleMesh* m, bool smooth)
{
    //For every face
    for(uint32_t i=0; i<m->faces.size(); i++)
    {
        TriangleFace thisFace = m->faces[i];
        unsigned int neighbours = 0;
        
        //For all other faces with higher indexes
        for(uint32_t h=0; h<m->faces.size(); h++)
        {
            if(h != i)
            {
                TriangleFace thatFace = m->faces[h];
                unsigned int common = 0;
                
                //For every vertex of 'thisFace'
                for(int j = 0; j < 3; j++)
                {
                    for(int k = 0; k < 3; k++) //Check every vertex of 'thatFace'
                        if(thisFace.vertexIndex[j] == thatFace.vertexIndex[k])
                        {
                            common++;
                            break;
                        }
                }
                
                if(common == 2) //faces are neighbours if two vertices are common
                {
                    thisFace.neighbourIndex[neighbours] = h;
                    neighbours++;
                }
                
                if(neighbours == 3) //triangle can only have 3 neighbours
                    break;
            }
        }
    }
    
    //generate new smoothed normals for rendering
    if(smooth)
    {
        //delete old glnormals
        m->glnormals.clear();
        
        //for all faces
        for(uint32_t i=0; i<m->faces.size(); i++)
        {
            TriangleFace thisFace = m->faces[i];
            btVector3 thisN = m->normals[thisFace.normalIndex];
            
            //For every vertex
            for(uint8_t h=0; h<3; h++)
            {
                btVector3 n = thisN;
                uint contrib = 1;
                
                //Loop through all faces
                for(uint32_t j=0; j<m->faces.size(); j++)
                {
                    if(j != i) //Reject same face
                    {
                        TriangleFace thatFace = m->faces[j];
                        btVector3 thatN = m->normals[thatFace.normalIndex];
                        
                        for(uint8_t k=0; k<3; k++)
                            if((thatFace.vertexIndex[k] == thisFace.vertexIndex[h])&&(thatN.dot(thisN) > 0.707))
                            {
                                n += thatN;
                                contrib++;
                                break;
                            }
                    }
                }
                
                n /= (btScalar)contrib;
                
                OpenGLNormal gln;
                gln.x = (GLfloat)n.x();
                gln.y = (GLfloat)n.y();
                gln.z = (GLfloat)n.z();
                m->glnormals.push_back(gln);
                m->faces[i].glnormalIndex[h] = m->glnormals.size()-1;
            }
        }
    }
}

void SubdivideFace(TriangleMesh* mesh, uint32_t faceId, unsigned int n)
{
    uint32_t fs, fn;
    SubdivideFace(mesh, faceId, n, fs, fn);
}

void SubdivideFace(TriangleMesh* mesh, uint32_t faceId, unsigned int n, uint32_t& newFacesStart, uint32_t& newFacesCount)
{
    TriangleFace face = mesh->faces[faceId];
    mesh->faces.remove(face);
    newFacesStart = mesh->faces.size();
    newFacesCount = n*n;
    
    btVector3 p1 = mesh->vertices[face.vertexIndex[0]];
    btVector3 p2 = mesh->vertices[face.vertexIndex[1]];
    btVector3 p3 = mesh->vertices[face.vertexIndex[2]];
    
    uint32_t nextVertexIndex = mesh->vertices.size();
    btVector3* side12 = new btVector3[n+1];
    btVector3* side13 = new btVector3[n+1];
    uint32_t* side12Id = new uint32_t[n+1];
    uint32_t* side13Id = new uint32_t[n+1];
    uint32_t** spanId = new uint32_t*[n-1];
    
    //subdivision points
    side12[0] = p1;
    side12Id[0] = face.vertexIndex[0];
    side12[n] = p2;
    side12Id[n] = face.vertexIndex[1];
    side13[0] = p1;
    side13Id[0] = face.vertexIndex[0];
    side13[n] = p3;
    side13Id[n] = face.vertexIndex[2];
    
    for(int i=1; i<n; i++)
    {
        side12[i] = p1+(i/(btScalar)n)*(p2-p1);
        mesh->vertices.push_back(side12[i]);
        side12Id[i] = nextVertexIndex;
        nextVertexIndex++;
    }

    for(int i=1; i<n; i++)
    {
        side13[i] = p1+(i/(btScalar)n)*(p3-p1);
        mesh->vertices.push_back(side13[i]);
        side13Id[i] = nextVertexIndex;
        nextVertexIndex++;
    }
    
    //span points
    for(int i=0; i<n-1; i++)
    {
        spanId[i] = new uint32_t[i+1];
        for(int j=0; j<i+1; j++)
        {
            btVector3 v = side12[i+2]+(j+1)/(btScalar)(i+2)*(side13[i+2]-side12[i+2]);
            mesh->vertices.push_back(v);
            spanId[i][j] = nextVertexIndex;
            nextVertexIndex++;
        }
    }
    
    //faces
    TriangleFace f;
    f.normalIndex = face.normalIndex;
    f.glnormalIndex[0] = f.glnormalIndex[1] = f.glnormalIndex[2] = face.glnormalIndex[0];
    f.uvIndex[0] = f.uvIndex[1] = f.uvIndex[2] = 0;
    f.neighbourIndex[0] = f.neighbourIndex[1] = f.neighbourIndex[2] = -1;
    
    //top four
    f.vertexIndex[0] = side12Id[0];
    f.vertexIndex[1] = side12Id[1];
    f.vertexIndex[2] = side13Id[1];
    mesh->faces.push_back(f);
    
    f.vertexIndex[0] = side12Id[1];
    f.vertexIndex[1] = side12Id[2];
    f.vertexIndex[2] = spanId[0][0];
    mesh->faces.push_back(f);
    
    f.vertexIndex[0] = side12Id[1];
    f.vertexIndex[1] = spanId[0][0];
    f.vertexIndex[2] = side13Id[1];
    mesh->faces.push_back(f);
    
    f.vertexIndex[0] = side13Id[1];
    f.vertexIndex[1] = spanId[0][0];
    f.vertexIndex[2] = side13Id[2];
    mesh->faces.push_back(f);
    
    //rest
    for(int i=0; i<n-2; i++)
    {
        //side 12
        f.vertexIndex[0] = side12Id[i+2];
        f.vertexIndex[1] = spanId[i+1][0];
        f.vertexIndex[2] = spanId[i][0];
        mesh->faces.push_back(f);
        
        f.vertexIndex[0] = side12Id[i+2];
        f.vertexIndex[1] = side12Id[i+3];
        f.vertexIndex[2] = spanId[i+1][0];
        mesh->faces.push_back(f);
        
        //center
        for(int j=0; j<i+1; j++)
        {
            f.vertexIndex[0] = spanId[i][j];
            f.vertexIndex[1] = spanId[i+1][j];
            f.vertexIndex[2] = spanId[i+1][j+1];
            mesh->faces.push_back(f);
            
            if(i>0 && j<i)
            {
                f.vertexIndex[0] = spanId[i][j];
                f.vertexIndex[1] = spanId[i+1][j+1];
                f.vertexIndex[2] = spanId[i][j+1];
                mesh->faces.push_back(f);
            }
        }
        
        //side 13
        f.vertexIndex[0] = side13Id[i+2];
        f.vertexIndex[1] = spanId[i][i+1-1];
        f.vertexIndex[2] = spanId[i+1][i+2-1];
        mesh->faces.push_back(f);
        
        f.vertexIndex[0] = side13Id[i+2];
        f.vertexIndex[1] = spanId[i+1][i+2-1];
        f.vertexIndex[2] = side13Id[i+3];
        mesh->faces.push_back(f);
    }
    
    delete [] side12;
    delete [] side12Id;
    delete [] side13;
    delete [] side13Id;
    for(int i=0; i<n-1; i++)
        delete [] spanId[i];
    delete [] spanId;
}

void AABB(TriangleMesh* mesh, btVector3& min, btVector3& max)
{
    btScalar minX=BT_LARGE_FLOAT, maxX=-BT_LARGE_FLOAT;
    btScalar minY=BT_LARGE_FLOAT, maxY=-BT_LARGE_FLOAT;
    btScalar minZ=BT_LARGE_FLOAT, maxZ=-BT_LARGE_FLOAT;
    
    for(int i=0; i<mesh->vertices.size(); i++)
    {
        btVector3 vertex = mesh->vertices[i];
        
        if(vertex.x()>maxX)
            maxX = vertex.x();
        
        if(vertex.x()<minX)
            minX = vertex.x();
        
        if(vertex.y()>maxY)
            maxY = vertex.y();
        
        if(vertex.y()<minY)
            minY = vertex.y();
        
        if(vertex.z()>maxZ)
            maxZ = vertex.z();
        
        if(vertex.z()<minZ)
            minZ = vertex.z();
    }
    
    min = btVector3(minX, minY, minZ);
    max = btVector3(maxX, maxY, maxZ);
}

void AABS(TriangleMesh* mesh, btScalar& bsRadius, btVector3& bsCenterOffset)
{
    btVector3 tempCenter = btVector3(0,0,0);
    
    for(int i=0; i<mesh->vertices.size(); i++)
        tempCenter += mesh->vertices[i];
    
    tempCenter /= (btScalar)mesh->vertices.size();
    
    btScalar radius = 0;
    
    for(int i=0; i<mesh->vertices.size(); i++)
    {
        btVector3 v = mesh->vertices[i];
        btScalar r = (v-tempCenter).length();
        if(r > radius)
            radius = r;
    }
    
    bsRadius = radius;
    bsCenterOffset.setX(tempCenter.x());
    bsCenterOffset.setY(tempCenter.y());
    bsCenterOffset.setZ(tempCenter.z());
}

btScalar signedVolumeOfTetrahedron6(const btVector3& v1, const btVector3& v2, const btVector3& v3)
{
    return v1.dot(v2.cross(v3));
}


btScalar distanceFromCenteredPlane(const btVector3& planeN, const btVector3& v)
{
    btScalar distance = planeN.dot(v);
    return distance == 0.0 ? 10e-9 : distance;
}