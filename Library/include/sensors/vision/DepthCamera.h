//
//  DepthCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 07/05/18.
//  Copyright (c) 2018-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_DepthCamera__
#define __Stonefish_DepthCamera__

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
         \param horizontalFOVDeg the horizontal field of view [deg]
         \param minDepth the minimum measured depth [m]
         \param maxDepth the maximum measured depth [m]
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         */
        DepthCamera(std::string uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar horizontalFOVDeg,
                    Scalar minDepth, Scalar maxDepth, Scalar frequency = Scalar(-1));
       
        //! A destructor.
        ~DepthCamera();
        
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
        void NewDataReady(unsigned int index = 0);
        
        //! A method used to set a callback function called when new data is available.
        /*!
         \param callback a function to be called
         */
        void InstallNewDataHandler(std::function<void(DepthCamera*)> callback);
        
        //! A method returning the depth range of the camera.
        glm::vec2 getDepthRange();
        
        //! A method returning the pointer to the image data.
        /*!
         \param index the id of the OpenGL camera for which the data pointer is requested
         \return pointer to the image data buffer
         */
        void* getImageDataPointer(unsigned int index = 0);
        
    private:
        void InitGraphics();
        
        OpenGLDepthCamera* glCamera;
        GLfloat* imageData;
        glm::vec2 depthRange;
        std::function<void(DepthCamera*)> newDataCallback;
    };
}

#endif
