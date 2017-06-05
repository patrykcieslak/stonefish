//
//  MeshEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "MeshEntity.h"
#include "OpenGLSolids.h"
#include "SystemUtil.h"
#include "GeometryFileUtil.h"

MeshEntity::MeshEntity(std::string uniqueName, const char* modelFilename, btScalar scale, Material* mat, Look l, bool smoothNormals) : SolidEntity(uniqueName, mat)
{
    scale = UnitSystem::SetLength(scale);
    
    //1.Load triangle mesh from file
    mesh = LoadModel(modelFilename, scale, smoothNormals);
    
    //2.Calculate mesh volume and COG
    btVector3 meshCog = btVector3(0,0,0);
    btScalar meshVolume = 0;
    
    for(int i=0; i<mesh->faces.size(); i++)
    {
        //triangle
        btVector3 v1 = mesh->vertices[mesh->faces[i].vertexIndex[0]];
        btVector3 v2 = mesh->vertices[mesh->faces[i].vertexIndex[1]];
        btVector3 v3 = mesh->vertices[mesh->faces[i].vertexIndex[2]];
        
        btVector3 tetraCOG = (v1+v2+v3)/4.0;
        btScalar tetraVolume6 = signedVolumeOfTetrahedron6(v1, v2, v3);
        meshCog += tetraCOG * tetraVolume6;
        meshVolume += tetraVolume6;
    }
    
    if(meshVolume > 0.0)
        meshCog /= meshVolume;
    else
        meshCog = btVector3(0,0,0);
    
    volume = meshVolume * 1.0/6.0;
    
    //3.Move vertices so that COG is in (0,0,0)
    localTransform.setOrigin(meshCog);
    
    //4.Calculate moments of inertia for local coordinate system located in COG (not necessarily principal)
    btScalar Pxx = 0.0, Pyy = 0.0, Pzz = 0.0, Pxy = 0.0, Pxz = 0.0, Pyz = 0.0; //products of inertia
    
    for(int i=0; i<mesh->faces.size(); i++)
    {
        //Triangle verticies with respect to COG
        btVector3 v1 = mesh->vertices[mesh->faces[i].vertexIndex[0]] - meshCog;
        btVector3 v2 = mesh->vertices[mesh->faces[i].vertexIndex[1]] - meshCog;
        btVector3 v3 = mesh->vertices[mesh->faces[i].vertexIndex[2]] - meshCog;
        
        //Pjk = const * dV * (2*Aj*Ak + 2*Bj*Bk + 2*Cj*Ck + Aj*Bk + Ak*Bj + Aj*Ck + Ak*Cj + Bj*Ck + Bk*Cj)
        btScalar V6 = signedVolumeOfTetrahedron6(v1, v2, v3);
        Pxx += V6 * 2 *(v1.x()*v1.x() + v2.x()*v2.x() + v3.x()*v3.x() + v1.x()*v2.x() + v1.x()*v3.x() + v2.x()*v3.x());
        Pyy += V6 * 2 *(v1.y()*v1.y() + v2.y()*v2.y() + v3.y()*v3.y() + v1.y()*v2.y() + v1.y()*v3.y() + v2.y()*v3.y());
        Pzz += V6 * 2 *(v1.z()*v1.z() + v2.z()*v2.z() + v3.z()*v3.z() + v1.z()*v2.z() + v1.z()*v3.z() + v2.z()*v3.z());
        Pxy += V6 * (2*(v1.x()*v1.y() + v2.x()*v2.y() + v3.x()*v3.y()) + v1.x()*v2.y() + v1.y()*v2.x() + v1.x()*v3.y() + v1.y()*v3.x() + v2.x()*v3.y() + v2.y()*v3.x());
        Pxz += V6 * (2*(v1.x()*v1.z() + v2.x()*v2.z() + v3.x()*v3.z()) + v1.x()*v2.z() + v1.z()*v2.x() + v1.x()*v3.z() + v1.z()*v3.x() + v2.x()*v3.z() + v2.z()*v3.x());
        Pyz += V6 * (2*(v1.y()*v1.z() + v2.y()*v2.z() + v3.y()*v3.z()) + v1.y()*v2.z() + v1.z()*v2.y() + v1.y()*v3.z() + v1.z()*v3.y() + v2.y()*v3.z() + v2.z()*v3.y());
    }
    
    Pxx *= material->density / 120.0; //20 from formula and 6 from polyhedron volume
    Pyy *= material->density / 120.0;
    Pzz *= material->density / 120.0;
    Pxy *= material->density / 120.0;
    Pxz *= material->density / 120.0;
    Pyz *= material->density / 120.0;
    
    btMatrix3x3 I = btMatrix3x3(Pyy+Pzz, -Pxy, -Pxz, -Pxy, Pxx+Pzz, -Pyz, -Pxz, -Pyz, Pxx+Pyy);
    
    //5.Calculate principal moments of inertia
    btScalar T = I[0][0] + I[1][1] + I[2][2]; //Ixx + Iyy + Izz
    btScalar II = I[0][0]*I[1][1] + I[0][0]*I[2][2] + I[1][1]*I[2][2] - I[0][1]*I[0][1] - I[0][2]*I[0][2] - I[1][2]*I[1][2]; //Ixx Iyy + Ixx Izz + Iyy Izz - Ixy^2 - Ixz^2 - Iyz^2
    btScalar U = btSqrt(T*T-btScalar(3.)*II)/btScalar(3.);
    btScalar theta = btAcos((-btScalar(2.)*T*T*T + btScalar(9.)*T*II - btScalar(27.)*I.determinant())/(btScalar(54.)*U*U*U));
    btScalar A = T/btScalar(3.) - btScalar(2.)*U*btCos(theta/btScalar(3.));
    btScalar B = T/btScalar(3.) - btScalar(2.)*U*btCos(theta/btScalar(3.) - btScalar(2.)*M_PI/btScalar(3.));
    btScalar C = T/btScalar(3.) - btScalar(2.)*U*btCos(theta/btScalar(3.) + btScalar(2.)*M_PI/btScalar(3.));
    Ipri = btVector3(A, B, C);
    
    //6. Calculate principal axes of inertia
    btMatrix3x3 L;
    btVector3 axis1,axis2,axis3;
    axis1 = findInertiaAxis(I, A);
    axis2 = findInertiaAxis(I, B);
    axis3 = axis1.cross(axis2);
    axis2 = axis3.cross(axis1);
    
    //6.Rotate body so that principal axes are parallel to (x,y,z) system
    btMatrix3x3 rotMat(axis1[0],axis2[0],axis3[0], axis1[1],axis2[1],axis3[1], axis1[2],axis2[2],axis3[2]);
    localTransform.setBasis(rotMat);
    
    //7.Calculate AABB
    AABB(mesh, aabb[0], aabb[1]);
    mass = volume*material->density;
    
    //8.Graphics
    SetLook(l);
}

