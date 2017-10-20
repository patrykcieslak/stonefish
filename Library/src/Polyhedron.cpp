//
//  Polyhedron.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "Polyhedron.h"
#include "SystemUtil.hpp"

Polyhedron::Polyhedron(std::string uniqueName, std::string modelFilename, btScalar scale, Material m, int lookId, bool smoothNormals) : SolidEntity(uniqueName, m, lookId)
{
    scale = UnitSystem::SetLength(scale);
    
    //1.Load triangle mesh from file
    mesh = OpenGLContent::LoadMesh(modelFilename, scale, smoothNormals);
    
    //2.Calculate mesh volume and COG
    btVector3 meshCog = btVector3(0,0,0);
    btScalar meshVolume = 0;
    
    for(int i=0; i<mesh->faces.size(); i++)
    {
        //triangle
		glm::vec3 v1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
		glm::vec3 v2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
		glm::vec3 v3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
		
        btVector3 v1(v1gl.x,v1gl.y,v1gl.z);
        btVector3 v2(v2gl.x,v2gl.y,v2gl.z);
        btVector3 v3(v3gl.x,v3gl.y,v3gl.z);
        
        btVector3 tetraCOG = (v1+v2+v3)/4.0;
        btScalar tetraVolume6 = v1.dot(v2.cross(v3));
        meshCog += tetraCOG * tetraVolume6;
        meshVolume += tetraVolume6;
    }
    
    if(meshVolume > 0.0)
        meshCog /= meshVolume;
    else
        meshCog = btVector3(0,0,0);
    
    volume = meshVolume * btScalar(1)/btScalar(6);
    
    //3.Move vertices so that COG is in (0,0,0)
    localTransform.setOrigin(meshCog);
    
    //4.Calculate moments of inertia for local coordinate system located in COG (not necessarily principal)
    btScalar Pxx = 0.0, Pyy = 0.0, Pzz = 0.0, Pxy = 0.0, Pxz = 0.0, Pyz = 0.0; //products of inertia
    
    for(int i=0; i<mesh->faces.size(); i++)
    {
        //Triangle verticies with respect to COG
		glm::vec3 v1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
		glm::vec3 v2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
		glm::vec3 v3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
		
        btVector3 v1(v1gl.x,v1gl.y,v1gl.z);
        btVector3 v2(v2gl.x,v2gl.y,v2gl.z);
        btVector3 v3(v3gl.x,v3gl.y,v3gl.z);
		v1 -= meshCog;
        v2 -= meshCog;
        v3 -= meshCog;
        
        //Pjk = const * dV * (2*Aj*Ak + 2*Bj*Bk + 2*Cj*Ck + Aj*Bk + Ak*Bj + Aj*Ck + Ak*Cj + Bj*Ck + Bk*Cj)
        btScalar V6 = v1.dot(v2.cross(v3));
        Pxx += V6 * 2 *(v1.x()*v1.x() + v2.x()*v2.x() + v3.x()*v3.x() + v1.x()*v2.x() + v1.x()*v3.x() + v2.x()*v3.x());
        Pyy += V6 * 2 *(v1.y()*v1.y() + v2.y()*v2.y() + v3.y()*v3.y() + v1.y()*v2.y() + v1.y()*v3.y() + v2.y()*v3.y());
        Pzz += V6 * 2 *(v1.z()*v1.z() + v2.z()*v2.z() + v3.z()*v3.z() + v1.z()*v2.z() + v1.z()*v3.z() + v2.z()*v3.z());
        Pxy += V6 * (2*(v1.x()*v1.y() + v2.x()*v2.y() + v3.x()*v3.y()) + v1.x()*v2.y() + v1.y()*v2.x() + v1.x()*v3.y() + v1.y()*v3.x() + v2.x()*v3.y() + v2.y()*v3.x());
        Pxz += V6 * (2*(v1.x()*v1.z() + v2.x()*v2.z() + v3.x()*v3.z()) + v1.x()*v2.z() + v1.z()*v2.x() + v1.x()*v3.z() + v1.z()*v3.x() + v2.x()*v3.z() + v2.z()*v3.x());
        Pyz += V6 * (2*(v1.y()*v1.z() + v2.y()*v2.z() + v3.y()*v3.z()) + v1.y()*v2.z() + v1.z()*v2.y() + v1.y()*v3.z() + v1.z()*v3.y() + v2.y()*v3.z() + v2.z()*v3.y());
    }
    
    Pxx *= mat.density / btScalar(120); //20 from formula and 6 from polyhedron volume
    Pyy *= mat.density / btScalar(120);
    Pzz *= mat.density / btScalar(120);
    Pxy *= mat.density / btScalar(120);
    Pxz *= mat.density / btScalar(120);
    Pyz *= mat.density / btScalar(120);
    
    btMatrix3x3 I = btMatrix3x3(Pyy+Pzz, -Pxy, -Pxz, -Pxy, Pxx+Pzz, -Pyz, -Pxz, -Pyz, Pxx+Pyy);
    
	//5. Find primary moments of inertia
	Ipri = btVector3(I.getRow(0).getX(), I.getRow(1).getY(), I.getRow(2).getZ());
	
	//Check if inertia matrix is not diagonal
	if(!(btFuzzyZero(I.getRow(0).getY()) && btFuzzyZero(I.getRow(0).getZ())
	     && btFuzzyZero(I.getRow(1).getX()) && btFuzzyZero(I.getRow(1).getZ())
	     && btFuzzyZero(I.getRow(2).getX()) && btFuzzyZero(I.getRow(2).getY())))
	{
		//5.1. Calculate principal moments of inertia
		btScalar T = I[0][0] + I[1][1] + I[2][2]; //Ixx + Iyy + Izz
		btScalar II = I[0][0]*I[1][1] + I[0][0]*I[2][2] + I[1][1]*I[2][2] - I[0][1]*I[0][1] - I[0][2]*I[0][2] - I[1][2]*I[1][2]; //Ixx Iyy + Ixx Izz + Iyy Izz - Ixy^2 - Ixz^2 - Iyz^2
		btScalar U = btSqrt(T*T-btScalar(3)*II)/btScalar(3);
		btScalar theta = btAcos((-btScalar(2)*T*T*T + btScalar(9)*T*II - btScalar(27)*I.determinant())/(btScalar(54)*U*U*U));
		btScalar A = T/btScalar(3) - btScalar(2)*U*btCos(theta/btScalar(3));
		btScalar B = T/btScalar(3) - btScalar(2)*U*btCos(theta/btScalar(3) - btScalar(2)*M_PI/btScalar(3));
		btScalar C = T/btScalar(3) - btScalar(2)*U*btCos(theta/btScalar(3) + btScalar(2)*M_PI/btScalar(3));
		Ipri = btVector3(A, B, C);
    
		//5.2. Calculate principal axes of inertia
		btMatrix3x3 L;
		btVector3 axis1,axis2,axis3;
		axis1 = findInertiaAxis(I, A);
		axis2 = findInertiaAxis(I, B);
		axis3 = axis1.cross(axis2);
		axis2 = axis3.cross(axis1);
    
		//5.3. Rotate body so that principal axes are parallel to (x,y,z) system
		btMatrix3x3 rotMat(axis1[0],axis2[0],axis3[0], axis1[1],axis2[1],axis3[1], axis1[2],axis2[2],axis3[2]);
		localTransform.setBasis(rotMat);
	}
 
    //6.Calculate AABB
    OpenGLContent::AABB(mesh, aabb[0], aabb[1]);
    mass = volume*mat.density;
	
	//7.Calculate equivalent ellipsoid for hydrodynamic force computation
	ComputeEquivEllipsoid();
}

