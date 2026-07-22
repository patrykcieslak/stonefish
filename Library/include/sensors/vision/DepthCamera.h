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
//  DepthCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 07/05/18.
//  Copyright (c) 2018-2026 Patryk Cieslak. All rights reserved.
//

#pragma once

#include <functional>
#include "sensors/vision/Camera.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    class OpenGLDepthCamera;
    
    //! A class representing a depth camera.
    class DepthCamera : public Camera
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param resolutionX the horizontal resolution [pix]
         \param resolutionY the vertical resolution[pix]
         \param hFOVDeg the horizontal field of view [deg]
         \param minDepth the minimum measured depth [m]
         \param maxDepth the maximum measured depth [m]
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         */
        DepthCamera(const std::string& uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar hFOVDeg,
                    Scalar minDepth, Scalar maxDepth, Scalar frequency = Scalar(-1));
        
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
        void InstallNewDataHandler(std::function<void(DepthCamera*)> callback);
        
        //! A method used to set the noise characteristics of the sensor.
        /*!
         \param depthStdDev standard deviation of the depth measurement at 1m distance
         */
        void setNoise(float depthStdDev);

        //! A method returning the depth range of the camera.
        glm::vec2 getDepthRange() const;
        
        //! A method returning the pointer to the image data.
        /*!
         \param index the id of the OpenGL camera for which the data pointer is requested
         \return pointer to the image data buffer
         */
        void* getImageDataPointer(unsigned int index = 0);
        
        //! A method returning the type of the vision sensor.
        VisionSensorType getVisionSensorType() const override;

        //! A method returning a pointer to the underlaying OpenGLView object.
        OpenGLView* getOpenGLView() const override;

        //! A method returning the construction info for the sensor.
        static ConstructInfo getConstructInfo();

        //! A method constructing the sensor based on info structure.
        /*!
         \param info a construction info structure
        */
        static std::unique_ptr<DepthCamera> Construct(const std::string& uniqueName, Scalar frequency, ConstructInfo& info);
        
    private:
        void InitGraphics(bool& seesParticles);
        
        OpenGLDepthCamera* glCamera_;
        GLfloat* imageData_;
        glm::vec2 depthRange_;
        GLfloat noiseStdDev_;
        std::function<void(DepthCamera*)> newDataCallback_;
    };
}

