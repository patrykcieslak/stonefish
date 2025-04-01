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
//  Camera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Camera__
#define __Stonefish_Camera__

#include "sensors/VisionSensor.h"

namespace sf
{
    //! An abstract class representing a camera type sensor.
    class Camera : public VisionSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param resolutionX the horizontal resolution [pix]
         \param resolutionY the vertical resolution[pix]
         \param horizFOVDeg the horizontal field of view [deg]
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         */
        Camera(std::string uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar horizFOVDeg, Scalar frequency);
        
        //! A destructor.
        virtual ~Camera();
               
        //! A method used to inform about new data.
        /*!
         \param data a pointer to the OpenGL texture data
         \param index the id of the OpenGL camera uploading the data
         */
        virtual void NewDataReady(void* data, unsigned int index = 0) = 0;
        
        //! A method used to setup the OpenGL camera transformation.
        /*!
         \param eye the position of the camera eye [m]
         \param dir a unit vector parallel to the optical axis of the camera
         \param up a unit vector pointing up (from center of image to the top edge of the image)
         */
        virtual void SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up) = 0;
        
        //! A method updating the transform of the camera.
        virtual void UpdateTransform();
        
        //! A method implementing the rendering of the camera dummy.
        virtual std::vector<Renderable> Render();
        
        //! A method to set if the camera image should be displayed in the main window.
        /*!
         \param display show image in the program window?
         \param x position of the image in the window [px]
         \param y position of the image in the window [px]
         \param scale scaling factor of the image
         */
        void setDisplayOnScreen(bool display, unsigned int x, unsigned int y, float scale);
        
        //! A method informing if the camera image should be displayed in the main window.
        /*!
         \param x referece to the x position of the image in the window [px]
         \param y reference to the y position of the image in the window [px]
         \param scale reference to the scaling factor of the image
         \return a flag indicating if the view should be displayed
         */
        bool getDisplayOnScreen(unsigned int& x, unsigned int& y, float& scale) const;
        
        //! A method returning the horizontal field of view of the camera [deg].
        Scalar getHorizontalFOV() const;
        
        //! A method returning the resolution of the camera image.
        /*!
         \param x a reference to a variable that will store the horizontal resolution [pix]
         \param y a reference to a variable that will store the vertical resolution [pix]
         */
        void getResolution(unsigned int& x, unsigned int& y) const;
        
        //! A method returning the pointer to the image data.
        /*!
         \param index the id of the OpenGL camera for which the data pointer is requested
         \return pointer to the image data buffer
         */
        virtual void* getImageDataPointer(unsigned int index = 0) = 0;
        
    protected:
        Scalar fovH;
        unsigned int resX;
        unsigned int resY;
        unsigned int screenX;
        unsigned int screenY;
        float screenScale;
        bool screen;
    };
}

#endif
