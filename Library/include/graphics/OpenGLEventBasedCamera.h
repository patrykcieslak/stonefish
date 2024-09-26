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
//  OpenGLEventBasedCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/03/24.
//  Copyright (c) 2024 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLEventBasedCamera__
#define __Stonefish_OpenGLEventBasedCamera__

#include "graphics/OpenGLCamera.h"
#include <random>

namespace sf
{
    class EventBasedCamera;
 
    //! A class implementing a real camera in OpenGL.
    class OpenGLEventBasedCamera : public OpenGLCamera
    {
    public:
        //! A constructor.
        /*!
         \param eyePosition the position of the camera eye in world space [m]
         \param direction a unit vector parallel to the camera optical axis
         \param cameraUp a unit vector pointing to the top edge of the image
         \param originX the x coordinate of the view origin in the program window [px]
         \param originY the y coordinate of the view origin in the program window [px]
         \param width the width of the view [px]
         \param height the height of the view [px]
         \param horizontalFovDeg the horizontal field of view of the camera [deg]
         \param range the minimum and maximum rendering distance of the camera [m]
         \param C contrast threshold
         \param Tr refractory time [ns]
         \param continuousUpdate a flag indicating if this camera has to be always updated
         */
        OpenGLEventBasedCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp,
                         GLint originX, GLint originY, GLint width, GLint height, GLfloat horizontalFovDeg, 
                         glm::vec2 range, glm::vec2 C, uint32_t Tr, bool continuousUpdate);
        
        //! A destructor.
        ~OpenGLEventBasedCamera();
        
        //! A method that computes simulated event output.
        /*!
         \param dt time that passed since last frame [s]
        */
        void ComputeOutput(double dt);

        //! A method to render the low dynamic range (final) image to the screen.
        /*!
         \param destinationFBO the id of the framebuffer used as the destination for rendering
         \param updated a flag indicating if view content was updated
         */
        void DrawLDR(GLuint destinationFBO, bool updated);
        
        //! A method that sets up the camera.
        void SetupCamera();
        
        //! A method used to set up the camera.
        /*!
         \param eye the position of the camera [m]
         \param dir a unit vector parallel to the camera optical axis
         \param up a unit vector pointing to the top edge of the image
         */
        void SetupCamera(glm::vec3 eye, glm::vec3 dir, glm::vec3 up);
        
        //! A method that updates camera world transform.
        void UpdateTransform();
        
        //! A method that flags the camera as needing update.
        void Update();
        
        //! A method returning the view matrix.
        glm::mat4 GetViewMatrix() const;
        
        //! A method returning the eye position.
        glm::vec3 GetEyePosition() const;
        
        //! A method returning a unit vector parallel to the optical axis of the camera.
        glm::vec3 GetLookingDirection() const;
        
        //! A method returning a unit vector pointing to the top edge of the image.
        glm::vec3 GetUpDirection() const;
        
        //! A method returning the type of the view.
        ViewType getType() const override;
        
        //! A method to set a pointer to a camera sensor.
        /*!
         \param cam a pointer to a camera sensor
         */
        void setCamera(EventBasedCamera* cam);

        //! A method to set the noise properties of the event based camera.
        /*!
         \param sigmaC the standard deviation of the contrast threshold
         */
        void setNoise(glm::vec2 sigmaC);
         
        //! A method that informs if the camera needs update.
        bool needsUpdate();

        //! A static method to load shaders.
        static void Init();
        
        //! A static method to destroy shaders.
        static void Destroy();
        
    private:
        EventBasedCamera* camera;
        GLuint outputPBO;
        GLuint renderLogLumTex;   // Last logarithm luminance image
        GLuint renderEventTex[3]; // Events, last event timestamps, crossings
        GLuint renderEventCounter; // Atomic event counter
        GLuint displayFBO;
        GLuint displayTex;
        std::default_random_engine randGen;
        std::uniform_real_distribution<float> randDist;
        glm::vec2 C_;
        glm::vec2 sigmaC_;
        uint32_t Tr_;
        uint32_t maxNumEvents;
        
        glm::mat4 cameraTransform;
        glm::vec3 eye;
        glm::vec3 dir;
        glm::vec3 up;
        glm::vec3 tempEye;
        glm::vec3 tempDir;
        glm::vec3 tempUp;
        bool _needsUpdate;
        bool newData;
        bool initialized;

        static GLSLShader** eventOutputShaders;
        static GLSLShader* eventVisualizeShader;
    };
}

#endif