MeshEntity::~MeshEntity(void)
{
    delete mesh;
}

SolidEntityType MeshEntity::getSolidType()
{
    return SOLID_MESH;
}

void MeshEntity::SetLook(Look newLook)
{
    SolidEntity::SetLook(newLook);
    BuildDisplayList();
}

void MeshEntity::BuildCollisionList()
{
    if(rigidBody != NULL)
    {
        if(collisionList != 0)
            glDeleteLists(collisionList, 1);
        
        btConvexHullShape* shape = (btConvexHullShape*)rigidBody->getCollisionShape();
        collisionList = glGenLists(1);
        glNewList(collisionList, GL_COMPILE);
        glBegin(GL_POINTS);
        for(int i=0; i<shape->getNumVertices(); i++)
        {
            btVector3 v1;
            shape->getVertex(i, v1);
            glVertex3f((GLfloat)v1.x(), (GLfloat)v1.y(), (GLfloat)v1.z());
        }
        glEnd();
        glEndList();
    }
}

btCollisionShape* MeshEntity::BuildCollisionShape()
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
        
    */
    /*btGImpactMeshShape * trimesh = new btGImpactMeshShape(triangleArray);
    trimesh->updateBound();*/
        
    btConvexHullShape* convex = new btConvexHullShape();
    for(int i=0; i<mesh->vertices.size(); i++)
        convex->addPoint(mesh->vertices[i]);
    
    return convex;
}

