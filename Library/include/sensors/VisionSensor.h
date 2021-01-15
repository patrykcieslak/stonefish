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
//  Copyright (c) 2018-2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_VisionSensor__
#define __Stonefish_VisionSensor__

#include "sensors/Sensor.h"

namespace sf
{
    //! An enum defining types of vision sensors.
    enum class VisionSensorType {COLOR_CAMERA, DEPTH_CAMERA, MULTIBEAM2, FLS, SSS, MSIS};
    
    class Entity;
    class StaticEntity;
    class MovingEntity;
    
    //! An abstract class representing a vision sensor.
    class VisionSensor : public Sensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (0 if updated every simulation step)
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
        
        //! A method used to attach the sensor to the world frame.
        /*!
         \param origin the place where the sensor should be attached in the world frame
         */
        void AttachToWorld(const Transform& origin);
        
        //! A method used to attach the sensor to a static body.
        /*!
         \param body a pointer to the static body
         \param origin the place where the sensor should be attached in the body origin frame
         */
        void AttachToStatic(StaticEntity* body, const Transform& origin);

        //! A method used to attach the sensor to a moving body.
        /*!
         \param body a pointer to the moving body
         \param origin the place where the sensor should be attached in the body origin frame
         */
        void AttachToSolid(MovingEntity* body, const Transform& origin);
        
        //! A method used to set the sensor frame in the body frame.
        /*!
         \param origin a tranformation from the body frame to the sensor frame
         */
        void setRelativeSensorFrame(const Transform& origin);

        //! A method returning the sensor measurement frame.
        virtual Transform getSensorFrame() const;
        
        //! A method returning the type of the sensor.
        virtual SensorType getType();
        
        //! A method returning the type of the vision sensor.
        virtual VisionSensorType getVisionSensorType() = 0;
        
    protected:
        virtual void InitGraphics() = 0;
        
    private:
        Entity* attach;
        Transform o2s;
    };
}

#endif
