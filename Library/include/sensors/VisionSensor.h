//
//  VisionSensor.h
//  Stonefish
//
//  Created by Patryk Cieślak on 20/11/2018.
//  Copyright (c) 2018-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_VisionSensor__
#define __Stonefish_VisionSensor__

#include "sensors/Sensor.h"

namespace sf
{
    class SolidEntity;
    class FeatherstoneEntity;
    
    //! An abstract class representing a vision sensor.
    class VisionSensor : public Sensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         */
        VisionSensor(std::string uniqueName, Scalar frequency);
        
        //! A destructor.
        virtual ~VisionSensor();
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the time stamp of the simulation [s]
         */
        virtual void InternalUpdate(Scalar dt) = 0;
        
        //! A method performing sensor transformation update.
        virtual void UpdateTransform() = 0;
        
        //! A method used to attach the sensor to a link of a multibody.
        /*!
         \param multibody a pointer to a multibody
         \param linkId the id of the attachment link
         \param origin the place where the sensor should be attached in the link origin frame
         */
        void AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId, const Transform& origin);
        
        //! A method used to attach the sensor to a rigid body.
        /*!
         \param solid a pointer to the rigid body
         \param origin the place where the sensor should be attached in the solid origin frame
         */
        void AttachToSolid(SolidEntity* solid, const Transform& origin);
        
        //! A method returning the sensor measurement frame.
        virtual Transform getSensorFrame();
        
        //! A method returning the type of the sensor.
        virtual SensorType getType();
        
    protected:
        virtual void InitGraphics() = 0;
        
    private:
        SolidEntity* attach;
        Transform o2s;
    };
}

#endif
