//
//  Actuator.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/8/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Actuator__
#define __Stonefish_Actuator__

#include "StonefishCommon.h"

namespace sf
{
    struct Renderable;
    
    //! An enum designating a type of the actuator.
    typedef enum {ACTUATOR_JOINT, ACTUATOR_LINK} ActuatorType;
    
    //! An abstract class representing any actuator.
    class Actuator
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name of the actuator
         */
        Actuator(std::string uniqueName);
        
        //! A destructor.
        virtual ~Actuator();
        
        //! A method used to update the internal state of the actuator.
        /*!
         \param dt a time step of the simulation
         */
        virtual void Update(Scalar dt) = 0;
        
        //! A method implementing the rendering of the actuator.
        virtual std::vector<Renderable> Render();
        
        //! A method returning the type of the actuator.
        virtual ActuatorType getType() = 0;

        //! A method returning the name of the actuator.
        std::string getName();
        
    private:
        std::string name;
    };
}

#endif 
