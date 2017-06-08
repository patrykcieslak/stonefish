//
//  UnderwaterVehicle.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 17/05/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterVehicle.h"

UnderwaterVehicle::UnderwaterVehicle(std::string uniqueName) : SystemEntity(uniqueName)
{
    vehicleBody = NULL;
    internalList = 0;
    externalList = 0;
    showInternals = false;
    linearAcc = btVector3(0,0,0);
    angularAcc = btVector3(0,0,0);
}

UnderwaterVehicle::~UnderwaterVehicle()
{
    //Destroy subsystems......
}

btTransform UnderwaterVehicle::getTransform() const
{
    if(vehicleBody != NULL)
    {
        btTransform trans;
        vehicleBody->getMotionState()->getWorldTransform(trans);
        return trans;
    }
    else
        return btTransform::getIdentity();
}

void UnderwaterVehicle::AddInternalPart(SolidEntity* solid, const btTransform& position)
{
    if(solid != NULL)
    {
        VehiclePart part;
        part.solid = solid;
        part.position = position;
        part.isExternal = false;
        bodyParts.push_back(part);
    }
}

void UnderwaterVehicle::AddExternalPart(SolidEntity* solid, const btTransform& position)
{
    if(solid != NULL)
    {
        VehiclePart part;
        part.solid = solid;
        part.position = position;
        part.isExternal = true;
        bodyParts.push_back(part);
    }
}

void UnderwaterVehicle::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform)
{
    if(vehicleBody != NULL || bodyParts.size() == 0) //Already added???
        return;
    
    //Create compound rigid body
    /*
      1. Calculate compound mass and compound COG (sum of location - local * m / M)
      2. Calculate inertia of part in global frame and compound COG (rotate and translate inertia tensor) 3x3
         and calculate compound inertia 3x3 (sum of parts inertia)
      3. Calculate primary moments of inertia
      4. Find primary axes of inertia
      5. Rotate frame to match primary axes and move to COG
      6. Create rigid body with inverse transformation
    */
     
    //1. Calculate compound mass and COG
    btVector3 compoundCOG = btVector3(0,0,0);
    btScalar compoundMass = 0;
    localTransform = btTransform::getIdentity();
        
    for(unsigned int i=0; i<bodyParts.size(); ++i)
    {
        compoundMass += bodyParts[i].solid->getMass();
        //compoundCOG += (bodyParts[i].position.getOrigin() + bodyParts[i].solid->getLocalTransform().getOrigin())*bodyParts[i].solid->getMass();
        compoundCOG += (bodyParts[i].position*bodyParts[i].solid->getLocalTransform().getOrigin())*bodyParts[i].solid->getMass();
    }
    
    compoundCOG /= compoundMass;
    localTransform.setOrigin(compoundCOG);
        
    //2. Calculate compound inertia matrix
    btMatrix3x3 I = btMatrix3x3(0.,0.,0.,0.,0.,0.,0.,0.,0.);
        
    for(unsigned int i=0; i<bodyParts.size(); ++i)
    {
        //Calculate inertia matrix 3x3 of solid in the global frame and COG
        btVector3 solidPriInertia = bodyParts[i].solid->getMomentsOfInertia();
        btMatrix3x3 solidInertia = btMatrix3x3(solidPriInertia.x(), 0, 0, 0, solidPriInertia.y(), 0, 0, 0, solidPriInertia.z());
            
        //Rotate inertia tensor from local to global
        btMatrix3x3 rotation = bodyParts[i].position.getBasis()*bodyParts[i].solid->getLocalTransform().getBasis();
        solidInertia = rotation * solidInertia * rotation.transpose();
            
        //Translate inertia tensor from local COG to global COG
        btVector3 translation = bodyParts[i].position.getOrigin()+bodyParts[i].solid->getLocalTransform().getOrigin()-compoundCOG;
        solidInertia = solidInertia +  btMatrix3x3(translation.y()*translation.y()+translation.z()*translation.z(), -translation.x()*translation.y(), -translation.x()*translation.z(),
                                                    -translation.y()*translation.x(), translation.x()*translation.x()+translation.z()*translation.z(), -translation.y()*translation.z(),
                                                    -translation.z()*translation.x(), -translation.z()*translation.y(), translation.x()*translation.x()+translation.y()*translation.y()).scaled(btVector3(bodyParts[i].solid->getMass(), bodyParts[i].solid->getMass(), bodyParts[i].solid->getMass()));
            
        //Accumulate inertia tensor
        I += solidInertia;
    }
    
    //3.Calculate principal moments of inertia
    btScalar T = I[0][0] + I[1][1] + I[2][2]; //Ixx + Iyy + Izz
    btScalar II = I[0][0]*I[1][1] + I[0][0]*I[2][2] + I[1][1]*I[2][2] - I[0][1]*I[0][1] - I[0][2]*I[0][2] - I[1][2]*I[1][2]; //Ixx Iyy + Ixx Izz + Iyy Izz - Ixy^2 - Ixz^2 - Iyz^2
    btScalar U = btSqrt(T*T-btScalar(3.)*II)/btScalar(3.);
    btScalar theta = btAcos((-btScalar(2.)*T*T*T + btScalar(9.)*T*II - btScalar(27.)*I.determinant())/(btScalar(54.)*U*U*U));
    btScalar A = T/btScalar(3.) - btScalar(2.)*U*btCos(theta/btScalar(3.));
    btScalar B = T/btScalar(3.) - btScalar(2.)*U*btCos(theta/btScalar(3.) - btScalar(2.)*M_PI/btScalar(3.));
    btScalar C = T/btScalar(3.) - btScalar(2.)*U*btCos(theta/btScalar(3.) + btScalar(2.)*M_PI/btScalar(3.));
    btVector3 compoundPriInertia = btVector3(A, B, C);
    
    //4. Calculate principal axes of inertia
    btMatrix3x3 L;
    btVector3 axis1,axis2,axis3;
    axis1 = findInertiaAxis(I, A);
    axis2 = findInertiaAxis(I, B);
    axis3 = axis1.cross(axis2);
    axis2 = axis3.cross(axis1);
    
    //5.Rotate body so that principal axes are parallel to (x,y,z) system
    btMatrix3x3 rotMat(axis1[0],axis2[0],axis3[0], axis1[1],axis2[1],axis3[1], axis1[2],axis2[2],axis3[2]);
    localTransform.setBasis(rotMat);
    
    //6. Build collision shape from external parts
    btCompoundShape* compoundShape = new btCompoundShape();
    for(unsigned int i=0; i<bodyParts.size(); ++i)
    {
        if(bodyParts[i].isExternal)
        {
            btTransform childTrans = localTransform.inverse()*bodyParts[i].position;
            compoundShape->addChildShape(childTrans, bodyParts[i].solid->BuildCollisionShape());
        }
    }

    //7. Build rigid body from all parts
    btDefaultMotionState* motionState = new btDefaultMotionState(localTransform);
        
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(compoundMass, motionState, compoundShape, compoundPriInertia);
    rigidBodyCI.m_friction = 0.5;
    rigidBodyCI.m_rollingFriction = 0.1;
    rigidBodyCI.m_restitution = 0.5;
    rigidBodyCI.m_linearDamping = 0.0;
    rigidBodyCI.m_angularDamping = 0.0;
    rigidBodyCI.m_linearSleepingThreshold = 0.001;
    rigidBodyCI.m_angularSleepingThreshold = 0.01;
        
    vehicleBody = new btRigidBody(rigidBodyCI);
    vehicleBody->setActivationState(DISABLE_DEACTIVATION);
    vehicleBody->setUserPointer(this);

    //Add vehicle to dynamics world
    vehicleBody->setCenterOfMassTransform(localTransform * UnitSystem::SetTransform(worldTransform));
    world->addRigidBody(vehicleBody, MASK_DEFAULT, MASK_STATIC | MASK_DEFAULT);
    
    //Add manipulators to dynamics world
    //Add joints connecting manipulators and vehicle
    //Display
    BuildGraphicalObjects();
}

