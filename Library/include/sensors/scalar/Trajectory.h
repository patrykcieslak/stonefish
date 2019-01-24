//
//  Trajectory.h
//  Stonefish
//
//  Created by Patryk Cieslak on 25/05/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Trajectory__
#define __Stonefish_Trajectory__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    //! A class representing a sensor that captures the trajectory of a rigid body.
    class Trajectory : public LinkSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        Trajectory(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method resetting the state of the sensor.
        std::vector<Renderable> Render();
    };
}

#endif
