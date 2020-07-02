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
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLSSS__
#define __Stonefish_OpenGLSSS__

#include "graphics/OpenGLView.h"
#include <random>

namespace sf
{
    class GLSLShader;
    class SSS;
    class SolidEntity;

    //! A class representing a side-scan sonar (SSS).
    class OpenGLSSS : public OpenGLView
    {
    public:
        //! A constructor.
        /*!
         \param centerPosition the position of the sonar center in world space [m]
         \param direction a unit vector parallel to the sonar central axis
         \param forward a vector aligned with the sonar track
         \param originX the x coordinate of the view origin in the program window [px]
         \param originY the y coordinate of the view origin in the program window [px]
         \param verticalBeamWidthDeg the field of view of the single sonar transducer [deg]
         \param horizontalBeamWidthDeg the width of the sonar beam along the track [deg]
         \param numOfBins the number of sonar range bins
         \param numOfLines the length of the waterfall display memory
         \param verticalTiltDeg the angle between the horizon and the transducer axis [deg]
         \param range_ the distance to the closest and farthest recorded object [m]
         \param continuousUpdate a flag indicating if the sonar should be always updated
         */
        OpenGLSSS(glm::vec3 centerPosition, glm::vec3 direction, glm::vec3 forward,
                  GLint originX, GLint originY, GLfloat verticalBeamWidthDeg, GLfloat horizontalBeamWidthDeg, 
                  GLuint numOfBins, GLuint numOfLines, GLfloat verticalTiltDeg, glm::vec2 range_, bool continuousUpdate);
        
        //! A destructor.
        ~OpenGLSSS();
        
        //! A method that computes simulated sonar data.
        void ComputeOutput(std::vector<Renderable>& objects);
        
        //! A method to render the low dynamic range (final) image to the screen.
        /*!
         \param destinationFBO the id of the framebuffer used as the destination for rendering
         */
        void DrawLDR(GLuint destinationFBO);
        
        //! A method returning the eye position.
        glm::vec3 GetEyePosition() const;
        
        //! A method returning a unit vector parallel to the optical axis of the camera.
        glm::vec3 GetLookingDirection() const;
        
        //! A method returning a unit vector pointing to the top edge of the image.
        glm::vec3 GetUpDirection() const;

        //! A method returning the projection matrix.
        glm::mat4 GetProjectionMatrix() const;
        
        //! A method returning the view matrix.
        glm::mat4 GetViewMatrix() const;

        //! A method that returns the far clip plane distance.
        GLfloat GetFarClip() const;
        
        //! A method that sets up the sonar.
        void SetupSonar();
        
        //! A method used to set up the sonar.
        /*!
         \param eye the position of the sonar [m]
         \param dir a unit vector parallel to the sonar central axis
         \param up a unit vector perpendicular to the sonar plane
         */
        void SetupSonar(glm::vec3 eye, glm::vec3 dir, glm::vec3 up);
        
        //! A method that updates sonar world transform.
        void UpdateTransform();

        //! A method that flags the sonar as needing update.
        void Update();
        
        //! A method that informs if the sonar needs update.
        bool needsUpdate();
        
        //! A method to set a pointer to a sonar sensor.
        /*!
         \param son a pointer to a sonar sensor
         */
        void setSonar(SSS* s);
        
        //! A method to set the color map used during sonar data visulaization.
        /*!
         \param cm name of the color map to be used
         */
        void setColorMap(ColorMap cm);
        
        //! A method returning the type of the view.
        ViewType getType();
        
        //! A static method to load shaders.
        static void Init();
        
        //! A static method to destroy shaders.
        static void Destroy();
        
    protected:
        SSS* sonar;
        glm::mat4 sonarTransform;
        glm::vec3 center;
        glm::vec3 dir;
        glm::vec3 forward;
        glm::vec3 tempCenter;
        glm::vec3 tempDir;
        glm::vec3 tempForward;
        glm::mat4 projection;
        bool _needsUpdate;
        bool update;
        bool newData;

        GLfloat tilt;
        glm::uvec2 nBeamSamples;
        glm::vec2 fov;
        glm::vec2 range;
        glm::mat4 views[2];
        std::default_random_engine randGen;
        std::uniform_real_distribution<float> randDist;
        ColorMap cMap;
        
        GLuint inputRangeIntensityTex;
        GLuint inputDepthRBO;
        GLuint outputTex[3];
        GLuint pingpong;
        GLuint outputPBO;
        GLuint displayTex;
        GLuint displayFBO;
        GLuint displayPBO;
        GLSLShader* sonarOutputShader[2];

        static GLSLShader* sonarInputShader[2];
        static GLSLShader* sonarShiftShader;
        static GLSLShader* sonarVisualizeShader;
    };
}

#endif
