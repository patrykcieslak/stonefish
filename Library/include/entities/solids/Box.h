//
//  Box.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Box__
#define __Stonefish_Box__

#include "entities/SolidEntity.h"

namespace sf
{
    //! A class representing a box-shaped rigid body.
    class Box : public SolidEntity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the box
         \param dimensions a vector of box dimensions (side lengths)
         \param origin a transformation of the physical mesh (graphical is the same for box)
         \param m a material of the box
         \param lookId an index of the graphical material used to render the box
         \param thickness defines the thickness of the box walls, if positive the box is treated as shell
         \param isBuoyant defines if the box should be subject to buoyancy force
        */
        Box(std::string uniqueName, const Vector3& dimensions, const Transform& origin, Material m,
                                    int lookId = -1, Scalar thickness = Scalar(-1), bool isBuoyant = true);
        
        //! A method that returns the type of body.
        SolidType getSolidType();
        
        //! A method that returns the collision shape for the box.
        btCollisionShape* BuildCollisionShape();
        
    private:
        Vector3 halfExtents;
    };
}

#endif
