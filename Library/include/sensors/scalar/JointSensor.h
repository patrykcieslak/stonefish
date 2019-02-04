//
//  JointSensor.h
//  Stonefish
//
//  Created by Patryk Cieślak on 20/11/2018.
//  Copyright (c) 2018-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_JointSensor__
#define __Stonefish_JointSensor__

#include "sensors/ScalarSensor.h"

namespace sf
{
    class FeatherstoneEntity;
    class Joint;
    
    //! An abstract class representing a sensor attached to a joint.
    class JointSensor : public ScalarSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        JointSensor(std::string uniqueName, Scalar frequency, int historyLength);
        
        //! A method performing internal update of the sensor state.
        /*!
         \param dt the sample time of the simulation [s]
         */
        virtual void InternalUpdate(Scalar dt) = 0;
        
        //! A method used to attach the sensor to a multibody joint.
        /*!
         \param multibody a pointer to a multibody
         \param jointId an index of the multibody joint
         */
        virtual void AttachToJoint(FeatherstoneEntity* multibody, unsigned int jointId);
        
        //! A method used to attach the sensor to a discrete joint.
        /*!
         \param joint a pointer to a discrete joint
         */
        virtual void AttachToJoint(Joint* joint);
        
        //! A method returning the current sensor frame in world.
        virtual Transform getSensorFrame();
        
        //! A method returning the type of the sensor.
        SensorType getType();
        
    protected:
        FeatherstoneEntity* fe;
        unsigned int jId;
        Joint* j;
    };
}

#endif
