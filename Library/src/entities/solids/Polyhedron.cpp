//
//  Polyhedron.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright (c) 2012-2018 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Polyhedron.h"

#include "graphics/OpenGLContent.h"
#include "utils/SystemUtil.hpp"
#include "utils/MathUtil.hpp"

namespace sf
{

Polyhedron::Polyhedron(std::string uniqueName,
                       std::string graphicsFilename, Scalar graphicsScale, const Transform& graphicsOrigin,
                       std::string physicsFilename, Scalar physicsScale, const Transform& physicsOrigin,
                       Material m, int lookId, bool smoothGraphicsNormals, Scalar thickness,
                       bool isBuoyant, HydrodynamicProxyType proxy) : SolidEntity(uniqueName, m, lookId, thickness, isBuoyant)
{
    //1.Load geometry from file
    graMesh = OpenGLContent::LoadMesh(graphicsFilename, graphicsScale, smoothGraphicsNormals);
    T_O2G = graphicsOrigin;
    
    if(physicsFilename != "")
    {
        phyMesh = OpenGLContent::LoadMesh(physicsFilename, physicsScale, false);
        T_O2C = physicsOrigin;
    }
    else
    {
        phyMesh = graMesh;
        T_O2C = T_O2G;
    }
        
    //2.Calculate mesh volume and CG
    Vector3 meshCG(0,0,0);
    
    if(thick > Scalar(0)) //Shell
    {
        Scalar meshVolume(0);
        
        for(size_t i=0; i<phyMesh->faces.size(); ++i)
        {
            //Get triangle, convert from OpenGL to physics
            glm::vec3 v1gl = phyMesh->vertices[phyMesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = phyMesh->vertices[phyMesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = phyMesh->vertices[phyMesh->faces[i].vertexID[2]].pos;
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            
            //Calculate volume of shell triangle
            Scalar A = (v2-v1).cross(v3-v1).length()/Scalar(2);
            Vector3 triCG = (v1+v2+v3)/Scalar(3);
            Scalar triVolume = A * thick;
            meshCG += triCG * triVolume;
            meshVolume += triVolume;
        }
        
        //Compute mesh CG
        if(meshVolume > Scalar(0))
            meshCG /= meshVolume;
        else
            meshCG = Vector3(0,0,0);
        
        //Assign mesh volume
        volume = meshVolume;
    }
    else //Solid body
    {
        Scalar meshVolume(0);
        
        for(size_t i=0; i<phyMesh->faces.size(); ++i)
        {
            //Get triangle, convert from OpenGL to physics
            glm::vec3 v1gl = phyMesh->vertices[phyMesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = phyMesh->vertices[phyMesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = phyMesh->vertices[phyMesh->faces[i].vertexID[2]].pos;
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            
            //Calculate signed volume of a tetrahedra
            Vector3 tetraCG = (v1+v2+v3)/Scalar(4);
            Scalar tetraVolume6 = v1.dot(v2.cross(v3));
            meshCG += tetraCG * tetraVolume6;
            meshVolume += tetraVolume6;
        }
        
        //Compute mesh CG
        if(meshVolume > Scalar(0))
            meshCG /= meshVolume;
        else
            meshCG = Vector3(0,0,0);
        
        //Compute mesh volume
        volume = meshVolume * Scalar(1)/Scalar(6);
    }
    
    //3.Move geometry so that CG is in (0,0,0)
    T_CG2C.setOrigin(-meshCG); //<------ SET ORIGIN OF G TO CG TRANSFORMATION
    
    //4.Calculate moments of inertia for local coordinate system located in CG (not necessarily principal)
    Matrix3 I;
    
    if(thick > Scalar(0)) //Shell - I have doubts if it is correct!
    {
        //Compute properties a shell by subtracting the inner solid from the outer solid
        //Outer solid -> eternal surface
        Scalar Pxx = Scalar(0);
        Scalar Pyy = Scalar(0);
        Scalar Pzz = Scalar(0);
        Scalar Pxy = Scalar(0);
        Scalar Pxz = Scalar(0);
        Scalar Pyz = Scalar(0);
        
        for(size_t i=0; i<phyMesh->faces.size(); ++i)
        {
            //Triangle verticies with respect to CG
            glm::vec3 v1gl = phyMesh->vertices[phyMesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = phyMesh->vertices[phyMesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = phyMesh->vertices[phyMesh->faces[i].vertexID[2]].pos;
            
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            Vector3 n = (v2-v1).cross(v3-v1).normalize();
            v1 = v1 + n*thick/Scalar(2) - meshCG;
            v2 = v2 + n*thick/Scalar(2) - meshCG;
            v3 = v3 + n*thick/Scalar(2) - meshCG;
            
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
        
        //Inner solid -> internal surface
        Pxx = Scalar(0);
        Pyy = Scalar(0);
        Pzz = Scalar(0);
        Pxy = Scalar(0);
        Pxz = Scalar(0);
        Pyz = Scalar(0); //products of inertia
        
        for(unsigned int i=0; i<phyMesh->faces.size(); ++i)
        {
            //Triangle verticies with respect to CG
            glm::vec3 v1gl = phyMesh->vertices[phyMesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = phyMesh->vertices[phyMesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = phyMesh->vertices[phyMesh->faces[i].vertexID[2]].pos;
            
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            Vector3 n = (v2-v1).cross(v3-v1).normalize();
            v1 = v1 - n*thick/Scalar(2) - meshCG;
            v2 = v2 - n*thick/Scalar(2) - meshCG;
            v3 = v3 - n*thick/Scalar(2) - meshCG;
            
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
        
        for(size_t i=0; i<phyMesh->faces.size(); ++i)
        {
            //Triangle verticies with respect to CG
            glm::vec3 v1gl = phyMesh->vertices[phyMesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = phyMesh->vertices[phyMesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = phyMesh->vertices[phyMesh->faces[i].vertexID[2]].pos;
            
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            v1 -= meshCG;
            v2 -= meshCG;
            v3 -= meshCG;
            
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
        T_CG2C.setBasis(rotMat.inverse()); //<------ SET BASIS OF G TO CG TRANSFORMATION
    }
    
    //6.Calculate AABB
    OpenGLContent::AABB(phyMesh, aabb[0], aabb[1]);
    mass = volume*mat.density;
    
    //7.Calculate equivalent ellipsoid for hydrodynamic force computation
    ComputeHydrodynamicProxy(proxy);
    
    //8. Compute missing transformations
    T_CG2O = T_CG2C * T_O2C.inverse();
    T_CG2G = T_CG2O * T_O2G;
    P_CB = Vector3(0,0,0);
}
    
Polyhedron::Polyhedron(std::string uniqueName,
                       std::string modelFilename, Scalar scale, const Transform& origin,
                       Material m, int lookId, bool smoothNormals, Scalar thickness, bool isBuoyant, HydrodynamicProxyType proxy)
    : Polyhedron(uniqueName, modelFilename, scale, origin, "", scale, origin, m, lookId, smoothNormals, thickness, isBuoyant, proxy)
{
}

Polyhedron::~Polyhedron()
{
    if(graMesh != NULL)
    {
        if(graMesh == phyMesh) phyMesh = NULL;
        delete graMesh;
    }
}
    
SolidType Polyhedron::getSolidType()
{
    return SOLID_POLYHEDRON;
}

btCollisionShape* Polyhedron::BuildCollisionShape()
{
    btConvexHullShape* convex = new btConvexHullShape();
    for(size_t i=0; i<phyMesh->vertices.size(); ++i)
	{
		Vector3 v(phyMesh->vertices[i].pos.x, phyMesh->vertices[i].pos.y, phyMesh->vertices[i].pos.z);
        convex->addPoint(v);
	}
	convex->optimizeConvexHull();
    return convex;
}

void Polyhedron::BuildGraphicalObject()
{
    if(graMesh == NULL || !SimulationApp::getApp()->hasGraphics())
        return;
    
    graObjectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(graMesh);
}

}
