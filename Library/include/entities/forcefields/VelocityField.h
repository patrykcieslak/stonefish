//
//  VelocityField.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/07/18.
//  Copyright (c) 2018-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_VelocityField__
#define __Stonefish_VelocityField__

#include "StonefishCommon.h"
#include "graphics/OpenGLContent.h"

namespace sf
{
    //! An abstract class representing a velocity field.
    class VelocityField
    {
    public:
        //! A constructor.
        VelocityField();
        
        //! A destructor.
        virtual ~VelocityField();
        
        //! A method returning velocity at a specified point.
        /*!
         \param p a point at which the velocity is requested
         \return velocity [m/s]
         */
        virtual Vector3 GetVelocityAtPoint(const Vector3& p) = 0;
        
        //! A method implementing the rendering of the velocity field.
        virtual std::vector<Renderable> Render() = 0;
    };
}

#endif
