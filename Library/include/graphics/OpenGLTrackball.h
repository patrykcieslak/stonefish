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
//  OpenGLTrackball.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013-2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLTrackball__
#define __Stonefish_OpenGLTrackball__

#include "graphics/OpenGLCamera.h"

namespace sf
{
    class MovingEntity;

    //! A class implementing a camera that can be rotated as a trackball, with mouse.
    class OpenGLTrackball : public OpenGLCamera
    {
    public:
        //! A constructor.
        /*!
         \param centerPosition the position of the camera orbit center in world space [m]
         \param orbitRadius the radius of the camera orbit [m]
         \param up a unit vector pointing to the top edge of the image
         \param originX the x coordinate of the view origin in the program window [px]
         \param originY the y coordinate of the view origin in the program window [px]
         \param width the width of the view [px]
         \param height the height of the view [px]
         \param horizontalFovDeg the horizontal field of view of the camera [deg]
         \param range the minimum and maximum rendering distance of the camera [m]
         */
        OpenGLTrackball(glm::vec3 centerPosition, GLfloat orbitRadius, glm::vec3 up,
                        GLint originX, GLint originY, GLint width, GLint height,
                        GLfloat horizontalFovDeg, glm::vec2 range);

        //! A destructor.
        ~OpenGLTrackball();
        
        //! A method to apply a rotation to the trackball.
        /*!
         \param rot a quaternion designating rotation
         */
        void Rotate(glm::quat rot);
        
        //! A method to move the center of the trackball orbit.
        /*!
         \param step a position step [m]
         */
        void MoveCenter(glm::vec3 step);
        
        //! A method to glue the trackball to a moving body.
        /*!
         \param ent a pointer to a moving body
         */
        void GlueToMoving(MovingEntity* ent);
        
        //! A method saving the new centre for update.
        void UpdateCenterPos();

        //! A method used to update the trasformation of the trackball.
        void UpdateTransform();
        
        //! A method servicing the mouse down event.
        /*!
         \param x the x coordinate of the mouse pointer
         \param y the y coordinate of the mouse pointer
         \param translate a flag to decide between translate and rotate
         */
        void MouseDown(GLfloat x, GLfloat y, bool translate);
        
        //! A method servicing the mouse move event.
        /*!
         \param x the x coordinate of the mouse pointer
         \param y the y coordinate of the mouse pointer
         */
        void MouseMove(GLfloat x, GLfloat y);
        
        //! A method servicing the mouse up event.
        void MouseUp();
        
        //! A method servicing the mouse scroll event.
        /*!
         \param s amount of scrolling
         */
        void MouseScroll(GLfloat s);

        //! A method drawing a selection outline.
        /*!
         \param r a reference to a vector of renderable objects
         */
        void DrawSelection(const std::vector<Renderable>& r, GLuint destinationFBO);
        
        //! A method returning the view matrix.
        glm::mat4 GetViewMatrix() const;
        
        //! A method returning the eye position in the world frame.
        glm::vec3 GetEyePosition() const;
        
        //! A method returning a unit vector parallel to the optical axis of the camera.
        glm::vec3 GetLookingDirection() const;
        
        //! A method returning a unit vector pointing to the top edge of the image.
        glm::vec3 GetUpDirection() const;
        
        //! A method returning the type of the view.
        ViewType getType();
        
        //! A method that informs if the camera needs update.
        bool needsUpdate();
        
    private:
        GLfloat calculateZ(GLfloat x, GLfloat y);
        
        MovingEntity* holdingEntity;
        
        glm::vec3 tempCenter;
        glm::mat4 trackballTransform;
        glm::quat rotation;
        glm::vec3 center;
        glm::vec3 up;
        GLfloat radius;
        
        //Mouse interaction
        GLfloat x_start, y_start, z_start;
        glm::quat rotation_start;
        glm::vec3 translation_start;
        bool dragging;
        bool transMode;

        //Shaders
        GLSLShader* outlineShader[2];
    };
}

#endif
