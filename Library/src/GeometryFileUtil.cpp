//
//  GeometryFileUtil.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/23/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "GeometryFileUtil.h"
#include "OpenGLUtil.h"

TriangleMesh* LoadModel(const char* filename, btScalar scale, bool smooth)
{
    std::string extension = std::string(filename).substr(strlen(filename)-3,3);
    
    if(extension == "stl" || extension == "STL")
    {
        return LoadSTL(filename, scale, smooth);
    }
    else if(extension == "obj" || extension == "OBJ")
    {
        return LoadOBJ(filename, scale, smooth);
    }
    else
        return NULL;
}

TriangleMesh* LoadSTL(const char *filename, btScalar scale, bool smooth)
{
    //Read STL data
	FILE* file = fopen(filename, "rb");
    char line[256];
    char keyword[10];
    TriangleMesh *mesh = new TriangleMesh();
    /*
    while(fgets(line, 256, file))
    {
        sscanf(line, "%s", keyword);
        
        if(strcmp(keyword, "facet")==0)
            mesh->faceCount++;
    }
    
    fseek(file, 0, SEEK_SET);
    mesh->vertexCount = mesh->faceCount*3;
    mesh->vertices = new btScalar[3*mesh->vertexCount];
    mesh->normalCount = mesh->faceCount;
    mesh->normals = new btScalar[3*mesh->normalCount];
    mesh->glnormalCount = mesh->normalCount;
    mesh->glnormals = new GLfloat[3*mesh->normalCount];
    mesh->uvCount = 0;
    mesh->uvs = NULL;
    mesh->faces = new TriangleFace[mesh->faceCount];
    
    unsigned int fRead = 0;
    unsigned int vRead = 0;
    unsigned int vnRead = 0;
    */
    
    while (fgets(line, 256, file))
    {
        sscanf(line, "%s", keyword);
        
        if(strcmp(keyword, "facet")==0)
        {
            btVector3Data nd;
            
#ifdef BT_USE_DOUBLE_PRECISION
            sscanf(line, " facet normal %lf %lf %lf\n", &nd.m_floats[0], &nd.m_floats[1], &nd.m_floats[2]);
#else
            sscanf(line, " facet normal %f %f %f\n", &nd.m_floats[0], &nd.m_floats[1], &nd.m_floats[2]);
#endif
			btVector3 n = btVector3(nd.m_floats[0], nd.m_floats[1], nd.m_floats[2]);
            mesh->normals.push_back(n);
            
            OpenGLNormal gln;
            gln.x = (GLfloat)n.x();
            gln.y = (GLfloat)n.y();
            gln.z = (GLfloat)n.z();
            mesh->glnormals.push_back(gln);
        }
        else if(strcmp(keyword, "vertex")==0)
        {
            btVector3Data vd;
            
#ifdef BT_USE_DOUBLE_PRECISION
            sscanf(line, " vertex %lf %lf %lf\n", &vd.m_floats[0], &vd.m_floats[1], &vd.m_floats[2]);
#else
            sscanf(line, " vertex %f %f %f\n", &vd.m_floats[0], &vd.m_floats[1], &vd.m_floats[2]);
#endif
			btVector3 v = btVector3(vd.m_floats[0], vd.m_floats[1], vd.m_floats[2]) * scale;
            mesh->vertices.push_back(v);
        }
        else if(strcmp(keyword, "endfacet")==0)
        {
            TriangleFace f;
            uint32_t lastVertexIndex = mesh->vertices.size()-1;
            uint32_t lastNormalIndex = mesh->normals.size()-1;
            f.vertexIndex[0] = lastVertexIndex-2;
            f.vertexIndex[1] = lastVertexIndex-1;
            f.vertexIndex[2] = lastVertexIndex;
            f.normalIndex = lastNormalIndex;
            f.glnormalIndex[0] = f.glnormalIndex[1] = f.glnormalIndex[2] = lastNormalIndex;
            f.uvIndex[0] = f.uvIndex[1] = f.uvIndex[2] = 0;
            mesh->faces.push_back(f);
        }
    }
    fclose(file);
    
    //Remove duplicates (so that it becomes equivalent to OBJ file representation)
    
    
    //Process
    CalculateNeighbours(mesh, smooth);
    
    return mesh;
}

