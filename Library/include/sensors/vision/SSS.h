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
//  SSS.h
//  Stonefish
//
//  Created by Patryk Cieslak on 20/06/20.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SSS__
#define __Stonefish_SSS__

#include <functional>
#include "sensors/vision/Camera.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    class OpenGLSSS;
    
    //! A class representing a side-scan sonar.
    class SSS : public Camera
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param numOfBins the number of the range bins in the sonar image
         \param numOfLines the length of the waterfall display memory
         \param verticalBeamWidthDeg the vertical width of the beam [deg]
         \param horizontalBeamWidthDeg the horizontal width of the beam [deg]
         \param verticalTiltDeg the angle between the horizon and the transducer axis [deg]
         \param minRange the minimum measured range [m]
         \param maxRange the maximum measured range [m]
         \param cm the color map used to display sonar data
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated based on maximum range)
         */
        SSS(std::string uniqueName, unsigned int numOfBins, unsigned int numOfLines, Scalar verticalBeamWidthDeg,
            Scalar horizontalBeamWidthDeg, Scalar verticalTiltDeg, Scalar minRange, Scalar maxRange, ColorMap cm, 
            Scalar frequency = Scalar(-1));
       
        //! A destructor.
        ~SSS();
        
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
        void NewDataReady(void* data, unsigned int index = 0);
        
        //! A method used to set a callback function called when new data is available.
        /*!
         \param callback a function to be called
         */
        void InstallNewDataHandler(std::function<void(SSS*)> callback);
        
        //! A method implementing the rendering of the sonar dummy.
        std::vector<Renderable> Render();
        
        //! A method setting the minimum range of the sonar.
        /*!
         \param r range [m]
         */
        void setRangeMin(Scalar r);

        //! A method setting the minimum range of the sonar.
        /*!
         \param r range [m]
         */
        void setRangeMax(Scalar r);

        //! A method setting the gain of the sonar.
        /*!
         \param g gain factor [1]
         */
        void setGain(Scalar g);

        //! A method setting the noise characteristics of the sensor.
        /*!
         \param multiplicativeStdDev the standard deviation of the multiplicative noise
         \param additiveStdDev the standard deviation of the additive noise
         */
        void setNoise(float multiplicativeStdDev, float additiveStdDev);

        //! A method returning the minimum range of the sonar.
        Scalar getRangeMin() const;
        
        //! A method returning the maximum range of the sonar.
        Scalar getRangeMax() const;

        //! A method returning the gain of the sonar.
        Scalar getGain() const;
        
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
        GLubyte* getDisplayDataPointer();
        
        //! A method returning the type of the vision sensor.
        VisionSensorType getVisionSensorType();
        
    private:
        void InitGraphics();
        
        OpenGLSSS* glSSS;
        GLubyte* sonarData;
        GLubyte* displayData;
        glm::vec2 range;
        glm::vec2 noise;
        Scalar gain;
        Scalar fovV;
        Scalar tilt;
        ColorMap cMap;
        std::function<void(SSS*)> newDataCallback;
    };
}

#endif