void UnderwaterVehicle::UpdateAcceleration()
{
    if(vehicleBody != NULL)
    {
        linearAcc = vehicleBody->getTotalForce()*vehicleBody->getInvMass();
        btVector3 torque = vehicleBody->getTotalTorque();
        angularAcc = btVector3(torque.x()*vehicleBody->getInvInertiaDiagLocal().x() , torque.y()*vehicleBody->getInvInertiaDiagLocal().y(), torque.z()*vehicleBody->getInvInertiaDiagLocal().z());
    }
}

void UnderwaterVehicle::UpdateSensors(btScalar dt)
{
    for(unsigned int i=0; i<manipulators.size(); ++i)
        manipulators[i]->UpdateSensors(dt);
    
    for(unsigned int i=0; i<sensors.size(); ++i)
        sensors[i]->Update(dt);
}

void UnderwaterVehicle::UpdateControllers(btScalar dt)
{
    for(unsigned int i=0; i<manipulators.size(); ++i)
        manipulators[i]->UpdateControllers(dt);
}

void UnderwaterVehicle::UpdateActuators(btScalar dt)
{
    for(unsigned int i=0; i<manipulators.size(); ++i)
        manipulators[i]->UpdateActuators(dt);
    
    for(unsigned int i=0; i<thrusters.size(); ++i)
        thrusters[i]->Update(dt);
}

void UnderwaterVehicle::ApplyGravity()
{
    if(vehicleBody != NULL)
        vehicleBody->applyGravity();
}

void UnderwaterVehicle::ApplyFluidForces(Ocean* fluid)
{
    btVector3 Fb;
    btVector3 Tb;
    btVector3 Fd;
    btVector3 Td;
    btVector3 Fa;
    btVector3 Ta;

    btVector3 v = vehicleBody->getLinearVelocity();
    btVector3 omega = vehicleBody->getAngularVelocity();
    
    for(unsigned int i=0; i<bodyParts.size(); ++i)
    {
        btTransform T = getTransform() * localTransform.inverse() * bodyParts[i].position;
        bodyParts[i].solid->ComputeFluidForces(fluid, getTransform(), T, v, omega, linearAcc, angularAcc, Fb, Tb, Fd, Td, Fa, Ta, bodyParts[i].isExternal, bodyParts[i].isExternal);
        vehicleBody->applyCentralForce(Fb+Fd+Fa);
        vehicleBody->applyTorque(Tb+Td+Ta);
    }
}

void UnderwaterVehicle::BuildGraphicalObjects()
{
	for(unsigned int i=0; i<bodyParts.size(); ++i)
		bodyParts[i].solid->BuildGraphicalObject();
}

void UnderwaterVehicle::Render()
{
    if(vehicleBody != NULL)
    {
		btTransform vehicleTrans;
        vehicleBody->getMotionState()->getWorldTransform(vehicleTrans);
        vehicleTrans *= localTransform.inverse();
		
		for(unsigned int i=0; i<bodyParts.size(); ++i)
		{
			btTransform trans = vehicleTrans * bodyParts[i].position;
			btScalar openglTrans[16];
			trans.getOpenGLMatrix(openglTrans);
			OpenGLContent::getInstance()->DrawObject(bodyParts[i].solid->getObject(), bodyParts[i].solid->getLook(), openglTrans);
		}
	}
}
