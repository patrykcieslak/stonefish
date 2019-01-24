//
//  ColorCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 4/5/18.
//  Copyright (c) 2018-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ColorCamera__
#define __Stonefish_ColorCamera__

#include <functional>
#include "sensors/vision/Camera.h"

namespace sf
{
    class OpenGLRealCamera;
    
    //! A class representing a color camera.
    class ColorCamera : public Camera
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param resolutionX the horizontal resolution [pix]
         \param resolutionY the vertical resolution[pix]
         \param horizFOVDeg the horizontal field of view [deg]
         \param spp number of samples per pixel used (>1 needs multisampling support)
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         */
        ColorCamera(std::string uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar horizFOVDeg, unsigned char spp = 1, Scalar frequency = Scalar(-1));
        
        //! A destructor.
        ~ColorCamera();
        
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
        void InstallNewDataHandler(std::function<void(ColorCamera*)> callback);
        
        //! A method used to set the exposure compensation factor.
        /*!
         \param comp the exposure compensation value [EV]
         */
        void setExposureCompensation(Scalar comp);
        
        //! A method returning the exposure compensation factor [EV].
        Scalar getExposureCompensation();
    
        //! A method returning the pointer to the image data.
        /*!
         \param index the id of the OpenGL camera for which the data pointer is requested
         \return pointer to the image data buffer
         */
        void* getImageDataPointer(unsigned int index = 0);
        
    private:
        void InitGraphics();
        
        OpenGLRealCamera* glCamera;
        unsigned char samples;
        uint8_t* imageData;
        std::function<void(ColorCamera*)> newDataCallback;
    };
}

#endif
