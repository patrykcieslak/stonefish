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
//  ThermalCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 26/05/24.
//  Copyright (c) 2024 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ThermalCamera__
#define __Stonefish_ThermalCamera__

#include <functional>
#include "sensors/vision/Camera.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    class OpenGLThermalCamera;
    
    //! A class representing an optical flow camera.
    class ThermalCamera : public Camera
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param resolutionX the horizontal resolution [pix]
         \param resolutionY the vertical resolution[pix]
         \param hFOVDeg the horizontal field of view [deg]
         \param minTemp minimum temperature that the sensor can measure [degC]
         \param maxTemp maximum temperature that the sensor can measure [degC]
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param minDistance the minimum drawing distance [m]
         \param maxDistance the maximum drawing distance [m]
         */
        ThermalCamera(std::string uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar hFOVDeg, Scalar minTemp, Scalar maxTemp, 
            Scalar frequency = Scalar(-1), Scalar minDistance = Scalar(STD_NEAR_PLANE_DISTANCE), Scalar maxDistance = Scalar(STD_FAR_PLANE_DISTANCE));
        
        //! A destructor.
        ~ThermalCamera();
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to setup the OpenGL camera transformation.
        /*!
         \param eye the position of the camera eye [m]
         \param dir a unit vector parallel to the optical axis of the camera
         \param up a unit vector pointing up (from center of image to the top edge of the image)
         */
        void SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up);
        
        //! A method used to inform about new data.
        /*!
         \param index the id of the OpenGL camera uploading the data
         */
        void NewDataReady(void* data, unsigned int index = 0);
        
        //! A method used to set a callback function called when new data is available.
        /*!
         \param callback a function to be called
         */
        void InstallNewDataHandler(std::function<void(ThermalCamera*)> callback);
        
        //! A method used to set the noise characteristics of the sensor.
        /*!
         \param tempStdDev standard deviation of the temperature measurement [degC]
         */
        void setNoise(float tempStdDev);

        //! A method used to set the display settings of the sensor.
        /*!
         \param cm the color map used to display the image
         \param minTemp the minimum temperature displayed [degC]
         \param maxTemp the maximum temperature displayed [degC]
         */
        void setDisplaySettings(ColorMap cm, Scalar minTemp, Scalar maxTemp);

        //! A method returning the pointer to the image data.
        /*!
         \param index the id of the OpenGL camera for which the data pointer is requested
         \return pointer to the image data buffer
         */
        void* getImageDataPointer(unsigned int index = 0);

        //! A method returning a pointer to the visualisation image data.
        GLubyte* getDisplayDataPointer();
        
        //! A method returning the type of the vision sensor.
        VisionSensorType getVisionSensorType() const;
        
    private:
        void InitGraphics();
        
        OpenGLThermalCamera* glCamera;
        GLfloat* temperatureData;
        GLubyte* displayData;
        glm::vec2 depthRange;
        GLfloat noiseStdDev;
        glm::vec2 measurementRange;
        ColorMap colorMap;
        glm::vec2 displayRange;
        std::function<void(ThermalCamera*)> newDataCallback;
    };
}

#endif
