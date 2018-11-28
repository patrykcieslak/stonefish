//
//  Light.h
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Light__
#define __Stonefish_Light__

#include "actuators/LinkActuator.h"

namespace sf
{
    struct Color;
    class OpenGLLight;
    
    //! A class representing a light of two common types: omni and spot.
    class Light : public LinkActuator
    {
    public:
        //! A constructor of an omni light.
        /*!
         \param uniqueName a name of the light
         \param initialPos a position of light in the world frame
         \param color a color of the light
         */
        Light(std::string uniqueName, const Vector3& initialPos, Color color, float intensity);
        
        //! A constructor of a spot light.
        /*!
         \param uniqueName a name of the light
         \param initialPos a position of light in the world frame
         \param initialDir a direction of the spot in the world frame
         \param coneAngle a beam angle of the spot
         \param color a color of the light
         */
        Light(std::string uniqueName, const Vector3& initialPos, const Vector3& initialDir, Scalar coneAngle, Color color, float intensity);
        
        //! A method which updates the pose of the light
        /*!
         \param dt a time step of the simulation
         */
        void Update(Scalar dt);
        
    private:
        OpenGLLight* glLight;
    };
}

#endif
