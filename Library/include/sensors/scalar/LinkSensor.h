//
//  LinkSensor.h
//  Stonefish
//
//  Created by Patryk Cieślak on 20/11/2018.
//  Copyright (c) 2018-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_LinkSensor__
#define __Stonefish_LinkSensor__

#include "sensors/ScalarSensor.h"

namespace sf
{
    class SolidEntity;
    class FeatherstoneEntity;
    
    //! An abstract class representing a sensor attached to a rigid body.
    class LinkSensor : public ScalarSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        LinkSensor(std::string uniqueName, Scalar frequency, int historyLength);
        
        //! A destructor.
        virtual ~LinkSensor();
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        virtual void InternalUpdate(Scalar dt) = 0;
        
        //! A method implementing the rendering of the sensor.
        virtual std::vector<Renderable> Render();
        
        //! A method used to attach the sensor to a multibody link.
        /*!
         \param multibody a pointer to a multibody
         \param linkId the id of the link
         \param origin a transformation from the link frame to the sensor frame
         */
        void AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId, const Transform& origin);
        
        //! A method used to attach the sensor to a rigid body.
        /*!
         \param solid a pointer to a rigid body
         \param origin a transformation from the body frame to the sensor frame
         */
        void AttachToSolid(SolidEntity* solid, const Transform& origin);
        
        //! A method returning the current sensor frame in world.
        Transform getSensorFrame();
        
        //! A method returning the type of the sensor.
        SensorType getType();
        
    protected:
        SolidEntity* attach;
        Transform o2s;
    };
}

#endif
