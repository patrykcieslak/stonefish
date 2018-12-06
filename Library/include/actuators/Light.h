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
         \param origin a pose of the light in the link frame
         \param color a color of the light
         \param illuminance a value of luminous flux per unit area of light [lux]
         */
        Light(std::string uniqueName, const Transform& origin, Color color, float illuminance);
        
        //! A constructor of a spot light.
        /*!
         \param uniqueName a name of the light
         \param origin a pose of the light in the link frame (cone axis is z axis)
         \param coneAngleDeg a cone angle of the spot light in degrees
         \param color a color of the light
         \param illuminance a value of luminous flux per unit area of light [lux]
         */
        Light(std::string uniqueName, const Transform& origin, Scalar coneAngleDeg, Color color, float illuminance);
        
        //! A method which updates the pose of the light
        /*!
         \param dt a time step of the simulation
         */
        void Update(Scalar dt);
        
        //! A method that updates light position.
        void UpdateTransform();
        
        //! A method returning the type of the actuator.
        ActuatorType getType();
        
    private:
        OpenGLLight* glLight;
    };
}

#endif
