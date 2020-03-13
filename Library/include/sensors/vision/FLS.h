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
//  FLS.h
//  Stonefish
//
//  Created by Patryk Cieslak on 17/02/20.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FLS__
#define __Stonefish_FLS__

#include <functional>
#include "sensors/vision/Camera.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    class OpenGLFLS;
    
    //! A class representing a forward looking sonar.
    class FLS : public Camera
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param numOfBeams a number of acoustic beams
         \param numOfBins a range resolution of the sonar image
         \param horizontalFOVDeg the horizontal field of view [deg]
         \param verticalFOVDeg the vertical beam width [deg]
         \param minRange the minimum measured range [m]
         \param maxRange the maximum measured range [m]
         \param cm the color map used to display sonar data
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param beamHPix the number of samples taken for each beam in the horizontal plane [pix]
         \param beamVPix the number of samples taken for each beam in the vertical plane [pix]
         */
        FLS(std::string uniqueName, unsigned int numOfBeams, unsigned int numOfBins, Scalar horizontalFOVDeg, Scalar verticalFOVDeg,
                    Scalar minRange, Scalar maxRange, ColorMap cm, Scalar frequency = Scalar(-1), unsigned int beamHPix = 0, unsigned int beamVPix = 0);
       
        //! A destructor.
        ~FLS();
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to setup the OpenGL sonar transformation.
        /*!
         \param eye the position of the sonar eye [m]
         \param dir a unit vector parallel to the central axis of the sonar
         \param up a unit vector perpendicular to the sonar scanning plane
         */
        void SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up);
        
        //! A method used to inform about new data.
        /*!
         \param index the id of the OpenGL camera (here sonar) uploading the data
         */
        void NewDataReady(unsigned int index = 0);
        
        //! A method used to set a callback function called when new data is available.
        /*!
         \param callback a function to be called
         */
        void InstallNewDataHandler(std::function<void(FLS*)> callback);
        
        //! A method implementing the rendering of the sonar dummy.
        std::vector<Renderable> Render();
        
        //! A method returning the range limits of the sonar.
        glm::vec2 getRangeLimits();
        
        //! A method returning a pointer to the sonar data.
        /*!
         \param index the id of the OpenGL camera (here sonar) for which the data pointer is requested
         \return pointer to the image data buffer
         */
        void* getImageDataPointer(unsigned int index = 0);
        
        //! A method returning the resolution of the simulated display image.
        /*!
         \param x a reference to a variable that will store the horizontal resolution [pix]
         \param y a reference to a variable that will store the vertical resolution [pix]
         */
        void getDisplayResolution(unsigned int& x, unsigned int& y);
        
        //! A method returning a pointer to the visualisation image data.
        GLuint* getDisplayDataPointer();
        
        //! A method returning the type of the vision sensor.
        VisionSensorType getVisionSensorType();
        
    private:
        void InitGraphics();
        
        OpenGLFLS* glFLS;
        GLfloat* sonarData;
        GLuint* displayData;
        glm::vec2 range;
        glm::ivec2 beamRes;
        Scalar fovV;
        ColorMap cMap;
        std::function<void(FLS*)> newDataCallback;
    };
}

#endif
