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
//  OpenGLMSIS.h
//  Stonefish
//
//  Created by Patryk Cieslak on 21/07/20.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLMSIS__
#define __Stonefish_OpenGLMSIS__

#include "graphics/OpenGLSonar.h"
#include <random>

namespace sf
{
    class GLSLShader;
    class MSIS;

    //! A class representing a mechanical scanning imaging sonar (MSIS).
    class OpenGLMSIS : public OpenGLSonar
    {
    public:
        //! A constructor.
        /*!
         \param eyePosition the position of the sonar eye in world space [m]
         \param direction a unit vector parallel to the sonar central axis
         \param sonarUp a unit vector perpendicular to the sonar plane
         \param horizontalBeamWidthDeg the horizontal width of the sonar beam [deg]
         \param verticalBeamWidthDeg the vertical width of the sonar beam [deg]
         \param numOfSteps the number of sonar motor steps
         \param numOfBins the number of sonar range bins
         \param range_ the distance to the closest and farthest recorded object [m]
         */
        OpenGLMSIS(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 sonarUp,
                   GLfloat horizontalBeamWidthDeg, GLfloat verticalBeamWidthDeg, 
                   GLuint numOfSteps, GLuint numOfBins, glm::vec2 range_);
        
        //! A destructor.
        ~OpenGLMSIS();
        
        //! A method that computes simulated sonar data.
        /*!
         \param objects a reference to a vector of renderable objects
         */
        void ComputeOutput(std::vector<Renderable>& objects);
        
        //! A method to render the low dynamic range (final) image to the screen.
        /*!
         \param destinationFBO the id of the framebuffer used as the destination for rendering
         \param updated a flag indicating if view content was updated
         */
        void DrawLDR(GLuint destinationFBO, bool updated);
        
        //! A method that updates sonar world transform.
        void UpdateTransform();

        //! A method to set the noise properties of the sonar.
        /*!
         \param signalStdDev the standard deviation of the echo intensity
         */
        void setNoise(glm::vec2 signalStdDev);

        //! A method to set a pointer to a sonar sensor.
        /*!
         \param son a pointer to a sonar sensor
         */
        void setSonar(MSIS* s);
        
    protected:
        //MSIS specific
        MSIS* sonar;
        GLuint nSteps;
        GLuint nBins;
        glm::uvec2 nBeamSamples;
        glm::mat4 beamRotation;
        GLint currentStep;
        glm::vec2 fov;
        glm::vec2 rotationLimits;
        glm::vec2 noise;
        //OpenGL
        GLuint outputTex[2];
        GLuint fanDiv;
        GLSLShader* sonarOutputShader;
        GLSLShader* sonarUpdateShader;
    };
}

#endif
