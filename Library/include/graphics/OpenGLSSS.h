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
//  OpenGLSSS.h
//  Stonefish
//
//  Created by Patryk Cieslak on 20/06/20.
//  Copyright (c) 2020-2025 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLSSS__
#define __Stonefish_OpenGLSSS__

#include "graphics/OpenGLSonar.h"
#include "sensors/vision/SSS.h"
#include <random>

namespace sf
{
    class GLSLShader;
    class SSS;

    //! A class representing a side-scan sonar (SSS).
    class OpenGLSSS : public OpenGLSonar
    {
    public:
        //! A constructor.
        /*!
         \param centerPosition the position of the sonar center in world space [m]
         \param direction a unit vector parallel to the sonar central axis
         \param forward a vector aligned with the sonar track
         \param verticalBeamWidthDeg the field of view of the single sonar transducer [deg]
         \param horizontalBeamWidthDeg the width of the sonar beam along the track [deg]
         \param numOfBins the number of sonar range bins
         \param numOfLines the length of the waterfall display memory
         \param verticalTiltDeg the angle between the horizon and the transducer axis [deg]
         \param range the distance to the closest and farthest recorded object [m]
         \param outputFormat the format of the sonar output data
         */
        OpenGLSSS(glm::vec3 centerPosition, glm::vec3 direction, glm::vec3 forward,
                  GLfloat verticalBeamWidthDeg, GLfloat horizontalBeamWidthDeg, 
                  GLuint numOfBins, GLuint numOfLines, GLfloat verticalTiltDeg, glm::vec2 range,
                  const BeamPattern& verticalBeamPattern, SonarOutputFormat outputFormat);
        
        //! A destructor.
        ~OpenGLSSS();
        
        //! A method that computes simulated sonar data.
        /*!
         \param objects a reference to a vector of renderable objects
         */
        void ComputeOutput(std::vector<Renderable>& objects) override;
        
        //! A method to render the low dynamic range (final) image to the screen.
        /*!
         \param destinationFBO the id of the framebuffer used as the destination for rendering
         \param updated a flag indicating if view content was updated
         */
        void DrawLDR(GLuint destinationFBO, bool updated) override;
        
        //! A method that updates sonar world transform.
        void UpdateTransform() override;

        //! A method to set the noise properties of the sonar.
        /*!
         \param signalStdDev the standard deviation of the echo intensity
         */
        void setNoise(glm::vec2 signalStdDev);

        //! A method to set a pointer to a sonar sensor.
        /*!
         \param son a pointer to a sonar sensor
         */
        void setSonar(SSS* s);

        //! Return the linearly interpolated gain at one beam-relative angle.
        /*!
         \param pattern the beam pattern
         \param angleDeg the angle relative to the sonar central axis [deg]
         \return the gain factor [1]
         */
        static GLfloat SampleBeamPattern(const BeamPattern& pattern,
                                        GLfloat angleDeg);

        //! Build the beam pattern weights and upload them to a 1D texture.
        /*!
         \param pattern the beam pattern
         \param BeamWidthDeg the width of the sonar beam [deg]
         \return the OpenGL handle of the texture
         */
        GLuint CreateBeamPatternTexture(const BeamPattern& pattern, GLfloat BeamWidthDeg);
            
    protected:
        //SSS specific
        SSS* sonar_;
        GLfloat tilt_;
        glm::uvec2 nBeamSamples_;
        glm::vec2 noise_;
        glm::mat4 views_[2];
        
        //OpenGL
        GLuint outputTex_[3];
        GLuint verticalBeamPatternTex_;
        GLint pingpong_;
        GLSLShader* sonarOutputShader_[2];
        GLSLShader* sonarShiftShader_;
    };
}

#endif
