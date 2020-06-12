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
//  OpenGLView.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/05/18.
//  Copyright (c) 2018-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLView__
#define __Stonefish_OpenGLView__

#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    //! An enum defining types of views.
    typedef enum {CAMERA, TRACKBALL, DEPTH_CAMERA, SONAR} ViewType;

    #pragma pack(1)
    struct ViewUBO
    {
        glm::mat4 VP;
        glm::vec4 frustum[6];
        glm::vec3 eye;
        GLfloat pad;
    };
    #pragma pack(0)
    
    //! An abstract class representing an OpenGL view.
    class OpenGLView
    {
    public:
        //! A constructor.
        /*!
         \param originX the x coordinate of the view origin in the program window
         \param originY the y coordinate of the view origin in the program window
         \param width the width of the view
         \param height the height of the view
         */
        OpenGLView(GLint originX, GLint originY, GLint width, GLint height);
        
        //! A destructor.
        virtual ~OpenGLView();
        
        //! A method to render the low dynamic range (final) image to the screen.
        /*!
         \param destinationFBO the id of the framebuffer used as the destination for rendering
         */
        virtual void DrawLDR(GLuint destinationFBO) = 0;

        //! A method that returns eye position.
        virtual glm::vec3 GetEyePosition() const = 0;
        
        //! A method returning a unit vector parallel to the optical axis of the camera.
        virtual glm::vec3 GetLookingDirection() const = 0;
        
        //! A method returning a unit vector pointing to the top edge of the image.
        virtual glm::vec3 GetUpDirection() const = 0;
        
        //! A method that returns the projection matrix.
        virtual glm::mat4 GetProjectionMatrix() const = 0;
        
        //! A method that returns the view matrix.
        virtual glm::mat4 GetViewMatrix() const = 0;

        //! A method that returns the far clip plane distance.
        virtual GLfloat GetFarClip() const = 0;

        //! A method that return logarithmic depth constant.
        GLfloat GetLogDepthConstant() const;
        
        //! A method that checks if the view needs to be updated.
        virtual bool needsUpdate() = 0;
        
        //! A method returning the type of the view.
        virtual ViewType getType() = 0;
        
        //! A method that sets the current viewport.
        void SetViewport();
        
        //! A method that returns the viewport origin and size.
        GLint* GetViewport() const;
        
        //! A method returning the rendering framebuffer of the view.
        GLuint getRenderFBO() const;
        
        //! A method returning a pointer to the view UBO data.
        const ViewUBO* getViewUBOData() const;
        
        //! A method to set if the view is enabled.
        /*!
         \param en a flag that says if the view should be enabled
         */
        void setEnabled(bool en);
        
        //! A method returning a flag saying if the view is enabled.
        bool isEnabled();

        //! A method saying if the view works in continuous update mode.
        bool isContinuous();

        //! A method extracting frustium planes from the view-projection matrix.
        /*!
         \param frustum a pointer to the 6 frustum planes
         \param VP the view-projection matrix
         */
        static void ExtractFrustumFromVP(glm::vec4 frustum[6], const glm::mat4& VP);
        
    protected:
        GLint originX;
        GLint originY;
        GLint viewportWidth;
        GLint viewportHeight;
        GLuint renderFBO;
        bool enabled;
        bool continuous;
        ViewUBO viewUBOData;
    };
}
    
#endif
