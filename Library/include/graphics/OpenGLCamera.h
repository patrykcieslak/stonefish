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
//  OpenGLCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013-2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLCamera__
#define __Stonefish_OpenGLCamera__

#include "graphics/OpenGLView.h"

#define SCENE_ATTACHMENT        GL_COLOR_ATTACHMENT1
#define FINAL_ATTACHMENT        GL_COLOR_ATTACHMENT0
#define AO_RANDOMTEX_SIZE		4
#define HBAO_RANDOM_SIZE		AO_RANDOMTEX_SIZE
#define HBAO_RANDOM_ELEMENTS	(HBAO_RANDOM_SIZE*HBAO_RANDOM_SIZE)
#define NUM_MRT					8
#define MAX_SAMPLES				8

namespace sf
{
    //! A structure holding data for HBAO algorithm.
    struct AOData
    {
        GLfloat RadiusToScreen;
        GLfloat R2;
        GLfloat NegInvR2;
        GLfloat NDotVBias;
        
        glm::vec2 InvFullResolution;
        glm::vec2 InvQuarterResolution;
        
        GLfloat AOMultiplier;
        GLfloat PowExponent;
        glm::vec2 _pad0;
        
        glm::vec4 projInfo;
        glm::vec2 projScale;
        glm::vec2 _pad1;
        //GLint     projOrtho;
        //GLint     _pad1;
        
        glm::vec4 float2Offsets[AO_RANDOMTEX_SIZE*AO_RANDOMTEX_SIZE];
        glm::vec4 jitters[AO_RANDOMTEX_SIZE*AO_RANDOMTEX_SIZE];
    };
 
    class GLSLShader;
    
    //! An abstract class implementing a camera view.
    class OpenGLCamera : public OpenGLView
    {
    public:
        //! A constructor.
        /*!
         \param originX the x coordinate of the view origin in the program window [px]
         \param originY the y coordinate of the view origin in the program window [px]
         \param width the width of the view [px]
         \param height the height of the view [px]
         \param range the minimum and maximum rendering distance of the camera [m]
         */
        OpenGLCamera(GLint originX, GLint originY, GLint width, GLint height, glm::vec2 range);
        
        //! A destructor.
        virtual ~OpenGLCamera();
        
        //! A method to render the low dynamic range (final) image to the screen.
        /*!
         \param destinationFBO the id of the framebuffer used as the destination for rendering
         \param updated a flag indicating if view content was updated
         */
        virtual void DrawLDR(GLuint destinationFBO, bool updated);
        
        //! A method drawing the ambient occlusion effect.
        /*!
         \param intensity a factor defining how much of AO should be applied
         */
        void DrawAO(GLfloat intensity);
        
        //! A method drawing the screen-space refflections effect.
        void DrawSSR();

        //! A method generating a bloom effect.
        void GenerateBloom();

        //! A method drawing the bloom effect.
        void DrawBloom(GLfloat amount);
        
        //! A method to set current projection matrix.
        void SetProjection();
        
        //! A method to set current view matrix.
        void SetViewTransform();

        //! A method used to determine which buffers are drawn during rendering.
        /*!
         \param colorBufferIndex the id of the color buffer to be used for rendering
         \param normalBuffer a flag indicating if normal buffer should be written
         \param clearBuffers a flag indicating if buffers should be cleared
         */
        void SetRenderBuffers(GLuint colorBufferIndex, bool normalBuffer, bool clearBuffers);
        
        //! A method to generate a ray in world space (picking).
        /*!
         \param x the x coordinate in window frame
         \param y the y coordinate in window frame
         \return a unit vector constituting the ray
         */
        glm::vec3 Ray(GLint x, GLint y);
        
        //! A method to generate a linear depth texture from the normal depth buffer.
        /*!
         \param front a flag indicating if the depth should be generated for front or back faces
        */
        void GenerateLinearDepth(bool front);
        
        //! A method to show the color texture.
        /*!
         \param rect the rectangle in which to render the texture on screen
         */
        void ShowSceneTexture(glm::vec4 rect);
        
        //! A method to show the depth texture.
        /*!
         \param rect the rectangle in which to render the texture on screen
         \param frontFace a flag to decide if front or back face depth should be displayed
         */
        void ShowLinearDepthTexture(glm::vec4 rect, bool frontFace);
        
        //! A method to show the view-normal texture.
        /*!
         \param rect the rectangle in which to render the texture on screen
         */
        void ShowViewNormalTexture(glm::vec4 rect);

        //! A method to show the depth-stencil texture.
        /*!
         \param rect the rectangle in which to render the texture on screen
         */
        void ShowDepthStencilTexture(glm::vec4 rect);
        
        //! A method to show the deinterleaved depth texture (HBAO).
        /*!
         \param rect the rectangle in which to render the texture on screen
         \param index the id of the sample
         */
        void ShowDeinterleavedDepthTexture(glm::vec4 rect, GLuint index);
        
