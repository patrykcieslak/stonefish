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
//  OpenGLSonar.h
//  Stonefish
//
//  Created by Patryk Cieslak on 22/07/20.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLSonar__
#define __Stonefish_OpenGLSonar__

#include "graphics/OpenGLView.h"
#include <random>

namespace sf
{
    class GLSLShader;
    
    //! An abstract class representing a sonar view.
    class OpenGLSonar : public OpenGLView
    {
    public:
        //! A constructor.
        /*!
         \param eyePosition the position of the sonar eye in world space [m]
         \param direction a unit vector parallel to the sonar central axis [1]
         \param sonarUp a unit vector perpendicular to the sonar plane [1]
         \param displayResolution the resolution of the sonar display [px]
         \param range_ the distance to the closest and farthest recorded object [m]
         */
        OpenGLSonar(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 sonarUp, glm::uvec2 displayResolution, glm::vec2 range_);
        
        //! A destructor.
        virtual ~OpenGLSonar();
        
        //! A method that computes simulated sonar data.
        virtual void ComputeOutput(std::vector<Renderable>& objects) = 0;
        
        //! A method to render the low dynamic range (final) image to the screen.
        /*!
         \param destinationFBO the id of the framebuffer used as the destination for rendering
         \param updated a flag indicating if view content was updated
         */
        virtual void DrawLDR(GLuint destinationFBO, bool updated) = 0;
        
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
        virtual void UpdateTransform();

        //! A method that flags the sonar as needing update.
        void Update();
        
        //! A method that informs if the sonar needs update.
        bool needsUpdate();
        
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
        //Sonar specific
        glm::mat4 sonarTransform;
        glm::vec3 eye;
        glm::vec3 dir;
        glm::vec3 up;
        glm::vec3 tempEye;
        glm::vec3 tempDir;
        glm::vec3 tempUp;
        glm::mat4 projection;
        glm::vec2 range;
        GLfloat gain;
        std::default_random_engine randGen;
        std::uniform_real_distribution<float> randDist;
        ColorMap cMap;
        bool settingsUpdated;
        bool _needsUpdate;
        bool newData;
        
        //OpenGL
        GLuint inputRangeIntensityTex;
        GLuint inputDepthRBO;
        GLuint outputPBO;
        GLuint displayTex;
        GLuint displayFBO;
        GLuint displayPBO;
        GLuint displayVAO;
        GLuint displayVBO;
        
        static GLSLShader* sonarInputShader[2];
        static GLSLShader* sonarVisualizeShader;
    };
}

#endif
