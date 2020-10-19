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
//  OpenGLFLS.h
//  Stonefish
//
//  Created by Patryk Cieslak on 13/02/20.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLFLS__
#define __Stonefish_OpenGLFLS__

#include "graphics/OpenGLSonar.h"
#include <random>

namespace sf
{
    class GLSLShader;
    class FLS;

    struct SonarView
    {
        GLuint nBeams;
        glm::mat4 projection;
        glm::mat4 view;
    };

    //! A class representing a forward looking sonar (FLS).
    class OpenGLFLS : public OpenGLSonar
    {
    public:
        //! A constructor.
        /*!
         \param eyePosition the position of the sonar eye in world space [m]
         \param direction a unit vector parallel to the sonar central axis
         \param sonarUp a unit vector perpendicular to the sonar plane
         \param horizontalFovDeg the horizontal field of view of the sonar [deg]
         \param verticalFovDeg the vertical field of view of the sonar [deg]
         \param numOfBeams the number of sonar beams
         \param numOfBins the number of sonar range bins
         \param range_ the distance to the closest and farthest recorded object [m]
         */
        OpenGLFLS(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 sonarUp,
                  GLfloat horizontalFOVDeg, GLfloat verticalFOVDeg, GLuint numOfBeams, GLuint numOfBins, glm::vec2 range_);
        
        //! A destructor.
        ~OpenGLFLS();
        
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
        
        //! A method to set a pointer to a sonar sensor.
        /*!
         \param son a pointer to a sonar sensor
         */
        void setSonar(FLS* s);
                 
    private:
        //FLS specific
        FLS* sonar;
        std::vector<SonarView> views;
        GLuint nBeams;
        GLuint nViewBeams;
        GLuint nBins;
        GLuint nBeamSamples;
        glm::vec2 fov;
        //OpenGL
        GLuint outputTex[2];
        GLuint fanDiv;
        GLSLShader* sonarOutputShader;
        GLSLShader* sonarPostprocessShader;
    };
}

#endif
