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
//  MSIS.h
//  Stonefish
//
//  Created by Patryk Cieslak on 21/07/20.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_MSIS__
#define __Stonefish_MSIS__

#include <functional>
#include "sensors/vision/Camera.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    class OpenGLMSIS;
    
    //! A class representing a mechanical scanning imaging sonar.
    class MSIS : public Camera
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param stepAngleDeg size of the rotation step [deg]
         \param numOfBins a range resolution of the sonar image
         \param horizontalBeamWidthDeg the horizontal beam width [deg]
         \param verticalBeamWidthDeg the vertical beam width [deg]
         \param minRotationDeg the minimum rotation of the sonar head [deg]
         \param maxRotationDeg the maximum rotation of the sonar head [deg]
         \param minRange the minimum measured range [m]
         \param maxRange the maximum measured range [m]
         \param cm the color map used to display sonar data
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated based on maximum range)
         */
        MSIS(std::string uniqueName, Scalar stepAngleDeg, unsigned int numOfBins, Scalar horizontalBeamWidthDeg, Scalar verticalBeamWidthDeg,
             Scalar minRotationDeg, Scalar maxRotationDeg, Scalar minRange, Scalar maxRange, ColorMap cm, Scalar frequency = Scalar(-1));
       
        //! A destructor.
        ~MSIS();
        
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
        void InstallNewDataHandler(std::function<void(MSIS*)> callback);
        
        //! A method implementing the rendering of the sonar dummy.
        std::vector<Renderable> Render();

        //! A method setting the limits of the sonar head rotation.
        /*!
         \param l1Deg first limit of rotation angle [deg]
         \param l2Deg second limit of rotation angle [deg]
         */
        void setRotationLimits(Scalar l1Deg, Scalar l2Deg);
        
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

        //! A method returning the rotation limits.
        /*!
         \param l1Deg first limit of rotation angle [deg]
         \param l2Deg second limit of rotation angle [deg]
         */
        void getRotationLimits(Scalar& l1Deg, Scalar& l2Deg) const;

        //! A method returning the minimum range of the sonar.
        Scalar getRangeMin() const;
        
        //! A method returning the maximum range of the sonar.
        Scalar getRangeMax() const;

        //! A method returning the gain of the sonar.
        Scalar getGain() const;

        //! A method returning the step size.
        Scalar getRotationStepAngle() const;

        //! A method returning the current rotation step.
        int getCurrentRotationStep() const;

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
        
        OpenGLMSIS* glMSIS;
        GLubyte* sonarData;
        GLubyte* displayData;
        int currentStep;
        bool cw;
        glm::ivec2 roi;
        bool fullRotation;
        glm::vec2 range;
        glm::vec2 noise;
        Scalar gain;
        Scalar fovV;
        Scalar stepSize;
        ColorMap cMap;
        std::function<void(MSIS*)> newDataCallback;
    };
}

#endif