Polyhedron::~Polyhedron(void)
{
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
                                                       mesh->vertices.size(), (btScalar*)&mesh->vertices[0].x(), sizeof(btVector3));
        
    
    btGImpactMeshShape * trimesh = new btGImpactMeshShape(triangleArray);
    trimesh->updateBound();*/
        
    btConvexHullShape* convex = new btConvexHullShape();
    for(int i=0; i<mesh->vertices.size(); i++)
	{
		btVector3 v(mesh->vertices[i].pos.x, mesh->vertices[i].pos.y, mesh->vertices[i].pos.z);
        convex->addPoint(v);
	}
	
	convex->optimizeConvexHull();
    convex->setMargin(UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), btScalar(0.001)));
	return convex;
}

////////// OLD ROUTINES /////////


/*void calculateDrag(const btVector3& v1, const btVector3& v2, const btVector3& v3, const Fluid* fluid, const btVector3& fluidV, const btVector3& angularV, btVector3& dragForce, btVector3& dragTorque)
 {
 btVector3 fv1 = v2-v1;
 btVector3 fv2 = v3-v1;
 btScalar fa = fv1.cross(fv2).length(); //area of rectangle (length of normal)
 btVector3 fn = (fv1.cross(fv2))/fa; //normal
 btVector3 fc = ((v1+v2+v3)/3.0); //face center
 btVector3 localV = fluidV - angularV.cross(fc);
 btScalar v = localV.length(); //magnitude
 localV /= v;                  //normalized
 
 btScalar nDotV = fn.dot(localV);
 if(nDotV < 0)
 {
 nDotV = fabs(nDotV);
 btVector3 areaForce = 0.5*fluid->density*0.5*(nDotV*0.5*fa)*localV*v*v; // area drag Fd = 0.5*rho*Cd*A*V^2
 btVector3 skinForce = btVector3(0,0,0);//skin drag Fd = 0.5*rho*Cf*A*V^2;
 btVector3 totalForce = areaForce+skinForce;
 dragForce += totalForce;
 dragTorque += fc.cross(totalForce); // Td = r x Fd
 }
 }
 
 void MeshEntity::CalculateFluidDynamics(const btVector3& surfaceN, const btVector3&surfaceD, const btVector3&fluidV, const Fluid* fluid,
 btScalar& submergedVolume, btVector3& cob,  btVector3& drag, btVector3& angularDrag,
 btTransform* worldTransform, const btVector3& velocity,
 const btVector3& angularVelocity)
 {
 fullyImmersed = false;
 
 //0.Initialize
 double fluidVolume = 0;
 cob = btVector3(0,0,0);
 submergedVolume = 0;
 drag = btVector3(0,0,0);
 angularDrag = btVector3(0,0,0);
 
 btTransform solidTrans;
 btVector3 linVelocity;
 btVector3 angVelocity;
 
 if(worldTransform != NULL)
 {
 solidTrans = btTransform(*worldTransform);
 linVelocity = velocity;
 angVelocity = angularVelocity;
 }
 else if(rigidBody != NULL)
 {
 solidTrans = rigidBody->getWorldTransform();
 linVelocity = rigidBody->getLinearVelocity();
 angVelocity = rigidBody->getAngularVelocity();
 }
 else
 return;
 
 //1.Transform plane into rigid body local coordinate system
 btVector3 localSurfaceN = solidTrans.getBasis().inverse() * surfaceN;
 btVector3 localSurfaceD = solidTrans.inverse() * surfaceD;
 btVector3 localFluidV = solidTrans.getBasis().inverse() * (fluidV - linVelocity);
 btVector3 localAngularV = solidTrans.getBasis().inverse() * angVelocity;
 
 //2.Get rigid body BBox and check if surface crosses it
 btVector3 v1 = aabb[0] - localSurfaceD;
 btVector3 v2 = btVector3(aabb[1].x(), aabb[0].y(), aabb[0].z()) - localSurfaceD;
 btVector3 v3 = btVector3(aabb[0].x(), aabb[1].y(), aabb[0].z()) - localSurfaceD;
 btVector3 v4 = btVector3(aabb[0].x(), aabb[0].y(), aabb[1].z()) - localSurfaceD;
 btVector3 v5 = btVector3(aabb[1].x(), aabb[1].y(), aabb[0].z()) - localSurfaceD;
 btVector3 v6 = btVector3(aabb[0].x(), aabb[1].y(), aabb[1].z()) - localSurfaceD;
 btVector3 v7 = btVector3(aabb[1].x(), aabb[0].y(), aabb[1].z()) - localSurfaceD;
 btVector3 v8 = aabb[1] - localSurfaceD;
 
 btScalar d1 = distanceFromCenteredPlane(localSurfaceN, v1);
 btScalar d2 = distanceFromCenteredPlane(localSurfaceN, v2);
 btScalar d3 = distanceFromCenteredPlane(localSurfaceN, v3);
 btScalar d4 = distanceFromCenteredPlane(localSurfaceN, v4);
 btScalar d5 = distanceFromCenteredPlane(localSurfaceN, v5);
 btScalar d6 = distanceFromCenteredPlane(localSurfaceN, v6);
 btScalar d7 = distanceFromCenteredPlane(localSurfaceN, v7);
 btScalar d8 = distanceFromCenteredPlane(localSurfaceN, v8);
 
 //3A.Calculate submerged volume and COB - surface crossed by bounding box
 if(d1*d2 < 0 || d1*d4 < 0 || d1*d3 < 0 || d2*d7 < 0 || d2*d5 < 0 || d3*d5 < 0 || d3*d6 < 0 || d4*d7 < 0 || d4*d6 < 0 || d5*d8 < 0 || d6*d8 < 0 || d7*d8 < 0)
 {
 for(int i=0; i<mesh->faces.size(); i++)
 {
 //a.Get triangle vertices
 btVector3 v1 = mesh->vertices[mesh->faces[i].vertexIndex[0]];
 btVector3 v2 = mesh->vertices[mesh->faces[i].vertexIndex[1]];
 btVector3 v3 = mesh->vertices[mesh->faces[i].vertexIndex[2]];
 
 //b.Transform to surface point coordinate system
 v1 -= localSurfaceD;
 v2 -= localSurfaceD;
 v3 -= localSurfaceD;
 
 //c.Calculate distances from surface
 btScalar d1 = distanceFromCenteredPlane(localSurfaceN, v1);
 btScalar d2 = distanceFromCenteredPlane(localSurfaceN, v2);
 btScalar d3 = distanceFromCenteredPlane(localSurfaceN, v3);
 
 //d.Check if intersection occurs
 if(d1*d2 < 0 || d2*d3 < 0) //intersection (|| d3*d1 < 0)
 {
 //e.Calculate intersected volume
 btVector3 i1, i2;
 btScalar fracVolume = 0.0;
 btVector3 fracCOG;
 
 if(d1*d2 < 0)
 {
 i1 = v1+(d1/(d1-d2))*(v2-v1);
 
 if(d2*d3 < 0) //edges 1-2 and 2-3 intersected
 {
 i2 = v2+(d2/(d2-d3))*(v3-v2);
 
 if(d2 < 0)
 {
 fracVolume = signedVolumeOfTetrahedron6(v2, i2, i1);
 fracCOG = (v2+i2+i1)/4.0 * fracVolume;
 
 calculateDrag(v2+localSurfaceD, i2+localSurfaceD, i1+localSurfaceD, fluid, localFluidV, localAngularV, drag, angularDrag);
 }
 else
 {
 fracVolume = signedVolumeOfTetrahedron6(v1, i1, i2);
 btScalar tmp = signedVolumeOfTetrahedron6(v1, i2, v3);
 fracCOG = (v1+i1+i2)/4.0 * fracVolume + (v1+i2+v3)/4.0 * tmp;
 fracVolume += tmp;
 
 calculateDrag(v1+localSurfaceD, i1+localSurfaceD, i2+localSurfaceD, fluid, localFluidV, localAngularV, drag, angularDrag);
 calculateDrag(v1+localSurfaceD, i2+localSurfaceD, v3+localSurfaceD, fluid, localFluidV, localAngularV, drag, angularDrag);
 }
 }
 else //edges 1-2 and 3-1 intersected
 {
 i2 = v3+(d3/(d3-d1))*(v1-v3);
 
 if(d1 < 0)
 {
 fracVolume = signedVolumeOfTetrahedron6(v1, i1, i2);
 fracCOG = (v1+i1+i2)/4.0 * fracVolume;
 
 calculateDrag(v1+localSurfaceD, i1+localSurfaceD, i2+localSurfaceD, fluid, localFluidV, localAngularV, drag, angularDrag);
 }
 else
 {
 fracVolume = signedVolumeOfTetrahedron6(v3, i2, i1);
 btScalar tmp = signedVolumeOfTetrahedron6(v3, i1, v2);
 fracCOG = (v3+i2+i1)/4.0 * fracVolume + (v3+i1+v2)/4.0 * tmp;
 fracVolume += tmp;
 
 calculateDrag(v3+localSurfaceD, i2+localSurfaceD, i1+localSurfaceD, fluid, localFluidV, localAngularV, drag, angularDrag);
 calculateDrag(v3+localSurfaceD, i1+localSurfaceD, v2+localSurfaceD, fluid, localFluidV, localAngularV, drag, angularDrag);
 }
 }
 }
 else //edges 2-3 and 3-1 intersected
 {
 i1 = v2+(d2/(d2-d3))*(v3-v2);
 i2 = v3+(d3/(d3-d1))*(v1-v3);
 
 if(d3 < 0)
 {
 fracVolume = signedVolumeOfTetrahedron6(v3, i2, i1);
 fracCOG = (v3+i2+i1)/4.0 * fracVolume;
 
 calculateDrag(v3+localSurfaceD, i2+localSurfaceD, i1+localSurfaceD, fluid, localFluidV, localAngularV, drag, angularDrag);
 }
 else
 {
 fracVolume = signedVolumeOfTetrahedron6(v1, i1, i2);
 btScalar tmp = signedVolumeOfTetrahedron6(v1, v2, i1);
 fracCOG = (v1+i1+i2)/4.0 * fracVolume + (v1+v2+i1)/4.0 * tmp;
 fracVolume += tmp;
 
 calculateDrag(v1+localSurfaceD, i1+localSurfaceD, i2+localSurfaceD, fluid, localFluidV, localAngularV, drag, angularDrag);
 calculateDrag(v1+localSurfaceD, v2+localSurfaceD, i1+localSurfaceD, fluid, localFluidV, localAngularV, drag, angularDrag);
 }
 }
 
 cob += fracCOG;
 fluidVolume += fracVolume;
 }
 else
 {
 //f.Calculate normal volume (full immersion)
 if(d1 < 0)
 {
 //buoyancy
 btVector3 tetraCOG = (v1+v2+v3)/4.0;
 btScalar tetraVolume = signedVolumeOfTetrahedron6(v1, v2, v3);
 cob += tetraCOG * tetraVolume;
 fluidVolume += tetraVolume;
 
 calculateDrag(v1+localSurfaceD, v2+localSurfaceD, v3+localSurfaceD, fluid, localFluidV, localAngularV, drag, angularDrag);
 }
 }
 }
 
 if(fluidVolume > 0)
 {
 cob /= fluidVolume;
 cob += localSurfaceD;
 fluidVolume *= 1.0/6.0;
 }
 else
 {
 cob = btVector3(0,0,0);
 fluidVolume = 0.0;
 }
 }
 //3B.Check if fully immersed in fluid
 else if(d1 < 0 || d2 < 0 || d3 < 0 || d4 < 0 || d5 < 0 || d6 < 0 || d7 < 0 || d8 < 0)
 {
 fullyImmersed = true;
 
 cob = btVector3(0,0,0);
 fluidVolume = volume;
 
 for(int i=0; i<mesh->faces.size(); i++)
 {
 btVector3 v1 = mesh->vertices[mesh->faces[i].vertexIndex[0]];
 btVector3 v2 = mesh->vertices[mesh->faces[i].vertexIndex[1]];
 btVector3 v3 = mesh->vertices[mesh->faces[i].vertexIndex[2]];
 
 calculateDrag(v1, v2, v3, fluid, localFluidV, localAngularV, drag, angularDrag);
 }
 }
 
 submergedVolume = fluidVolume;
 drag = solidTrans.getBasis()*drag;
 angularDrag = solidTrans.getBasis()*angularDrag;
 cob = solidTrans.getBasis() * cob;
 centerOfBuoyancy = cob;
 }*/

