//
//  StaticEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_StaticEntity__
#define __Stonefish_StaticEntity__

#include "core/MaterialManager.h"
#include "entities/Entity.h"

namespace sf
{
    //!
    typedef enum {STATIC_PLANE, STATIC_TERRAIN, STATIC_OBSTACLE} StaticEntityType;
    
    struct Mesh;
    
    //!
    class StaticEntity : public Entity
    {
    public:
        StaticEntity(std::string uniqueName, Material m, int lookId = -1);
        virtual ~StaticEntity();
        
        std::vector<Renderable> Render();
        void SetLook(int newLookId);
        void SetWireframe(bool enabled);
        
        void AddToSimulation(SimulationManager* sm);
        void AddToSimulation(SimulationManager* sm, const Transform& origin);
        virtual void getAABB(Vector3& min, Vector3& max);
        
        void setTransform(const Transform& trans);
        Transform getTransform();
        Material getMaterial();
        btRigidBody* getRigidBody();
        EntityType getType();
        
        virtual StaticEntityType getStaticType() = 0;
        
        static void GroupTransform(std::vector<StaticEntity*>& objects, const Transform& centre, const Transform& transform);
        
    protected:
        void BuildRigidBody(btCollisionShape* shape);
        virtual void BuildGraphicalObject();
        
        btRigidBody* rigidBody;
        Material mat;
        Mesh* phyMesh;
        
        int objectId;
        int lookId;
        bool wireframe;
    };
}

#endif
