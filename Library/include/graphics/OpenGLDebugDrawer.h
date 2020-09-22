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
//  OpenGLDebugDrawer.h
//  Stonefish
//
//  Created by Patryk Cieslak on 28/06/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLDebugDrawer__
#define __Stonefish_OpenGLDebugDrawer__

#include "LinearMath/btIDebugDraw.h"
#include "StonefishCommon.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    //! A class that implements a debug drawer for the Bullet Physics engine.
    class OpenGLDebugDrawer : public btIDebugDraw
    {
    public:
        //! A constructor.
        /*!
         \param debugMode the debug display mode
         */
        OpenGLDebugDrawer(int debugMode);
        
        //! A method to draw a line.
        /*!
         \param from the start of the line
         \param to the end of the line
         \param color the color of the line
         */
        void drawLine(const Vector3& from,const Vector3& to,const Vector3& color);
        
        //! A method to draw a line.
        /*!
         \param from the start of the line
         \param to the end of the line
         \param fromColor color at the start
         \param toColor color at the end
         */
        void drawLine(const Vector3& from,const Vector3& to, const Vector3& fromColor, const Vector3& toColor);
        
        //! A method to draw a contact point.
        /*!
         \param PointOnB point on the body B
         \param normalOnB normal on body B
         \param distance
         \param lifeTime
         \param color the color of the contact point
         */
        void drawContactPoint(const Vector3& PointOnB, const Vector3& normalOnB, Scalar distance, int lifeTime, const Vector3& color);
        
        //! A method to drawe a 3d text.
        /*!
         \param location the location of the text in space
         \param textString a pointer to the string
         */
        void draw3dText(const Vector3& location, const char* textString);
        
        //! A method to report an error or warning.
        /*!
         \param warningString a pointer to the string
         */
        void reportErrorWarning(const char* warningString);
        
        //! A method implementing the rendering of all debug info.
        void Render();
        
        //! A method to set the debug display mode.
        /*!
         \param debugMode the debug display mode
         */
        void setDebugMode(int debugMode);
        
        //! A method to get the current debug mode.
        int	getDebugMode() const;
        
    private:
        int mode;
        std::vector<glm::vec3> lineVertices;
    };
}

#endif