void MeshEntity::ComputeFluidForces(const FluidEntity* fluid, const btTransform& cogTransform, const btTransform& geometryTransform, const btVector3& v, const btVector3& omega, const btVector3& a, const btVector3& epsilon, btVector3& Fb, btVector3& Tb, btVector3& Fd, btVector3& Td, btVector3& Fa, btVector3& Ta, bool damping, bool addedMass)
{
    //Set zeros
    SolidEntity::ComputeFluidForces(fluid, cogTransform, geometryTransform, v, omega, a, epsilon, Fb, Tb, Fd, Td, Fa, Ta, damping, addedMass);
    btVector3 p = cogTransform.getOrigin();
    
    //Calculate fluid dynamics forces and torques
    //Loop through all faces...
    for(int i=0; i<mesh->faces.size(); ++i)
    {
        //Global coordinates
        btVector3 p1 = geometryTransform * mesh->vertices[mesh->faces[i].vertexIndex[0]];
        btVector3 p2 = geometryTransform * mesh->vertices[mesh->faces[i].vertexIndex[1]];
        btVector3 p3 = geometryTransform * mesh->vertices[mesh->faces[i].vertexIndex[2]];
        
        //Check if underwater
        btScalar pressure = (fluid->GetPressure(p1) + fluid->GetPressure(p2) + fluid->GetPressure(p3))/btScalar(3);
        if(pressure <= btScalar(0.))
            continue;
        
        //Calculate face features
        btVector3 fv1 = p2-p1; //One side of the face (triangle)
        btVector3 fv2 = p3-p1; //Another side of the face (triangle)
        btVector3 fc = (p1+p2+p3)/btScalar(3); //Face centroid
        btVector3 fn = fv1.cross(fv2); //Normal of the face (length != 1)
        btScalar len = fn.length();
        btVector3 fn1 = fn/len; //Normalised normal (length = 1)
        btScalar A = len/btScalar(2); //Area of the face (triangle)
        
        //Buoyancy force
        btVector3 Fbi = -fn/btScalar(2)*pressure; //Buoyancy force per face (based on pressure)
        
        //Accumulate
        Fb += Fbi;
        Tb += (fc - p).cross(Fbi);
        
        //Damping force
        if(damping)
        {
            //Skin drag force
            btVector3 vc = fluid->GetFluidVelocity(fc) - (v + omega.cross(fc - p)); //Water velocity at face center
            btVector3 vt = vc - (vc.dot(fn)*fn)/fn.length2(); //Water velocity projected on face (tangent to face)
            btVector3 Fds = fluid->getFluid()->viscosity * vt * A / btScalar(0.0001);
            //btVector3 Fds = vt.safeNormalize()*btScalar(0.5)*fluid->getFluid()->density*btScalar(1.328)/1000.0*vt.length2()*fn.length()/btScalar(2);
        
            //Pressure drag force
            btVector3 vn = vc - vt; //Water velocity normal to face
            btVector3 Fdp(0,0,0);
            if(fn.dot(vn) < btScalar(0))
                Fdp = btScalar(0.5)*fluid->getFluid()->density * vn * vn.length() * A;
            
            //Accumulate
            Fd += Fds + Fdp;
            Td += (fc - p).cross(Fds + Fdp);
        }
        
        //Added mass effect
        if(addedMass)
        {
            btVector3 ac = -(a + epsilon.cross(fc - p)); //Water acceleration at face center (velocity of fluid is constant)
            btVector3 Fai(0,0,0);
            btScalar an; //acceleration normal to face
            
            if((an = fn1.dot(ac)) < btScalar(0))
            {
                btScalar d = btScalar(1.)/(-an + btScalar(1.)); //Positive thickness of affected layer of fluid
                Fai = fluid->getFluid()->density * A * d * an * fn1; //Fa = rho V a = rho A d a
            }
            
            //Accumulate
            Fa += Fai;
            Ta += (fc - p).cross(Fai);
        }
    }
    
    //printf("Fluid: %1.2f, %1.2f, %1.2f\n", fluidForce.x(), fluidForce.y(), fluidForce.z());
    //printf("Fluid Torque: %1.2f, %1.2f, %1.10f\n", fluidTorque.x(), fluidTorque.y(), fluidTorque.z());
    //printf("Fluid: %1.2f, %1.2f, %1.2f\n", testt.x(), testt.y(), testt.z());
    //printf("Body acc: %1.2f, %1.2f, %1.2f\n", epsilon.x(), epsilon.y(), epsilon.z());

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