        //! A method to show the deinterleaved ambient occlusion texture (HBAO).
        /*!
         \param rect the rectangle in which to render the texture on screen
         \param index the id of the sample
         */
        void ShowDeinterleavedAOTexture(glm::vec4 rect, GLuint index);
        
        //! A method to show the ambient occlusion texture (HBAO).
        /*!
         \param rect the rectangle in which to render the texture on screen
         */
        void ShowAmbientOcclusion(glm::vec4 rect);
        
        //! A method that returns the projection matrix.
        glm::mat4 GetProjectionMatrix() const;
        
        //! A method that returns the horizontal field of view.
        GLfloat GetFOVX() const;
        
        //! A method that returns the vertical field of view.
        GLfloat GetFOVY() const;
        
        //! A method that returns the near clip plane distance.
        GLfloat GetNearClip() const;
        
        //! A method that returns the far clip plane distance.
        GLfloat GetFarClip() const;
        
        //! A method to set the exposure compensation factor.
        /*!
         \param ec exposure compensation factor
         */
        void setExposureCompensation(GLfloat ec);

        //! A method returning the exposure compensation factor.
        GLfloat getExposureCompensation();

        //! A method returning the postprocessing framebuffer of the camera.
        GLuint getPostprocessFBO();

        //! A method returning the quater resolution postprocessing framebuffer of the camera.
        GLuint getQuaterPostprocessFBO();

        //! A method returning the id of the color texture.
        /*!
         \param index the id of the texture in the list
         \return OpenGL id of the texture
         */
        GLuint getColorTexture(unsigned int index);
        
        //! A method returning the id of the final texture.
        GLuint getFinalTexture();
        
        //! A method returning the id of the ambient occlusion texture.
        GLuint getAOTexture();
        
        //! A method returning the id of the linear depth texture.
        /*!
         \param frontFace a flag to decide if front or back face depth is requested
         \return OpenGL id of the texture
         */
        GLuint getLinearDepthTexture(bool frontFace);
        
        //! A method returning the id of a postprocessing texture.
        /*!
         \param index the id of the texture in the list
         \return OpenGL id of the texture
         */
        GLuint getPostprocessTexture(unsigned int index);
        
        //! A method returning the id of a quater resolution postprocessing texture.
        /*!
         \param index the id of the texture in the list
         \return OpenGL id of the texture
         */
        GLuint getQuaterPostprocessTexture(unsigned int index);

        //! A method returning the id of the last used color buffer.
        GLuint getLastActiveColorBuffer();

        //! A method informing if view is using ambient occlusion.
        bool hasAO();
		
		//! A method informing if HDR tone mapping is enabled.
		bool usingToneMapping();
		
		//! A method informing if auto exposure function is enabled.
        bool usingAutoExposure();
		
        //! A static method to load shaders.
        static void Init(const RenderSettings& rSettings);
        
        //! A static method to destroy shaders.
        static void Destroy();
        
    protected:
        //Buffers
        GLuint renderColorTex[2];
        GLuint renderViewNormalTex;
        GLuint renderDepthStencilTex;
        GLuint histogramSSBO;
        GLuint histogramBins;
        glm::vec2 histogramRange;
        GLuint exposureTex;
        GLfloat exposureComp;
        bool autoExposure;
		bool toneMapping;
        bool antiAliasing;
        GLuint lastActiveRenderColorBuffer;
        
        //Postprocessing
        GLuint linearDepthFBO;
        GLuint linearDepthTex[2];
        GLuint postprocessFBO;
        GLuint postprocessTex[2];
        GLuint postprocessStencilTex;
        GLuint quaterPostprocessFBO;
        GLuint quaterPostprocessTex[2];
        
        //HBAO Cache-aware (NVIDIA designworks)
        GLuint aoBlurTex;
        GLuint aoResultTex;
        GLuint aoDepthArrayTex;
        GLuint aoResultArrayTex;
        GLuint aoFinalFBO;
        GLuint aoDeinterleaveFBO;
        GLuint aoCalcFBO;
        GLuint aoDataUBO;
        AOData aoData;
        
        //Data
        GLuint aoFactor;
        GLfloat fovx;
        GLfloat near;
        GLfloat far;
        glm::mat4 projection;
        
        //Shaders
        static GLSLShader** tonemappingShaders;
        static GLSLShader* depthLinearizeShader;
        static GLSLShader* aoDeinterleaveShader;
        static GLSLShader* aoCalcShader;
        static GLSLShader* aoReinterleaveShader;
        static GLSLShader** aoBlurShader;		  //Two shaders -> first and second pass
        static GLSLShader* ssrShader;
        static GLSLShader* fxaaShader;
        static GLSLShader* flipShader;
        static GLSLShader* ssrBlur;
        static GLSLShader* bloomBlur;
    };
}

#endif
