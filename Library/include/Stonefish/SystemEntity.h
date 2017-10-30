//
//  SystemEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/05/2017.
//  Copyright © 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SystemEntity__
#define __Stonefish_SystemEntity__

#include "Entity.h"

typedef enum {SYSTEM_MANIPULATOR, SYSTEM_GRIPPER, SYSTEM_UNDERWATER_VEHICLE} SystemType;

class SystemEntity : public Entity
{
public:
    SystemEntity(std::string uniqueName);
    virtual ~SystemEntity();
    
    //Base methods
    EntityType getType();
    virtual void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world);

    //Pure virtual methods    
    virtual void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform) = 0;
	virtual void UpdateAcceleration(btScalar dt) = 0;
    virtual void UpdateSensors(btScalar dt);     //Update sensors in the system and its subsystems
    virtual void UpdateControllers(btScalar dt); //Update controllers in the system and its subsystems
    virtual void UpdateActuators(btScalar dt);   //Update actuators in the system and its subsystems
    virtual void ApplyGravity(const btVector3& g) = 0;
	virtual void ApplyDamping() = 0;
    
    virtual SystemType getSystemType() = 0;
    virtual btTransform getTransform() const = 0;
	virtual void GetAABB(btVector3& min, btVector3& max) = 0;
    virtual std::vector<Renderable> Render() = 0;
};

#endif
