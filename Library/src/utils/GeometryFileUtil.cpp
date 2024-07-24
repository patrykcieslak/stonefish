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
//  Copyright (c) 2018-2023 Patryk Cieslak. All rights reserved.
//

#include "utils/GeometryFileUtil.h"

#include <algorithm>
#include "core/SimulationApp.h"
#include "utils/SystemUtil.hpp"

#include <unordered_map>

namespace sf
{

Mesh* LoadGeometryFromFile(const std::string& path, GLfloat scale)
{
    std::string extension = path.substr(path.length()-3,3);
    Mesh* mesh = nullptr;
    
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
        return nullptr;
    }
    
    cInfo("Loading geometry from: %s", path.c_str());
    
    char line[1024];
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    Mesh* mesh_ = nullptr;
    const int64_t start = GetTimeInMicroseconds();
    
    //Read vertices
    while(fgets(line, 1024, file))
    {
        if(line[0] == 'v')
        {
            if(line[1] == ' ')
            {
                glm::vec3 v;
                sscanf(line, "v %f %f %f\n", &v.x, &v.y, &v.z);
                v *= scale; //Scaling
                positions.push_back(v);
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
    }
    fseek(file, 0, SEEK_SET); //Go back to beginning of file
    
    const std::size_t genVStart = positions.size();
    const bool hasNormals = normals.size() > 0;
    const bool hasUVs = uvs.size() > 0;

#ifdef DEBUG
    printf("Vertices: %ld Normals: %ld\n", genVStart, normals.size());
#endif
    
    if(hasUVs)
    {
        TexturableMesh* mesh = new TexturableMesh;

        mesh->vertices.reserve(std::max(positions.size(), normals.size()));

        std::unordered_map<TexturableVertex, GLuint, TexturableVertexHash> vertexCache;

        //Read faces
        while(fgets(line, 1024, file))
        {
            if(line[0] != 'f')
                continue;
        
            Face face;
            unsigned int vID[3];
            unsigned int uvID[3];
            unsigned int nID[3];

            if(hasNormals)
            {
                sscanf(line, "f %u/%u/%u %u/%u/%u %u/%u/%u\n", &vID[0], &uvID[0], &nID[0], &vID[1], &uvID[1], &nID[1], &vID[2], &uvID[2], &nID[2]);
            }
            else
            {
                sscanf(line, "f %u/%u %u/%u %u/%u\n", &vID[0], &uvID[0], &vID[1], &uvID[1], &vID[2], &uvID[2]);
            }
        
            for(short unsigned int i=0; i<3; ++i)
            {
                TexturableVertex v;
                v.pos = positions[vID[i]-1];
                v.uv = uvs[uvID[i]-1];
                if(hasNormals)
                {
                    v.normal = normals[nID[i]-1];
                }

                const auto it = vertexCache.find(v);
                if(it != vertexCache.end()) 
                {
                    face.vertexID[i] = it->second;
                } 
                else 
                {
                    mesh->vertices.push_back(std::move(v));
                    face.vertexID[i] = (GLuint)mesh->vertices.size() - 1;
                    vertexCache[v] = face.vertexID[i];
                }
            }

            mesh->faces.push_back(face);
        }
        mesh_ = mesh;
    }
    else    
    {
        PlainMesh* mesh = new PlainMesh;

        mesh->vertices.reserve(std::max(positions.size(), normals.size()));

        std::unordered_map<Vertex, GLuint, VertexHash> vertexCache;

        if (!hasNormals)
        {
            //Initialize positions
            for(size_t i=0; i<positions.size(); ++i)
            {
                Vertex v;
                v.pos = positions[i];
                mesh->vertices.push_back(std::move(v));
            }
        }

        //Read faces
        while(fgets(line, 1024, file))
        {
            if(line[0] != 'f')
                continue;
        
            Face face;

            if(hasNormals)
            {
                unsigned int vID[3];
                unsigned int nID[3];
                sscanf(line, "f %u//%u %u//%u %u//%u\n", &vID[0], &nID[0], &vID[1], &nID[1], &vID[2], &nID[2]);
                
                for(short unsigned int i=0; i<3; ++i)
                {
                    Vertex v;
                    v.pos = positions[vID[i]-1];
                    v.normal = normals[nID[i]-1];

                    const auto it = vertexCache.find(v);
                    if(it != vertexCache.end()) 
                    {
                        face.vertexID[i] = it->second;
                    } 
                    else 
                    {
                        mesh->vertices.push_back(std::move(v));
                        face.vertexID[i] = (GLuint)mesh->vertices.size() - 1;
                        vertexCache[v] = face.vertexID[i];
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
        mesh_ = mesh;
    }
    fclose(file);
    
    const int64_t end = GetTimeInMicroseconds();
    
#ifdef DEBUG
    printf("Loaded: %ld Generated: %ld\n", genVStart, mesh_->getNumOfVertices()-genVStart);
    printf("Total time: %ld\n", (long int)(end-start));
#endif
    cInfo("Loaded mesh with %ld faces in %ld ms.", mesh_->faces.size(), (end-start)/1000);
    return mesh_;
}

Mesh* LoadSTL(const std::string& path, GLfloat scale)
{
    //Read STL data
    FILE* file = fopen(path.c_str(), "rb");   
    
    if(file == NULL)
    {
        cCritical("Failed to open geometry file: %s", path.c_str());
        return nullptr;
    }
    
    cInfo("Loading geometry from: %s", path.c_str());
    
    char line[128];
    char keyword[10];
    PlainMesh* mesh = new PlainMesh;
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

void ComputePhysicalProperties(const Mesh* mesh, Scalar thickness, Scalar density, Scalar& mass, Vector3& CG, Scalar& volume, Scalar& surface, Vector3& Ipri, Matrix3& Irot)
{
    //1.Calculate mesh volume, CG and mass
    CG = Vector3(0,0,0);
    volume = Scalar(0);
    surface = Scalar(0);
    
    if(thickness > Scalar(0)) //Shell
    {
        for(size_t i=0; i<mesh->faces.size(); ++i)
        {
            //Get triangle, convert from OpenGL to physics
            glm::vec3 v1gl = mesh->getVertexPos(i, 0);
            glm::vec3 v2gl = mesh->getVertexPos(i, 1);
            glm::vec3 v3gl = mesh->getVertexPos(i, 2);
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            
            //Calculate volume of shell triangle
            Scalar A = (v2-v1).cross(v3-v1).length()/Scalar(2);
            surface += A;
            Vector3 triCG = (v1+v2+v3)/Scalar(3);
            Scalar triVolume = A * thickness;
            CG += triCG * triVolume;
            volume += triVolume;
        }
        
        //Compute mesh CG
        if(volume > Scalar(0))
            CG /= volume;
        else
            CG = Vector3(0,0,0);
    }
    else //Solid body
    {
        for(size_t i=0; i<mesh->faces.size(); ++i)
        {
            //Get triangle, convert from OpenGL to physics
            glm::vec3 v1gl = mesh->getVertexPos(i, 0);
            glm::vec3 v2gl = mesh->getVertexPos(i, 1);
            glm::vec3 v3gl = mesh->getVertexPos(i, 2);
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);

            //Calculate surface
            surface += (v2-v1).cross(v3-v1).length()/Scalar(2);

            //Calculate signed volume of a tetrahedra
            Vector3 tetraCG = (v1+v2+v3)/Scalar(4);
            Scalar tetraVolume6 = v1.dot(v2.cross(v3));
            CG += tetraCG * tetraVolume6;
            volume += tetraVolume6;
        }
        
        //Compute mesh CG
        if(volume > Scalar(0))
            CG /= volume;
        else
            CG = Vector3(0,0,0);
        
        //Compute mesh volume
        volume /= Scalar(6);
    }
    
    mass = volume * density;
    
    //2.Calculate moments of inertia for local coordinate system located in CG (not necessarily principal)
    Matrix3 I;
    
    if(thickness > Scalar(0)) //Shell - I have doubts if it is correct!
    {
        //Compute properties a shell by subtracting the inner solid from the outer solid
        //Outer solid -> eternal surface
        Scalar Pxx = Scalar(0);
        Scalar Pyy = Scalar(0);
        Scalar Pzz = Scalar(0);
        Scalar Pxy = Scalar(0);
        Scalar Pxz = Scalar(0);
        Scalar Pyz = Scalar(0);
        
        for(size_t i=0; i<mesh->faces.size(); ++i)
        {
            //Triangle verticies with respect to CG
            glm::vec3 v1gl = mesh->getVertexPos(i, 0);
            glm::vec3 v2gl = mesh->getVertexPos(i, 1);
            glm::vec3 v3gl = mesh->getVertexPos(i, 2);
            
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            Vector3 n = (v2-v1).cross(v3-v1).normalize();
            v1 = v1 + n*thickness/Scalar(2) - CG;
            v2 = v2 + n*thickness/Scalar(2) - CG;
            v3 = v3 + n*thickness/Scalar(2) - CG;
            
            //Pjk = const * dV * (2*Aj*Ak + 2*Bj*Bk + 2*Cj*Ck + Aj*Bk + Ak*Bj + Aj*Ck + Ak*Cj + Bj*Ck + Bk*Cj)
            Scalar V6 = v1.dot(v2.cross(v3));
            Pxx += V6 * 2 *(v1.x()*v1.x() + v2.x()*v2.x() + v3.x()*v3.x() + v1.x()*v2.x() + v1.x()*v3.x() + v2.x()*v3.x());
            Pyy += V6 * 2 *(v1.y()*v1.y() + v2.y()*v2.y() + v3.y()*v3.y() + v1.y()*v2.y() + v1.y()*v3.y() + v2.y()*v3.y());
            Pzz += V6 * 2 *(v1.z()*v1.z() + v2.z()*v2.z() + v3.z()*v3.z() + v1.z()*v2.z() + v1.z()*v3.z() + v2.z()*v3.z());
            Pxy += V6 * (2*(v1.x()*v1.y() + v2.x()*v2.y() + v3.x()*v3.y()) + v1.x()*v2.y() + v1.y()*v2.x() + v1.x()*v3.y() + v1.y()*v3.x() + v2.x()*v3.y() + v2.y()*v3.x());
            Pxz += V6 * (2*(v1.x()*v1.z() + v2.x()*v2.z() + v3.x()*v3.z()) + v1.x()*v2.z() + v1.z()*v2.x() + v1.x()*v3.z() + v1.z()*v3.x() + v2.x()*v3.z() + v2.z()*v3.x());
            Pyz += V6 * (2*(v1.y()*v1.z() + v2.y()*v2.z() + v3.y()*v3.z()) + v1.y()*v2.z() + v1.z()*v2.y() + v1.y()*v3.z() + v1.z()*v3.y() + v2.y()*v3.z() + v2.z()*v3.y());
        }
        
        Pxx *= density / Scalar(120); //20 from formula and 6 from polyhedron volume
        Pyy *= density / Scalar(120);
        Pzz *= density / Scalar(120);
        Pxy *= density / Scalar(120);
        Pxz *= density / Scalar(120);
        Pyz *= density / Scalar(120);
        
        I = Matrix3(Pyy+Pzz, -Pxy, -Pxz, -Pxy, Pxx+Pzz, -Pyz, -Pxz, -Pyz, Pxx+Pyy);
        
        //Inner solid -> internal surface
        Pxx = Scalar(0);
        Pyy = Scalar(0);
        Pzz = Scalar(0);
        Pxy = Scalar(0);
        Pxz = Scalar(0);
        Pyz = Scalar(0); //products of inertia
        
        for(unsigned int i=0; i<mesh->faces.size(); ++i)
        {
            //Triangle verticies with respect to CG
            glm::vec3 v1gl = mesh->getVertexPos(i, 0);
            glm::vec3 v2gl = mesh->getVertexPos(i, 1);
            glm::vec3 v3gl = mesh->getVertexPos(i, 2);
            
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            Vector3 n = (v2-v1).cross(v3-v1).normalize();
            v1 = v1 - n*thickness/Scalar(2) - CG;
            v2 = v2 - n*thickness/Scalar(2) - CG;
            v3 = v3 - n*thickness/Scalar(2) - CG;
            
            //Pjk = const * dV * (2*Aj*Ak + 2*Bj*Bk + 2*Cj*Ck + Aj*Bk + Ak*Bj + Aj*Ck + Ak*Cj + Bj*Ck + Bk*Cj)
            Scalar V6 = v1.dot(v2.cross(v3));
            Pxx += V6 * 2 *(v1.x()*v1.x() + v2.x()*v2.x() + v3.x()*v3.x() + v1.x()*v2.x() + v1.x()*v3.x() + v2.x()*v3.x());
            Pyy += V6 * 2 *(v1.y()*v1.y() + v2.y()*v2.y() + v3.y()*v3.y() + v1.y()*v2.y() + v1.y()*v3.y() + v2.y()*v3.y());
            Pzz += V6 * 2 *(v1.z()*v1.z() + v2.z()*v2.z() + v3.z()*v3.z() + v1.z()*v2.z() + v1.z()*v3.z() + v2.z()*v3.z());
            Pxy += V6 * (2*(v1.x()*v1.y() + v2.x()*v2.y() + v3.x()*v3.y()) + v1.x()*v2.y() + v1.y()*v2.x() + v1.x()*v3.y() + v1.y()*v3.x() + v2.x()*v3.y() + v2.y()*v3.x());
            Pxz += V6 * (2*(v1.x()*v1.z() + v2.x()*v2.z() + v3.x()*v3.z()) + v1.x()*v2.z() + v1.z()*v2.x() + v1.x()*v3.z() + v1.z()*v3.x() + v2.x()*v3.z() + v2.z()*v3.x());
            Pyz += V6 * (2*(v1.y()*v1.z() + v2.y()*v2.z() + v3.y()*v3.z()) + v1.y()*v2.z() + v1.z()*v2.y() + v1.y()*v3.z() + v1.z()*v3.y() + v2.y()*v3.z() + v2.z()*v3.y());
        }
        
        Pxx *= density / Scalar(120); //20 from formula and 6 from polyhedron volume
        Pyy *= density / Scalar(120);
        Pzz *= density / Scalar(120);
        Pxy *= density / Scalar(120);
        Pxz *= density / Scalar(120);
        Pyz *= density / Scalar(120);
        
        I -= Matrix3(Pyy+Pzz, -Pxy, -Pxz, -Pxy, Pxx+Pzz, -Pyz, -Pxz, -Pyz, Pxx+Pyy);
    }
    else
    {
        Scalar Pxx = Scalar(0);
        Scalar Pyy = Scalar(0);
        Scalar Pzz = Scalar(0);
        Scalar Pxy = Scalar(0);
        Scalar Pxz = Scalar(0);
        Scalar Pyz = Scalar(0);
        
        for(size_t i=0; i<mesh->faces.size(); ++i)
        {
            //Triangle verticies with respect to CG
            glm::vec3 v1gl = mesh->getVertexPos(i, 0);
            glm::vec3 v2gl = mesh->getVertexPos(i, 1);
            glm::vec3 v3gl = mesh->getVertexPos(i, 2);
            
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            v1 -= CG;
            v2 -= CG;
            v3 -= CG;
            
            //Pjk = const * dV * (2*Aj*Ak + 2*Bj*Bk + 2*Cj*Ck + Aj*Bk + Ak*Bj + Aj*Ck + Ak*Cj + Bj*Ck + Bk*Cj)
            Scalar V6 = v1.dot(v2.cross(v3));
            Pxx += V6 * 2 *(v1.x()*v1.x() + v2.x()*v2.x() + v3.x()*v3.x() + v1.x()*v2.x() + v1.x()*v3.x() + v2.x()*v3.x());
            Pyy += V6 * 2 *(v1.y()*v1.y() + v2.y()*v2.y() + v3.y()*v3.y() + v1.y()*v2.y() + v1.y()*v3.y() + v2.y()*v3.y());
            Pzz += V6 * 2 *(v1.z()*v1.z() + v2.z()*v2.z() + v3.z()*v3.z() + v1.z()*v2.z() + v1.z()*v3.z() + v2.z()*v3.z());
            Pxy += V6 * (2*(v1.x()*v1.y() + v2.x()*v2.y() + v3.x()*v3.y()) + v1.x()*v2.y() + v1.y()*v2.x() + v1.x()*v3.y() + v1.y()*v3.x() + v2.x()*v3.y() + v2.y()*v3.x());
            Pxz += V6 * (2*(v1.x()*v1.z() + v2.x()*v2.z() + v3.x()*v3.z()) + v1.x()*v2.z() + v1.z()*v2.x() + v1.x()*v3.z() + v1.z()*v3.x() + v2.x()*v3.z() + v2.z()*v3.x());
            Pyz += V6 * (2*(v1.y()*v1.z() + v2.y()*v2.z() + v3.y()*v3.z()) + v1.y()*v2.z() + v1.z()*v2.y() + v1.y()*v3.z() + v1.z()*v3.y() + v2.y()*v3.z() + v2.z()*v3.y());
        }
        
        Pxx *= density / Scalar(120); //20 from formula and 6 from polyhedron volume
        Pyy *= density / Scalar(120);
        Pzz *= density / Scalar(120);
        Pxy *= density / Scalar(120);
        Pxz *= density / Scalar(120);
        Pyz *= density / Scalar(120);
        
        I = Matrix3(Pyy+Pzz, -Pxy, -Pxz, -Pxy, Pxx+Pzz, -Pyz, -Pxz, -Pyz, Pxx+Pyy);
    }
    
    //3. Find primary moments of inertia
    Ipri = Vector3(I.getRow(0).getX(), I.getRow(1).getY(), I.getRow(2).getZ());
    Irot = I3();
    
    if(!IsDiagonal(I)) // If inertia matrix is not diagonal
    {
        //3.1. Calculate principal moments of inertia
        Scalar T = I[0][0] + I[1][1] + I[2][2]; //Ixx + Iyy + Izz
        Scalar II = I[0][0]*I[1][1] + I[0][0]*I[2][2] + I[1][1]*I[2][2] - I[0][1]*I[0][1] - I[0][2]*I[0][2] - I[1][2]*I[1][2]; //Ixx Iyy + Ixx Izz + Iyy Izz - Ixy^2 - Ixz^2 - Iyz^2
        Scalar U = btSqrt(T*T-Scalar(3)*II)/Scalar(3);
        Scalar theta = btAcos((-Scalar(2)*T*T*T + Scalar(9)*T*II - Scalar(27)*I.determinant())/(Scalar(54)*U*U*U));
        Scalar A = T/Scalar(3) - Scalar(2)*U*btCos(theta/Scalar(3));
        Scalar B = T/Scalar(3) - Scalar(2)*U*btCos(theta/Scalar(3) - Scalar(2)*M_PI/Scalar(3));
        Scalar C = T/Scalar(3) - Scalar(2)*U*btCos(theta/Scalar(3) + Scalar(2)*M_PI/Scalar(3));
        Ipri = Vector3(A, B, C);
        
        //3.2. Calculate principal axes of inertia
        Matrix3 L;
        Vector3 axis1,axis2,axis3;
        axis1 = FindInertialAxis(I, A);
        axis2 = FindInertialAxis(I, B);
        axis3 = axis1.cross(axis2);
        axis2 = axis3.cross(axis1);
        
        //3.3. Rotate body so that principal axes are parallel to (x,y,z) system
        Irot = Matrix3(axis1[0],axis2[0],axis3[0], axis1[1],axis2[1],axis3[1], axis1[2],axis2[2],axis3[2]);
    }
}

MeshProperties ComputePhysicalProperties(const Mesh* mesh, Scalar thickness, Scalar density)
{
    MeshProperties mp;
    ComputePhysicalProperties(mesh, thickness, density, mp.mass, mp.CG, mp.volume, mp.surface, mp.Ipri, mp.Irot);
    return mp;
}

Vector3 FindInertialAxis(const Matrix3& I, Scalar value)
{
    //Diagonalize
    Matrix3 L;
    Vector3 candidates[3];
    Vector3 axis;
    
    //Characteristic matrix
    L = I - Matrix3::getIdentity().scaled(Vector3(value,value,value));
    
    //Candidates (orthogonal vectors)
    candidates[0] = (L.getRow(0).cross(L.getRow(1)));
    candidates[1] = (L.getRow(0).cross(L.getRow(2)));
    candidates[2] = (L.getRow(1).cross(L.getRow(2)));
    
    //Find best candidate
    if(candidates[0].length2() >= candidates[1].length2())
    {
        if(candidates[0].length2() >= candidates[2].length2())
            axis = candidates[0].normalized();
        else
            axis = candidates[2].normalized();
    }
    else
    {
        if(candidates[1].length2() >= candidates[2].length2())
            axis = candidates[1].normalized();
        else
            axis = candidates[2].normalized();
    }   
    return axis;
}

bool IsDiagonal(const Matrix3& A)
{
    Scalar minDiagElem = btMin( btMin( btFabs(A.getRow(0).getX()), btFabs(A.getRow(1).getY())), btFabs(A.getRow(2).getZ()) );
    const Scalar delta = 1e6;
    if(btFabs(A.getRow(0).getY()) < minDiagElem/delta
        && btFabs(A.getRow(0).getZ()) < minDiagElem/delta
        && btFabs(A.getRow(1).getX()) < minDiagElem/delta
        && btFabs(A.getRow(1).getZ()) < minDiagElem/delta
        && btFabs(A.getRow(2).getX()) < minDiagElem/delta
        && btFabs(A.getRow(2).getY()) < minDiagElem/delta)
    {
        return true;
    }
    return false;
}

}
