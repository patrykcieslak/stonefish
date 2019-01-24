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
    typedef enum {CAMERA, TRACKBALL, DEPTH_CAMERA} ViewType;
    
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
        
        //! A method that checks if the view needs to be updated.
        virtual bool needsUpdate() = 0;
        
        //! A method returning the type of the view.
        virtual ViewType getType() = 0;
        
        //! A method that sets the current viewport.
        void SetViewport();
        
        //! A method that returns the viewport origin and size.
        GLint* GetViewport() const;
        
        //! A method returning the rendering framebuffer of the view.
        GLuint getRenderFBO();
        
        //! A method to set if the view is enabled.
        /*!
         \param en a flag that says if the view should be enabled
         */
        void setEnabled(bool en);
        
        //! A method returning a flag saying if the view is enabled.
        bool isEnabled();
        
    protected:
        GLint originX;
        GLint originY;
        GLint viewportWidth;
        GLint viewportHeight;
        GLuint renderFBO;
        bool enabled;
    };
}
    
#endif
