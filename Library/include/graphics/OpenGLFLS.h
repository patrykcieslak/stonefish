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

#include "graphics/OpenGLView.h"
#include <random>

namespace sf
{
    class GLSLShader;
    class FLS;
    class SolidEntity;
    
    //! A class representing a forward looking sonar (FLS).
    class OpenGLFLS : public OpenGLView
    {
    public:
        //! A constructor.
        /*!
         \param eyePosition the position of the sonar eye in world space [m]
         \param direction a unit vector parallel to the sonar central axis
         \param sonarUp a unit vector perpendicular to the sonar plane
         \param originX the x coordinate of the view origin in the program window [px]
         \param originY the y coordinate of the view origin in the program window [px]
         \param horizontalFovDeg the horizontal field of view of the sonar [deg]
         \param verticalFovDeg the vertical field of view of the sonar [deg]
         \param numOfBeams the number of sonar beams
         \param beamHPix the number of pixels used to represent horizontal beam width [px]
         \param beamVPix the number of pixels used to represent vertical beam width [px]
         \param minRange the distance to the closest recorded object [m]
         \param maxRange the distance to the farthest recorded object [m]
         \param numOfBins the number of sonar range bins
         */
        OpenGLFLS(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 sonarUp,
                  GLint originX, GLint originY, GLfloat horizontalFOVDeg, GLfloat verticalFOVDeg, GLuint numOfBeams, 
                  GLint beamHPix, GLint beamVPix, GLfloat minRange, GLfloat maxRange, GLuint numOfBins);
        
        //! A destructor.
        ~OpenGLFLS();
        
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
        void setSonar(FLS* s);
        
        //! A method returning the type of the view.
        ViewType getType();
        
        //! A static method to load shaders.
        static void Init();
        
        //! A static method to destroy shaders.
        static void Destroy();
        
    protected:
        FLS* sonar;
        glm::mat4 sonarTransform;
        glm::vec3 eye;
        glm::vec3 dir;
        glm::vec3 up;
        glm::vec3 tempEye;
        glm::vec3 tempDir;
        glm::vec3 tempUp;
        glm::mat4 projection;
        bool _needsUpdate;
        bool update;
        GLuint inputWidth;
        GLuint inputHeight;
        GLuint nBeams;
        GLuint nBins;
        glm::vec2 fov;
        glm::vec2 range;
        std::default_random_engine randGen;
        std::uniform_real_distribution<float> randDist;
        GLuint inputRangeIntensityTex;
        GLuint inputDepthRBO;
        GLuint outputTex;
        GLuint outputFBO;
        GLuint displayTex;
        GLuint displayFBO;
        GLuint fanVAO;
        GLuint fanBuf;
        GLuint fanDiv;
        static GLSLShader* sonarInputShader;
        static GLSLShader* sonarOutputShader;
        static GLSLShader* sonarVisualizeShader;
    };
}

#endif
