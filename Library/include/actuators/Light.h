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
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    class OpenGLLight;
    
    //! A class representing a light of two common types: omni and spot.
    class Light : public LinkActuator
    {
    public:
        //! A constructor of an omni light.
        /*!
         \param uniqueName a name of the light
         \param color a color of the light
         \param illuminance a value of luminous flux per unit area of light [lux]
         */
        Light(std::string uniqueName, Color color, Scalar illuminance);
        
        //! A constructor of a spot light.
        /*!
         \param uniqueName a name of the light
         \param coneAngleDeg a cone angle of the spot light in degrees
         \param color a color of the light
         \param illuminance a value of luminous flux per unit area of light [lux]
         */
        Light(std::string uniqueName, Scalar coneAngleDeg, Color color, Scalar illuminance);
        
        //! A method used to attach the actuator to a specified link of a multibody tree.
        /*!
         \param multibody a pointer to a rigid multibody object
         \param linkId an index of the link
         \param origin a transformation from the link origin to the actuator origin
         */
        void AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId, const Transform& origin);
        
        //! A method used to attach the actuator to a specified rigid body.
        /*!
         \param solid a pointer to a rigid body
         \param origin a transformation from the body origin to the actuator origin
         */
        void AttachToSolid(SolidEntity* solid, const Transform& origin);
        
        //! A method which updates the pose of the light
        /*!
         \param dt a time step of the simulation
         */
        void Update(Scalar dt);
        
        //! A method that updates light position.
        void UpdateTransform();
        
        //! A method implementing the rendering of the light dummy.
        std::vector<Renderable> Render();
        
        //! A method returning the type of the actuator.
        ActuatorType getType();
        
    private:
        void InitGraphics();
        
        Color c;
        Scalar illum;
        Scalar coneAngle;
        OpenGLLight* glLight;
    };
}

#endif