TriangleMesh* LoadOBJ(const char *filename, btScalar scale, bool smooth)
{
    //Read OBJ data
	FILE* file = fopen(filename, "rb");
    char line[256];
    char c1, c2;
    TriangleMesh* mesh = new TriangleMesh();
    bool glnormals = false, gluv = false;
    
    //Read file
    while(fgets(line, 256, file))
    {
        sscanf(line, "%c%c", &c1, &c2);
        if(c1 == 'v')
        {
            if(c2 == ' ')
            {
                btVector3Data vd;
                
#ifdef BT_USE_DOUBLE_PRECISION
                sscanf(line, "v %lf %lf %lf\n", &vd.m_floats[0], &vd.m_floats[1], &vd.m_floats[2]);
#else
                sscanf(line, "v %f %f %f\n", &vd.m_floats[0], &vd.m_floats[1], &vd.m_floats[2]);
#endif
                btVector3 v = btVector3(vd.m_floats[0], vd.m_floats[1], vd.m_floats[2]) * scale;
                mesh->vertices.push_back(v);
            }
            else if(c2 == 'n')
            {
                OpenGLNormal gln;
                sscanf(line, "vn %f %f %f\n", &gln.x, &gln.y, &gln.z);
                mesh->glnormals.push_back(gln);
                glnormals = true;
            }
            else if(c2 == 't')
            {
                OpenGLUV uv;
                sscanf(line, "vt %f %f\n", &uv.u, &uv.v);
                mesh->uvs.push_back(uv);
                gluv = true;
            }
        }
        else if(c1 == 'f')
        {
            TriangleFace face;
            face.neighbourIndex[0] = -1;
            face.neighbourIndex[1] = -1;
            face.neighbourIndex[2] = -1;
            
            if(glnormals && gluv)
            {
                
                sscanf(line, "f %u/%u/%u %u/%u/%u %u/%u/%u\n", &face.vertexIndex[0], &face.uvIndex[0], &face.glnormalIndex[0], &face.vertexIndex[1], &face.uvIndex[1], &face.glnormalIndex[1], &face.vertexIndex[2], &face.uvIndex[2], &face.glnormalIndex[2]);
                face.vertexIndex[0] -= 1;
                face.vertexIndex[1] -= 1;
                face.vertexIndex[2] -= 1;
                face.uvIndex[0] -= 1;
                face.uvIndex[1] -= 1;
                face.uvIndex[2] -= 1;
                face.glnormalIndex[0] -= 1;
                face.glnormalIndex[1] -= 1;
                face.glnormalIndex[2] -= 1;
                
                btVector3 n = btVector3(mesh->glnormals[face.glnormalIndex[0]].x + mesh->glnormals[face.glnormalIndex[1]].x + mesh->glnormals[face.glnormalIndex[2]].x,
                                        mesh->glnormals[face.glnormalIndex[0]].y + mesh->glnormals[face.glnormalIndex[1]].y + mesh->glnormals[face.glnormalIndex[2]].y,
                                        mesh->glnormals[face.glnormalIndex[0]].z + mesh->glnormals[face.glnormalIndex[1]].z + mesh->glnormals[face.glnormalIndex[2]].z);
                n /= (btScalar)3.0;
                mesh->normals.push_back(n);
                face.normalIndex = mesh->normals.size()-1;
            }
            else if(glnormals)
            {
                sscanf(line, "f %u//%u %u//%u %u//%u\n", &face.vertexIndex[0], &face.glnormalIndex[0], &face.vertexIndex[1], &face.glnormalIndex[1], &face.vertexIndex[2], &face.glnormalIndex[2]);
                face.vertexIndex[0] -= 1;
                face.vertexIndex[1] -= 1;
                face.vertexIndex[2] -= 1;
                face.uvIndex[0] = face.uvIndex[1] = face.uvIndex[2] = 0;
                face.glnormalIndex[0] -= 1;
                face.glnormalIndex[1] -= 1;
                face.glnormalIndex[2] -= 1;
                
                btVector3 n = btVector3(mesh->glnormals[face.glnormalIndex[0]].x + mesh->glnormals[face.glnormalIndex[1]].x + mesh->glnormals[face.glnormalIndex[2]].x,
                                        mesh->glnormals[face.glnormalIndex[0]].y + mesh->glnormals[face.glnormalIndex[1]].y + mesh->glnormals[face.glnormalIndex[2]].y,
                                        mesh->glnormals[face.glnormalIndex[0]].z + mesh->glnormals[face.glnormalIndex[1]].z + mesh->glnormals[face.glnormalIndex[2]].z);
                n /= (btScalar)3.0;
                mesh->normals.push_back(n);
                face.normalIndex = mesh->normals.size()-1;
            }
            else
            {
                sscanf(line, "f %u %u %u\n", &face.vertexIndex[0], &face.vertexIndex[1], &face.vertexIndex[2]);
                face.vertexIndex[0] -= 1;
                face.vertexIndex[1] -= 1;
                face.vertexIndex[2] -= 1;
                face.uvIndex[0] = face.uvIndex[1] = face.uvIndex[2] = 0;
                
                btVector3 v1 = btVector3(mesh->vertices[face.vertexIndex[1]].x() - mesh->vertices[face.vertexIndex[0]].x(),
                                         mesh->vertices[face.vertexIndex[1]].y() - mesh->vertices[face.vertexIndex[0]].y(),
                                         mesh->vertices[face.vertexIndex[1]].z() - mesh->vertices[face.vertexIndex[0]].z());
                
                btVector3 v2 = btVector3(mesh->vertices[face.vertexIndex[2]].x() - mesh->vertices[face.vertexIndex[0]].x(),
                                         mesh->vertices[face.vertexIndex[2]].y() - mesh->vertices[face.vertexIndex[0]].y(),
                                         mesh->vertices[face.vertexIndex[2]].z() - mesh->vertices[face.vertexIndex[0]].z());
                
                btVector3 n = v1.cross(v2).normalized();
                
                OpenGLNormal gln;
                gln.x = (GLfloat)n.x();
                gln.y = (GLfloat)n.y();
                gln.z = (GLfloat)n.z();
                mesh->glnormals.push_back(gln);
                face.glnormalIndex[0] = face.glnormalIndex[1] = face.glnormalIndex[2] = mesh->glnormals.size()-1;
                
                mesh->normals.push_back(n);
                face.normalIndex = mesh->normals.size()-1;
            }
            
            mesh->faces.push_back(face);
        }
    }
    
    fclose(file);
    
    //Process
    CalculateNeighbours(mesh, smooth);
    
    return mesh;
}