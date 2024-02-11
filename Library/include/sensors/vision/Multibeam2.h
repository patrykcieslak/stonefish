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
//  Multibeam2.h
//  Stonefish
//
//  Created by Patryk Cieślak on 21/01/2019.
//  Copyright (c) 2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Multibeam2__
#define __Stonefish_Multibeam2__

#include <functional>
#include "sensors/vision/Camera.h"
#include "graphics/OpenGLDataStructs.h"

#define MULTIBEAM_MAX_SINGLE_FOV 30.0

namespace sf
{
    class OpenGLDepthCamera;
    
    //! A structure holding information about a single OpenGL camera.
    struct CamData
    {
        OpenGLDepthCamera* cam;
        int width;
        GLfloat fovH;
        size_t dataOffset;
    };
    
    //! A class representing a multibeam sonar (simulated with a number of depth cameras).
    class Multibeam2 : public Camera
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param horizontalRes the horizontal resolution [pix]
         \param verticalRes the vertical resolution [pix]
         \param horizontalFOVDeg the horizontal field of view [deg]
         \param verticalFOVDeg the vertical field of view [deg]
         \param minRange the minimum measured range [m]
         \param maxRange the maximum measured range [m]
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         */
        Multibeam2(std::string uniqueName, unsigned int horizontalRes, unsigned int verticalRes, Scalar horizontalFOVDeg, Scalar verticalFOVDeg,
                    Scalar minRange, Scalar maxRange, Scalar frequency = Scalar(-1));
        
        //! A destructor.
        ~Multibeam2();
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method updating the transform of the multibeam.
        void UpdateTransform();
        
        //! A method used to setup the OpenGL depth camera transformation (not applicable).
        /*!
         \param eye the position of the camera eye [m]
         \param dir a unit vector parallel to the optical axis of the camera
         \param up a unit vector pointing up (from center of image to the top edge of the image)
         */
        void SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up);
        
        //! A method used to setup a specified OpenGL depth camera transformation.
        /*!
         \param index an id of the camera to update
         \param eye the position of the camera eye [m]
         \param dir a unit vector parallel to the optical axis of the camera
         \param up a unit vector pointing up (from center of image to the top edge of the image)
         */
        void SetupCamera(size_t index, const Vector3& eye, const Vector3& dir, const Vector3& up);
        
        //! A method used to inform about new data.
        /*!
         \param index the id of the OpenGL depth camera uploading the data
         */
        void NewDataReady(void* data, unsigned int index = 0);
        
        //! A method used to set a callback function called when new data is available.
        /*!
         \param callback a function to be called
         */
        void InstallNewDataHandler(std::function<void(Multibeam2*)> callback);
        
        //! A method implementing the rendering of the multibeam dummy.
        std::vector<Renderable> Render();
        
        //! A method that returns the limits of measured range.
        glm::vec2 getRangeLimits();
        
        //! A method that returns the vertical field of view of the sensor.
        Scalar getVerticalFOV();
        
        //! A method returning a pointer to the image data.
        /*!
         \param index the id of the OpenGL camera for which the data pointer is requested
         \return pointer to the image data buffer
         */
        void* getImageDataPointer(unsigned int index = 0);
        
        //! A method returning a pointer to range data.
        float* getRangeDataPointer();
        
        //! A method returning the type of the vision sensor.
        VisionSensorType getVisionSensorType();
        
    private:
        void InitGraphics();
        
        std::vector<CamData> cameras;
        GLfloat* imageData;
        GLfloat* rangeData;
        Scalar fovV;
        glm::vec2 range;
        std::function<void(Multibeam2*)> newDataCallback;
        int dataCounter;
    };




}

#endif
