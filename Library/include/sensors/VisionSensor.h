/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
    //! An enum defining types of vision sensors.
    typedef enum {SENSOR_COLOR_CAMERA = 0, SENSOR_DEPTH_CAMERA, SENSOR_MULTIBEAM2} VisionSensorType;
    
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
        
        //! A method returning the type of the vision sensor.
        virtual VisionSensorType getVisionSensorType() = 0;
        
    protected:
        virtual void InitGraphics() = 0;
        
    private:
        SolidEntity* attach;
        Transform o2s;
    };
}

#endif
