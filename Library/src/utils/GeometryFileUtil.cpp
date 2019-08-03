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
//  GeometryFileUtil.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 22/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "utils/GeometryFileUtil.h"

#include <algorithm>
#include "core/Console.h"
#include "utils/SystemUtil.hpp"

namespace sf
{

Mesh* LoadGeometryFromFile(const std::string& path, GLfloat scale)
{
    std::string extension = path.substr(path.length()-3,3);
    Mesh* mesh = NULL;
    
    if(extension == "stl" || extension == "STL")
        mesh = LoadSTL(path, scale);
    else if(extension == "obj" || extension == "OBJ")
        mesh = LoadOBJ(path, scale);
    else
        cError("Unsupported geometry file type: %s!", extension.c_str());
    
    return mesh;
}

Mesh* LoadOBJ(const std::string& path, GLfloat scale)
{
    //Read OBJ data
    FILE* file = fopen(path.c_str(), "rb");
    
    if(file == NULL)
    {
        cCritical("Failed to open geometry file: %s", path.c_str());
        return NULL;
    }
    
    cInfo("Loading geometry from: %s", path.c_str());
    
    char line[128];
    Mesh* mesh = new Mesh();
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    bool hasNormals = false;
    size_t genVStart = 0;
    
    int64_t start = GetTimeInMicroseconds();
    
    //Read vertices
    while(fgets(line, 128, file))
    {
        if(line[0] == 'v')
        {
            if(line[1] == ' ')
            {
                Vertex v;
                sscanf(line, "v %f %f %f\n", &v.pos.x, &v.pos.y, &v.pos.z);
                v.pos *= scale; //Scaling
                mesh->vertices.push_back(v);
            }
            else if(line[1] == 'n')
            {
                glm::vec3 n;
                sscanf(line, "vn %f %f %f\n", &n.x, &n.y, &n.z);
                normals.push_back(n);
            }
            else if(line[1] == 't')
            {
                glm::vec2 uv;
                sscanf(line, "vt %f %f\n", &uv.x, &uv.y);
                uvs.push_back(uv);
            }
        }
        else if(line[0] == 'f')
        {
            break;
        }
    }
    
    genVStart = mesh->vertices.size();
    hasNormals = normals.size() > 0;
    mesh->hasUVs = uvs.size() > 0;
    
#ifdef DEBUG
    printf("Vertices: %ld Normals: %ld\n", genVStart, normals.size());
#endif
    
    //Read faces
    do
    {
        if(line[0] != 'f')
            break;
        
        Face face;
        
        if(mesh->hasUVs && hasNormals)
        {
            unsigned int vID[3];
            unsigned int uvID[3];
            unsigned int nID[3];
            sscanf(line, "f %u/%u/%u %u/%u/%u %u/%u/%u\n", &vID[0], &uvID[0], &nID[0], &vID[1], &uvID[1], &nID[1], &vID[2], &uvID[2], &nID[2]);
            
            for(short unsigned int i=0; i<3; ++i)
            {
                Vertex v = mesh->vertices[vID[i]-1]; //Vertex from previously read pool
                
                if(glm::length2(v.normal) == 0.f) //Is it a fresh vertex?
                {
                    mesh->vertices[vID[i]-1].normal = normals[nID[i]-1];
                    mesh->vertices[vID[i]-1].uv = uvs[uvID[i]-1];
                    face.vertexID[i] = vID[i]-1;
                }
                else if((v.normal == normals[nID[i]-1]) && (v.uv == uvs[uvID[i]-1])) //Does it have the same normal and UV?
                {
                    face.vertexID[i] = vID[i]-1;
                }
                else //Otherwise search the generated pool
                {
                    v.normal = normals[nID[i]-1];
                    v.uv = uvs[uvID[i]-1];
                    
                    std::vector<Vertex>::iterator it;
                    if((it = std::find(mesh->vertices.begin()+genVStart, mesh->vertices.end(), v)) != mesh->vertices.end()) //If vertex exists
                    {
                        face.vertexID[i] = (GLuint)(it - mesh->vertices.begin());
                    }
                    else
                    {
                        mesh->vertices.push_back(v);
                        face.vertexID[i] = (GLuint)mesh->vertices.size()-1;
                    }
                }
            }
        }
        else if(hasNormals)
        {
            unsigned int vID[3];
            unsigned int nID[3];
            sscanf(line, "f %u//%u %u//%u %u//%u\n", &vID[0], &nID[0], &vID[1], &nID[1], &vID[2], &nID[2]);
            
            for(short unsigned int i=0; i<3; ++i)
            {
                Vertex v = mesh->vertices[vID[i]-1]; //Vertex from previously read pool
                
                if(glm::length2(v.normal) == 0.f) //Is it a fresh vertex?
                {
                    mesh->vertices[vID[i]-1].normal = normals[nID[i]-1];
                    face.vertexID[i] = vID[i]-1;
                }
                else if(v.normal == normals[nID[i]-1]) //Does it have the same normal?
                {
                    face.vertexID[i] = vID[i]-1;
                }
                else //Otherwise search the generated pool
                {
                    v.normal = normals[nID[i]-1];
                    
                    std::vector<Vertex>::iterator it;
                    if((it = std::find(mesh->vertices.begin()+genVStart, mesh->vertices.end(), v)) != mesh->vertices.end()) //If vertex exists
                    {
                        face.vertexID[i] = (GLuint)(it - mesh->vertices.begin());
                    }
                    else
                    {
                        mesh->vertices.push_back(v);
                        face.vertexID[i] = (GLuint)mesh->vertices.size()-1;
                    }
                }
            }
        }
        else
        {
            unsigned int vID[3];
            sscanf(line, "f %u %u %u\n", &vID[0], &vID[1], &vID[2]);
            
            face.vertexID[0] = vID[0]-1;
            face.vertexID[1] = vID[1]-1;
            face.vertexID[2] = vID[2]-1;
        }
        
        mesh->faces.push_back(face);
    }
    while(fgets(line, 128, file));
    
    fclose(file);
    
    int64_t end = GetTimeInMicroseconds();
    
#ifdef DEBUG
    printf("Loaded: %ld Generated: %ld\n", genVStart, mesh->vertices.size()-genVStart);
    printf("Total time: %ld\n", (long int)(end-start));
#endif
    
    cInfo("Loaded mesh with %ld faces in %ld ms.", mesh->faces.size(), (end-start)/1000);
    
    return mesh;
}

Mesh* LoadSTL(const std::string& path, GLfloat scale)
{
    //Read STL data
    FILE* file = fopen(path.c_str(), "rb");   
    
    if(file == NULL)
    {
        cCritical("Failed to open geometry file: %s", path.c_str());
        return NULL;
    }
    
    cInfo("Loading geometry from: %s", path.c_str());
    
    char line[128];
    char keyword[10];
    Mesh *mesh = new Mesh();
    mesh->hasUVs = false;
    
    Vertex v;
    
    while(fgets(line, 128, file))
    {
        sscanf(line, "%s", keyword);
        
        if(strcmp(keyword, "facet")==0)
        {
            sscanf(line, " facet normal %f %f %f\n", &v.normal.x, &v.normal.y, &v.normal.z);
        }
        else if(strcmp(keyword, "vertex")==0)
        {
            sscanf(line, " vertex %f %f %f\n", &v.pos.x, &v.pos.y, &v.pos.z);
            v.pos *= scale;
            mesh->vertices.push_back(v);
        }
        else if(strcmp(keyword, "endfacet")==0)
        {
            unsigned int lastVertexID = (GLuint)mesh->vertices.size()-1;
            
            Face f;
            f.vertexID[0] = lastVertexID-2;
            f.vertexID[1] = lastVertexID-1;
            f.vertexID[2] = lastVertexID;
            mesh->faces.push_back(f);
        }
    }
    
    fclose(file);
    
    //Remove duplicates (so that it becomes equivalent to OBJ file representation)
    
    return mesh;
}

}
