//
//  Polyhedron.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright (c) 2012-2018 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Polyhedron.h"

#include "utils/SystemUtil.hpp"
#include "utils/MathUtil.hpp"

namespace sf
{

Polyhedron::Polyhedron(std::string uniqueName, std::string modelFilename, Scalar scale, const Transform& originTrans, Material m, int lookId, bool smoothNormals, Scalar thickness, bool isBuoyant, HydrodynamicProxyType geoProxy) : SolidEntity(uniqueName, m, lookId, thickness, isBuoyant)
{
    //1.Load triangle mesh from file
    mesh = OpenGLContent::LoadMesh(modelFilename, scale, smoothNormals);
    transformMesh(mesh, originTrans);
    
    //2.Calculate mesh volume and COG    
    Vector3 meshCog(0,0,0);
    
    if(thick > Scalar(0))
    {
        Scalar meshVolume = 0;
        
        for(unsigned int i=0; i<mesh->faces.size(); ++i)
        {
            //triangle
            glm::vec3 v1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
		
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
        
            Scalar A = (v2-v1).cross(v3-v1).length()/Scalar(2);
            Vector3 triCOG = (v1+v2+v3)/Scalar(3);
            Scalar triVolume = A * thick;
            meshCog += triCOG * triVolume;
            meshVolume += triVolume;
        }
        
        if(meshVolume > Scalar(0))
            meshCog /= meshVolume;
        else
            meshCog = Vector3(0,0,0);
        
        volume = meshVolume;
    }
    else
    {
        Scalar meshVolume = 0;
    
        for(unsigned int i=0; i<mesh->faces.size(); ++i)
        {
            //triangle
            glm::vec3 v1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
		
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
        
            Vector3 tetraCOG = (v1+v2+v3)/Scalar(4);
            Scalar tetraVolume6 = v1.dot(v2.cross(v3));
            meshCog += tetraCOG * tetraVolume6;
            meshVolume += tetraVolume6;
        }
    
        if(meshVolume > 0.0)
            meshCog /= meshVolume;
        else
            meshCog = Vector3(0,0,0);
    
        volume = meshVolume * Scalar(1)/Scalar(6);
    }
    
    //3.Move vertices so that COG is in (0,0,0)
    T_G2CG.setOrigin(meshCog);
    
    //4.Calculate moments of inertia for local coordinate system located in COG (not necessarily principal)
    Matrix3 I;
    
    if(thick > Scalar(0))
    {
        //External
        Scalar Pxx = Scalar(0);
        Scalar Pyy = Scalar(0);
        Scalar Pzz = Scalar(0);
        Scalar Pxy = Scalar(0);
        Scalar Pxz = Scalar(0);
        Scalar Pyz = Scalar(0);
    
        for(unsigned int i=0; i<mesh->faces.size(); ++i)
        {
            //Triangle verticies with respect to COG
            glm::vec3 v1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
		
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            Vector3 n = (v2-v1).cross(v3-v1).normalize();
            v1 = v1 + n*thick/Scalar(2) - meshCog;
            v2 = v2 + n*thick/Scalar(2) - meshCog;
            v3 = v3 + n*thick/Scalar(2) - meshCog;
        
            //Pjk = const * dV * (2*Aj*Ak + 2*Bj*Bk + 2*Cj*Ck + Aj*Bk + Ak*Bj + Aj*Ck + Ak*Cj + Bj*Ck + Bk*Cj)
            Scalar V6 = v1.dot(v2.cross(v3));
            Pxx += V6 * 2 *(v1.x()*v1.x() + v2.x()*v2.x() + v3.x()*v3.x() + v1.x()*v2.x() + v1.x()*v3.x() + v2.x()*v3.x());
            Pyy += V6 * 2 *(v1.y()*v1.y() + v2.y()*v2.y() + v3.y()*v3.y() + v1.y()*v2.y() + v1.y()*v3.y() + v2.y()*v3.y());
            Pzz += V6 * 2 *(v1.z()*v1.z() + v2.z()*v2.z() + v3.z()*v3.z() + v1.z()*v2.z() + v1.z()*v3.z() + v2.z()*v3.z());
            Pxy += V6 * (2*(v1.x()*v1.y() + v2.x()*v2.y() + v3.x()*v3.y()) + v1.x()*v2.y() + v1.y()*v2.x() + v1.x()*v3.y() + v1.y()*v3.x() + v2.x()*v3.y() + v2.y()*v3.x());
            Pxz += V6 * (2*(v1.x()*v1.z() + v2.x()*v2.z() + v3.x()*v3.z()) + v1.x()*v2.z() + v1.z()*v2.x() + v1.x()*v3.z() + v1.z()*v3.x() + v2.x()*v3.z() + v2.z()*v3.x());
            Pyz += V6 * (2*(v1.y()*v1.z() + v2.y()*v2.z() + v3.y()*v3.z()) + v1.y()*v2.z() + v1.z()*v2.y() + v1.y()*v3.z() + v1.z()*v3.y() + v2.y()*v3.z() + v2.z()*v3.y());
        }
    
        Pxx *= mat.density / Scalar(120); //20 from formula and 6 from polyhedron volume
        Pyy *= mat.density / Scalar(120);
        Pzz *= mat.density / Scalar(120);
        Pxy *= mat.density / Scalar(120);
        Pxz *= mat.density / Scalar(120);
        Pyz *= mat.density / Scalar(120);
    
        I = Matrix3(Pyy+Pzz, -Pxy, -Pxz, -Pxy, Pxx+Pzz, -Pyz, -Pxz, -Pyz, Pxx+Pyy);
            
        //Internal
        Pxx = Scalar(0);
        Pyy = Scalar(0);
        Pzz = Scalar(0);
        Pxy = Scalar(0);
        Pxz = Scalar(0);
        Pyz = Scalar(0); //products of inertia
    
        for(unsigned int i=0; i<mesh->faces.size(); ++i)
        {
            //Triangle verticies with respect to COG
            glm::vec3 v1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
		
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            Vector3 n = (v2-v1).cross(v3-v1).normalize();
            v1 = v1 - n*thick/Scalar(2) - meshCog;
            v2 = v2 - n*thick/Scalar(2) - meshCog;
            v3 = v3 - n*thick/Scalar(2) - meshCog;
        
            //Pjk = const * dV * (2*Aj*Ak + 2*Bj*Bk + 2*Cj*Ck + Aj*Bk + Ak*Bj + Aj*Ck + Ak*Cj + Bj*Ck + Bk*Cj)
            Scalar V6 = v1.dot(v2.cross(v3));
            Pxx += V6 * 2 *(v1.x()*v1.x() + v2.x()*v2.x() + v3.x()*v3.x() + v1.x()*v2.x() + v1.x()*v3.x() + v2.x()*v3.x());
            Pyy += V6 * 2 *(v1.y()*v1.y() + v2.y()*v2.y() + v3.y()*v3.y() + v1.y()*v2.y() + v1.y()*v3.y() + v2.y()*v3.y());
            Pzz += V6 * 2 *(v1.z()*v1.z() + v2.z()*v2.z() + v3.z()*v3.z() + v1.z()*v2.z() + v1.z()*v3.z() + v2.z()*v3.z());
            Pxy += V6 * (2*(v1.x()*v1.y() + v2.x()*v2.y() + v3.x()*v3.y()) + v1.x()*v2.y() + v1.y()*v2.x() + v1.x()*v3.y() + v1.y()*v3.x() + v2.x()*v3.y() + v2.y()*v3.x());
            Pxz += V6 * (2*(v1.x()*v1.z() + v2.x()*v2.z() + v3.x()*v3.z()) + v1.x()*v2.z() + v1.z()*v2.x() + v1.x()*v3.z() + v1.z()*v3.x() + v2.x()*v3.z() + v2.z()*v3.x());
            Pyz += V6 * (2*(v1.y()*v1.z() + v2.y()*v2.z() + v3.y()*v3.z()) + v1.y()*v2.z() + v1.z()*v2.y() + v1.y()*v3.z() + v1.z()*v3.y() + v2.y()*v3.z() + v2.z()*v3.y());
        }
    
        Pxx *= mat.density / Scalar(120); //20 from formula and 6 from polyhedron volume
        Pyy *= mat.density / Scalar(120);
        Pzz *= mat.density / Scalar(120);
        Pxy *= mat.density / Scalar(120);
        Pxz *= mat.density / Scalar(120);
        Pyz *= mat.density / Scalar(120);
    
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
    
        for(unsigned int i=0; i<mesh->faces.size(); ++i)
        {
            //Triangle verticies with respect to COG
            glm::vec3 v1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
		
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            v1 -= meshCog;
            v2 -= meshCog;
            v3 -= meshCog;
        
            //Pjk = const * dV * (2*Aj*Ak + 2*Bj*Bk + 2*Cj*Ck + Aj*Bk + Ak*Bj + Aj*Ck + Ak*Cj + Bj*Ck + Bk*Cj)
            Scalar V6 = v1.dot(v2.cross(v3));
            Pxx += V6 * 2 *(v1.x()*v1.x() + v2.x()*v2.x() + v3.x()*v3.x() + v1.x()*v2.x() + v1.x()*v3.x() + v2.x()*v3.x());
            Pyy += V6 * 2 *(v1.y()*v1.y() + v2.y()*v2.y() + v3.y()*v3.y() + v1.y()*v2.y() + v1.y()*v3.y() + v2.y()*v3.y());
            Pzz += V6 * 2 *(v1.z()*v1.z() + v2.z()*v2.z() + v3.z()*v3.z() + v1.z()*v2.z() + v1.z()*v3.z() + v2.z()*v3.z());
            Pxy += V6 * (2*(v1.x()*v1.y() + v2.x()*v2.y() + v3.x()*v3.y()) + v1.x()*v2.y() + v1.y()*v2.x() + v1.x()*v3.y() + v1.y()*v3.x() + v2.x()*v3.y() + v2.y()*v3.x());
            Pxz += V6 * (2*(v1.x()*v1.z() + v2.x()*v2.z() + v3.x()*v3.z()) + v1.x()*v2.z() + v1.z()*v2.x() + v1.x()*v3.z() + v1.z()*v3.x() + v2.x()*v3.z() + v2.z()*v3.x());
            Pyz += V6 * (2*(v1.y()*v1.z() + v2.y()*v2.z() + v3.y()*v3.z()) + v1.y()*v2.z() + v1.z()*v2.y() + v1.y()*v3.z() + v1.z()*v3.y() + v2.y()*v3.z() + v2.z()*v3.y());
        }
    
        Pxx *= mat.density / Scalar(120); //20 from formula and 6 from polyhedron volume
        Pyy *= mat.density / Scalar(120);
        Pzz *= mat.density / Scalar(120);
        Pxy *= mat.density / Scalar(120);
        Pxz *= mat.density / Scalar(120);
        Pyz *= mat.density / Scalar(120);
    
        I = Matrix3(Pyy+Pzz, -Pxy, -Pxz, -Pxy, Pxx+Pzz, -Pyz, -Pxz, -Pyz, Pxx+Pyy);
    }
    
	//5. Find primary moments of inertia
	Ipri = Vector3(I.getRow(0).getX(), I.getRow(1).getY(), I.getRow(2).getZ());
	
	//Check if inertia matrix is not diagonal
	if(!(btFuzzyZero(I.getRow(0).getY()) && btFuzzyZero(I.getRow(0).getZ())
	     && btFuzzyZero(I.getRow(1).getX()) && btFuzzyZero(I.getRow(1).getZ())
	     && btFuzzyZero(I.getRow(2).getX()) && btFuzzyZero(I.getRow(2).getY())))
	{
		//5.1. Calculate principal moments of inertia
		Scalar T = I[0][0] + I[1][1] + I[2][2]; //Ixx + Iyy + Izz
		Scalar II = I[0][0]*I[1][1] + I[0][0]*I[2][2] + I[1][1]*I[2][2] - I[0][1]*I[0][1] - I[0][2]*I[0][2] - I[1][2]*I[1][2]; //Ixx Iyy + Ixx Izz + Iyy Izz - Ixy^2 - Ixz^2 - Iyz^2
		Scalar U = btSqrt(T*T-Scalar(3)*II)/Scalar(3);
		Scalar theta = btAcos((-Scalar(2)*T*T*T + Scalar(9)*T*II - Scalar(27)*I.determinant())/(Scalar(54)*U*U*U));
		Scalar A = T/Scalar(3) - Scalar(2)*U*btCos(theta/Scalar(3));
		Scalar B = T/Scalar(3) - Scalar(2)*U*btCos(theta/Scalar(3) - Scalar(2)*M_PI/Scalar(3));
		Scalar C = T/Scalar(3) - Scalar(2)*U*btCos(theta/Scalar(3) + Scalar(2)*M_PI/Scalar(3));
		Ipri = Vector3(A, B, C);
    
		//5.2. Calculate principal axes of inertia
		Matrix3 L;
		Vector3 axis1,axis2,axis3;
		axis1 = findInertiaAxis(I, A);
		axis2 = findInertiaAxis(I, B);
		axis3 = axis1.cross(axis2);
		axis2 = axis3.cross(axis1);
    
		//5.3. Rotate body so that principal axes are parallel to (x,y,z) system
		Matrix3 rotMat(axis1[0],axis2[0],axis3[0], axis1[1],axis2[1],axis3[1], axis1[2],axis2[2],axis3[2]);
		T_G2CG.setBasis(rotMat);
	}
 
    //6.Calculate AABB
    OpenGLContent::AABB(mesh, aabb[0], aabb[1]);
    mass = volume*mat.density;
	
	//7.Calculate equivalent ellipsoid for hydrodynamic force computation
	ComputeHydrodynamicProxy(geoProxy);
    
    //8. Set hydrodynamic properties
    CB = T_G2CG.getOrigin();
}

Polyhedron::Polyhedron(std::string uniqueName,
                       std::string geometryFilename, Scalar geometryScale, const Transform& geometryOrigin,
                       std::string collisionFilename, Scalar collisionScale, const Transform& collisionOrigin,
                       Material m, int lookId, bool smoothGeometryNormals, Scalar thickness,
                       bool isBuoyant, HydrodynamicProxyType geoProxy) : SolidEntity(uniqueName, m, lookId, thickness, isBuoyant)
{
    /*//1. Set transformations
    T_O2G = geometryOrigin;
    T_O2C = collisionOrigin;
    
    //1.Load triangle mesh from file
    mesh = OpenGLContent::LoadMesh(modelFilename, scale, smoothNormals);
    transformMesh(mesh, originTrans);
    
    //2.Calculate mesh volume and COG
    Vector3 meshCog(0,0,0);
    
    if(thick > Scalar(0))
    {
        Scalar meshVolume = 0;
        
        for(unsigned int i=0; i<mesh->faces.size(); ++i)
        {
            //triangle
            glm::vec3 v1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
            
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            
            Scalar A = (v2-v1).cross(v3-v1).length()/Scalar(2);
            Vector3 triCOG = (v1+v2+v3)/Scalar(3);
            Scalar triVolume = A * thick;
            meshCog += triCOG * triVolume;
            meshVolume += triVolume;
        }
        
        if(meshVolume > Scalar(0))
            meshCog /= meshVolume;
        else
            meshCog = Vector3(0,0,0);
        
        volume = meshVolume;
    }
    else
    {
        Scalar meshVolume = 0;
        
        for(unsigned int i=0; i<mesh->faces.size(); ++i)
        {
            //triangle
            glm::vec3 v1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
            
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            
            Vector3 tetraCOG = (v1+v2+v3)/Scalar(4);
            Scalar tetraVolume6 = v1.dot(v2.cross(v3));
            meshCog += tetraCOG * tetraVolume6;
            meshVolume += tetraVolume6;
        }
        
        if(meshVolume > 0.0)
            meshCog /= meshVolume;
        else
            meshCog = Vector3(0,0,0);
        
        volume = meshVolume * Scalar(1)/Scalar(6);
    }
    
    //3.Move vertices so that COG is in (0,0,0)
    T_G2CG.setOrigin(meshCog);
    
    //4.Calculate moments of inertia for local coordinate system located in COG (not necessarily principal)
    Matrix3 I;
    
    if(thick > Scalar(0))
    {
        //External
        Scalar Pxx = Scalar(0);
        Scalar Pyy = Scalar(0);
        Scalar Pzz = Scalar(0);
        Scalar Pxy = Scalar(0);
        Scalar Pxz = Scalar(0);
        Scalar Pyz = Scalar(0);
        
        for(unsigned int i=0; i<mesh->faces.size(); ++i)
        {
            //Triangle verticies with respect to COG
            glm::vec3 v1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
            
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            Vector3 n = (v2-v1).cross(v3-v1).normalize();
            v1 = v1 + n*thick/Scalar(2) - meshCog;
            v2 = v2 + n*thick/Scalar(2) - meshCog;
            v3 = v3 + n*thick/Scalar(2) - meshCog;
            
            //Pjk = const * dV * (2*Aj*Ak + 2*Bj*Bk + 2*Cj*Ck + Aj*Bk + Ak*Bj + Aj*Ck + Ak*Cj + Bj*Ck + Bk*Cj)
            Scalar V6 = v1.dot(v2.cross(v3));
            Pxx += V6 * 2 *(v1.x()*v1.x() + v2.x()*v2.x() + v3.x()*v3.x() + v1.x()*v2.x() + v1.x()*v3.x() + v2.x()*v3.x());
            Pyy += V6 * 2 *(v1.y()*v1.y() + v2.y()*v2.y() + v3.y()*v3.y() + v1.y()*v2.y() + v1.y()*v3.y() + v2.y()*v3.y());
            Pzz += V6 * 2 *(v1.z()*v1.z() + v2.z()*v2.z() + v3.z()*v3.z() + v1.z()*v2.z() + v1.z()*v3.z() + v2.z()*v3.z());
            Pxy += V6 * (2*(v1.x()*v1.y() + v2.x()*v2.y() + v3.x()*v3.y()) + v1.x()*v2.y() + v1.y()*v2.x() + v1.x()*v3.y() + v1.y()*v3.x() + v2.x()*v3.y() + v2.y()*v3.x());
            Pxz += V6 * (2*(v1.x()*v1.z() + v2.x()*v2.z() + v3.x()*v3.z()) + v1.x()*v2.z() + v1.z()*v2.x() + v1.x()*v3.z() + v1.z()*v3.x() + v2.x()*v3.z() + v2.z()*v3.x());
            Pyz += V6 * (2*(v1.y()*v1.z() + v2.y()*v2.z() + v3.y()*v3.z()) + v1.y()*v2.z() + v1.z()*v2.y() + v1.y()*v3.z() + v1.z()*v3.y() + v2.y()*v3.z() + v2.z()*v3.y());
        }
        
        Pxx *= mat.density / Scalar(120); //20 from formula and 6 from polyhedron volume
        Pyy *= mat.density / Scalar(120);
        Pzz *= mat.density / Scalar(120);
        Pxy *= mat.density / Scalar(120);
        Pxz *= mat.density / Scalar(120);
        Pyz *= mat.density / Scalar(120);
        
        I = Matrix3(Pyy+Pzz, -Pxy, -Pxz, -Pxy, Pxx+Pzz, -Pyz, -Pxz, -Pyz, Pxx+Pyy);
        
        //Internal
        Pxx = Scalar(0);
        Pyy = Scalar(0);
        Pzz = Scalar(0);
        Pxy = Scalar(0);
        Pxz = Scalar(0);
        Pyz = Scalar(0); //products of inertia
        
        for(unsigned int i=0; i<mesh->faces.size(); ++i)
        {
            //Triangle verticies with respect to COG
            glm::vec3 v1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
            
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            Vector3 n = (v2-v1).cross(v3-v1).normalize();
            v1 = v1 - n*thick/Scalar(2) - meshCog;
            v2 = v2 - n*thick/Scalar(2) - meshCog;
            v3 = v3 - n*thick/Scalar(2) - meshCog;
            
            //Pjk = const * dV * (2*Aj*Ak + 2*Bj*Bk + 2*Cj*Ck + Aj*Bk + Ak*Bj + Aj*Ck + Ak*Cj + Bj*Ck + Bk*Cj)
            Scalar V6 = v1.dot(v2.cross(v3));
            Pxx += V6 * 2 *(v1.x()*v1.x() + v2.x()*v2.x() + v3.x()*v3.x() + v1.x()*v2.x() + v1.x()*v3.x() + v2.x()*v3.x());
            Pyy += V6 * 2 *(v1.y()*v1.y() + v2.y()*v2.y() + v3.y()*v3.y() + v1.y()*v2.y() + v1.y()*v3.y() + v2.y()*v3.y());
            Pzz += V6 * 2 *(v1.z()*v1.z() + v2.z()*v2.z() + v3.z()*v3.z() + v1.z()*v2.z() + v1.z()*v3.z() + v2.z()*v3.z());
            Pxy += V6 * (2*(v1.x()*v1.y() + v2.x()*v2.y() + v3.x()*v3.y()) + v1.x()*v2.y() + v1.y()*v2.x() + v1.x()*v3.y() + v1.y()*v3.x() + v2.x()*v3.y() + v2.y()*v3.x());
            Pxz += V6 * (2*(v1.x()*v1.z() + v2.x()*v2.z() + v3.x()*v3.z()) + v1.x()*v2.z() + v1.z()*v2.x() + v1.x()*v3.z() + v1.z()*v3.x() + v2.x()*v3.z() + v2.z()*v3.x());
            Pyz += V6 * (2*(v1.y()*v1.z() + v2.y()*v2.z() + v3.y()*v3.z()) + v1.y()*v2.z() + v1.z()*v2.y() + v1.y()*v3.z() + v1.z()*v3.y() + v2.y()*v3.z() + v2.z()*v3.y());
        }
        
        Pxx *= mat.density / Scalar(120); //20 from formula and 6 from polyhedron volume
        Pyy *= mat.density / Scalar(120);
        Pzz *= mat.density / Scalar(120);
        Pxy *= mat.density / Scalar(120);
        Pxz *= mat.density / Scalar(120);
        Pyz *= mat.density / Scalar(120);
        
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
        
        for(unsigned int i=0; i<mesh->faces.size(); ++i)
        {
            //Triangle verticies with respect to COG
            glm::vec3 v1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
            
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            v1 -= meshCog;
            v2 -= meshCog;
            v3 -= meshCog;
            
            //Pjk = const * dV * (2*Aj*Ak + 2*Bj*Bk + 2*Cj*Ck + Aj*Bk + Ak*Bj + Aj*Ck + Ak*Cj + Bj*Ck + Bk*Cj)
            Scalar V6 = v1.dot(v2.cross(v3));
            Pxx += V6 * 2 *(v1.x()*v1.x() + v2.x()*v2.x() + v3.x()*v3.x() + v1.x()*v2.x() + v1.x()*v3.x() + v2.x()*v3.x());
            Pyy += V6 * 2 *(v1.y()*v1.y() + v2.y()*v2.y() + v3.y()*v3.y() + v1.y()*v2.y() + v1.y()*v3.y() + v2.y()*v3.y());
            Pzz += V6 * 2 *(v1.z()*v1.z() + v2.z()*v2.z() + v3.z()*v3.z() + v1.z()*v2.z() + v1.z()*v3.z() + v2.z()*v3.z());
            Pxy += V6 * (2*(v1.x()*v1.y() + v2.x()*v2.y() + v3.x()*v3.y()) + v1.x()*v2.y() + v1.y()*v2.x() + v1.x()*v3.y() + v1.y()*v3.x() + v2.x()*v3.y() + v2.y()*v3.x());
            Pxz += V6 * (2*(v1.x()*v1.z() + v2.x()*v2.z() + v3.x()*v3.z()) + v1.x()*v2.z() + v1.z()*v2.x() + v1.x()*v3.z() + v1.z()*v3.x() + v2.x()*v3.z() + v2.z()*v3.x());
            Pyz += V6 * (2*(v1.y()*v1.z() + v2.y()*v2.z() + v3.y()*v3.z()) + v1.y()*v2.z() + v1.z()*v2.y() + v1.y()*v3.z() + v1.z()*v3.y() + v2.y()*v3.z() + v2.z()*v3.y());
        }
        
        Pxx *= mat.density / Scalar(120); //20 from formula and 6 from polyhedron volume
        Pyy *= mat.density / Scalar(120);
        Pzz *= mat.density / Scalar(120);
        Pxy *= mat.density / Scalar(120);
        Pxz *= mat.density / Scalar(120);
        Pyz *= mat.density / Scalar(120);
        
        I = Matrix3(Pyy+Pzz, -Pxy, -Pxz, -Pxy, Pxx+Pzz, -Pyz, -Pxz, -Pyz, Pxx+Pyy);
    }
    
    //5. Find primary moments of inertia
    Ipri = Vector3(I.getRow(0).getX(), I.getRow(1).getY(), I.getRow(2).getZ());
    
    //Check if inertia matrix is not diagonal
    if(!(btFuzzyZero(I.getRow(0).getY()) && btFuzzyZero(I.getRow(0).getZ())
         && btFuzzyZero(I.getRow(1).getX()) && btFuzzyZero(I.getRow(1).getZ())
         && btFuzzyZero(I.getRow(2).getX()) && btFuzzyZero(I.getRow(2).getY())))
    {
        //5.1. Calculate principal moments of inertia
        Scalar T = I[0][0] + I[1][1] + I[2][2]; //Ixx + Iyy + Izz
        Scalar II = I[0][0]*I[1][1] + I[0][0]*I[2][2] + I[1][1]*I[2][2] - I[0][1]*I[0][1] - I[0][2]*I[0][2] - I[1][2]*I[1][2]; //Ixx Iyy + Ixx Izz + Iyy Izz - Ixy^2 - Ixz^2 - Iyz^2
        Scalar U = btSqrt(T*T-Scalar(3)*II)/Scalar(3);
        Scalar theta = btAcos((-Scalar(2)*T*T*T + Scalar(9)*T*II - Scalar(27)*I.determinant())/(Scalar(54)*U*U*U));
        Scalar A = T/Scalar(3) - Scalar(2)*U*btCos(theta/Scalar(3));
        Scalar B = T/Scalar(3) - Scalar(2)*U*btCos(theta/Scalar(3) - Scalar(2)*M_PI/Scalar(3));
        Scalar C = T/Scalar(3) - Scalar(2)*U*btCos(theta/Scalar(3) + Scalar(2)*M_PI/Scalar(3));
        Ipri = Vector3(A, B, C);
        
        //5.2. Calculate principal axes of inertia
        Matrix3 L;
        Vector3 axis1,axis2,axis3;
        axis1 = findInertiaAxis(I, A);
        axis2 = findInertiaAxis(I, B);
        axis3 = axis1.cross(axis2);
        axis2 = axis3.cross(axis1);
        
        //5.3. Rotate body so that principal axes are parallel to (x,y,z) system
        Matrix3 rotMat(axis1[0],axis2[0],axis3[0], axis1[1],axis2[1],axis3[1], axis1[2],axis2[2],axis3[2]);
        T_G2CG.setBasis(rotMat);
    }
    
    //6.Calculate AABB
    OpenGLContent::AABB(mesh, aabb[0], aabb[1]);
    mass = volume*mat.density;
    
    //7.Calculate equivalent ellipsoid for hydrodynamic force computation
    ComputeHydrodynamicProxy(geoProxy);
    
    //8. Set hydrodynamic properties
    CB = T_G2CG.getOrigin();*/
}

SolidType Polyhedron::getSolidType()
{
    return SOLID_POLYHEDRON;
}

btCollisionShape* Polyhedron::BuildCollisionShape()
{
    //Build GIMPACT concave shape
    /*int* indices = new int[mesh->faces.size()*3];
    for(int i=0; i<mesh->faces.size(); i++)
    {
        indices[i*3+0] = mesh->faces[i].vertexIndex[0];
        indices[i*3+1] = mesh->faces[i].vertexIndex[1];
        indices[i*3+2] = mesh->faces[i].vertexIndex[2];
    }
        
    triangleArray = new btTriangleIndexVertexArray(mesh->faces.size(), indices, 3*sizeof(int),
                                                       mesh->vertices.size(), (Scalar*)&mesh->vertices[0].x(), sizeof(Vector3));
        
    
    btGImpactMeshShape * trimesh = new btGImpactMeshShape(triangleArray);
    trimesh->updateBound();*/
        
    btConvexHullShape* convex = new btConvexHullShape();
    for(unsigned int i=0; i<mesh->vertices.size(); i++)
	{
		Vector3 v(mesh->vertices[i].pos.x, mesh->vertices[i].pos.y, mesh->vertices[i].pos.z);
        convex->addPoint(v);
	}
	
	convex->optimizeConvexHull();
    //convex->setMargin(0.0);//UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), Scalar(0.001)));
	return convex;
}
    
}
