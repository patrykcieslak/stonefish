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
//  OpenGLThermalCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 26/05/2024.
//  Copyright (c) 2024 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLThermalCamera__
#define __Stonefish_OpenGLThermalCamera__

#include "graphics/OpenGLView.h"
#include <random>

namespace sf
{
    class GLSLShader;
    class Camera;
    class SolidEntity;
    
    //! A class representing a thermal camera.
    class OpenGLThermalCamera : public OpenGLView
    {
    public:
        //! A constructor.
        /*!
         \param eyePosition the position of the camera eye in world space [m]
         \param direction a unit vector parallel to the camera optical axis
         \param cameraUp a unit vector pointing to the top edge of the image
         \param originX the x coordinate of the view origin in the program window
         \param originY the y coordinate of the view origin in the program window
         \param width the width of the view
         \param height the height of the view
         \param horizontalFovDeg the horizontal field of view of the camera [deg]
         \param tempRange the range of the temperature measured by the camera [degC]
         \param depthRange the range of the rendering distance of the camera [m]
         \param continuousUpdate a flag indicating if the depth camera has to be always updated
         */
        OpenGLThermalCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp,
                          GLint originX, GLint originY, GLint width, GLint height, GLfloat horizontalFOVDeg,
                          glm::vec2 tempRange, glm::vec2 depthRange, bool continuousUpdate);
        
        //! A destructor.
        ~OpenGLThermalCamera();
        
        //! A method that computes simulated thermal data.
        void ComputeOutput();

        //! A method to render the low dynamic range (final) image to the screen.
        /*!
         \param destinationFBO the id of the framebuffer used as the destination for rendering
         \param updated a flag indicating if view content was updated
         */
        void DrawLDR(GLuint destinationFBO, bool updated) override;
        
        //! A method returning the eye position.
        glm::vec3 GetEyePosition() const override;
        
        //! A method returning a unit vector parallel to the optical axis of the camera.
        glm::vec3 GetLookingDirection() const override;
        
        //! A method returning a unit vector pointing to the top edge of the image.
        glm::vec3 GetUpDirection() const override;

        //! A method returning the projection matrix.
        glm::mat4 GetProjectionMatrix() const override;
        
        //! A method returning the view matrix.
        glm::mat4 GetViewMatrix() const override;

        //! A method that returns the near clip plane distance.
        GLfloat GetNearClip() const override;

        //! A method that returns the far clip plane distance.
        GLfloat GetFarClip() const override;

        //! A method that returns the horizontal field of view.
        GLfloat GetFOVX() const override;
        
        //! A method that returns the vertical field of view.
        GLfloat GetFOVY() const override;
        
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
        void UpdateTransform() override;

        //! A method that flags the camera as needing update.
        void Update();
        
        //! A method that informs if the camera needs update.
        bool needsUpdate() override;
        
        //! A method to set a pointer to a camera sensor.
        /*!
         \param cam a pointer to a camera sensor
         \param index the id of the OpenGL thermal camera
         */
        void setCamera(Camera* cam, unsigned int index = 0);
        
        //! A method to set the noise properties of the thermal camera.
        /*!
         \param tempStdDev the standard deviation of the temperature measurement
         */
        void setNoise(GLfloat tempStdDev);

        //! A method to set the range of temperatures represented by the color mapped image.
        /*!
         \param range the range of the temperatures in deg C
        */
        void setDisplayRange(glm::vec2 tempRange);

        //! A method to set the color map used during temperature data visulization.
        /*!
         \param cm the color map to be used
         */
        void setColorMap(ColorMap cm);

        //! A method returning the type of the view.
        ViewType getType() const override;
        
        //! A static method to load shaders.
        static void Init();
        
        //! A static method to destroy shaders.
        static void Destroy();
        
    protected:
        Camera* camera;
        glm::mat4 cameraTransform;
        glm::vec3 eye;
        glm::vec3 dir;
        glm::vec3 up;
        glm::vec3 tempEye;
        glm::vec3 tempDir;
        glm::vec3 tempUp;
        glm::mat4 projection;
        glm::vec2 fov;
        bool _needsUpdate;
        bool newData;
        glm::vec2 depthRange;
        glm::vec2 temperatureRange;
        GLfloat temperatureNoise;
        std::default_random_engine randGen;
        std::uniform_real_distribution<float> randDist;
        glm::vec2 displayRange;
        ColorMap colorMap;

        GLuint renderDepthTex;
        GLuint renderTex[3];
        GLuint displayTex;
        GLuint outputPBO;
        GLuint displayPBO;
        GLuint displayFBO;
        GLuint displayVAO;
        GLuint displayVBO;
        static GLSLShader* thermalVisualizeShader;
        static GLSLShader* thermalOutputShader;
    };
}

#endif
