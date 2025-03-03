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
//  EventBasedCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/03/24.
//  Copyright (c) 2024 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_EventBasedCamera__
#define __Stonefish_EventBasedCamera__

#include <functional>
#include "sensors/vision/Camera.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    class OpenGLEventBasedCamera;
    
    //! A class representing a color camera.
    class EventBasedCamera : public Camera
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param resolutionX the horizontal resolution [pix]
         \param resolutionY the vertical resolution[pix]
         \param hFOVDeg the horizontal field of view [deg]
         \param Cp positive polarity contrast threshold
         \param Cm negative polarity contrast threshold
         \param Tref refractory period [ns]
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param minDistance the minimum drawing distance [m]
         \param maxDistance the maximum drawing distance [m]
         */
        EventBasedCamera(std::string uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar hFOVDeg, 
           float Cp, float Cm, uint32_t Tref, Scalar frequency = Scalar(-1), Scalar minDistance = Scalar(STD_NEAR_PLANE_DISTANCE), 
           Scalar maxDistance = Scalar(STD_FAR_PLANE_DISTANCE)); //Rendering options
        
        //! A destructor.
        ~EventBasedCamera();
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt) override;
        
        //! A method used to setup the OpenGL camera transformation.
        /*!
         \param eye the position of the camera eye [m]
         \param dir a unit vector parallel to the optical axis of the camera
         \param up a unit vector pointing up (from center of image to the top edge of the image)
         */
        void SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up) override;
        
        //! A method used to inform about new data.
        /*!
         \param index the id of the OpenGL camera uploading the data
         */
        void NewDataReady(void* data, unsigned int index = 0) override;
        
        //! A method used to set a callback function called when new data is available.
        /*!
         \param callback a function to be called
         */
        void InstallNewDataHandler(std::function<void(EventBasedCamera*)> callback);

         //! A method setting the noise characteristics of the sensor.
        /*!
         \param sigmaCp the standard deviation of the positive polarity contrast threshold noise
         \param sigmaCm the standard deviation of the negative polarity contrast threshold noise
         */
        void setNoise(float sigmaCp, float sigmaCm);
        
        //! A method returning the pointer to the image data.
        /*!
         \param index the id of the OpenGL camera for which the data pointer is requested
         \return pointer to the image data buffer
         */
        void* getImageDataPointer(unsigned int index = 0);

        //! method returning the last number of generated events.
        unsigned int getLastEventCount() const;
        
        //! A method returning the type of the vision sensor.
        VisionSensorType getVisionSensorType() const override;

        //! A method returning a pointer to the underlaying OpenGLView object.
        OpenGLView* getOpenGLView() const override;
        
    private:
        void InitGraphics();
        
        OpenGLEventBasedCamera* glCamera;
        glm::vec2 depthRange;
        glm::vec2 C;
        glm::vec2 sigmaC;
        uint32_t Tr;
        uint32_t lastEventCount;
        GLint* imageData;
        std::function<void(EventBasedCamera*)> newDataCallback;
    };
}

#endif
