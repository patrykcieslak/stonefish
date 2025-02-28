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
//  OpenGLRealCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/12/12.
//  Copyright (c) 2012-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLRealCamera__
#define __Stonefish_OpenGLRealCamera__

#include "graphics/OpenGLCamera.h"

namespace sf
{
    class ColorCamera;
 
    //! A class implementing a real camera in OpenGL.
    class OpenGLRealCamera : public OpenGLCamera
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
         \param continuousUpdate a flag indicating if this camera has to be always updated
         */
        OpenGLRealCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp,
                         GLint originX, GLint originY, GLint width, GLint height,
                         GLfloat horizontalFovDeg, glm::vec2 range, bool continuousUpdate);
        
        //! A destructor.
        ~OpenGLRealCamera();
        
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
        ViewType getType();
        
        //! A method to set a pointer to a camera sensor.
        /*!
         \param cam a pointer to a camera sensor
         */
        void setCamera(ColorCamera* cam);
         
        //! A method that informs if the camera needs update.
        bool needsUpdate();
        
    private:
        ColorCamera* camera;
        GLuint cameraFBO;
        GLuint cameraColorTex[2];
        GLuint cameraPBO;
        
        glm::mat4 cameraTransform;
        glm::vec3 eye;
        glm::vec3 dir;
        glm::vec3 up;
        glm::vec3 tempEye;
        glm::vec3 tempDir;
        glm::vec3 tempUp;
        bool _needsUpdate;
        bool newData;
    };
}

#endif
