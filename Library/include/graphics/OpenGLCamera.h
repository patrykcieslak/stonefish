//
//  OpenGLCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013-2019 Patryk Cieslak. All rights reserved.
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
    class SolidEntity;
    
    //! An abstract class implementing a camera view.
    class OpenGLCamera : public OpenGLView
    {
    public:
        //! A constructor.
        /*!
         \param originX the x coordinate of the view origin in the program window
         \param originY the y coordinate of the view origin in the program window
         \param width the width of the view
         \param height the height of the view
         \param horizon the distance to the far plane of the camera [m]
         \param spp number of samples used when rendering (>1 means multisampling)
         \param ao a flag to set if the ambient occlusion should be rendered
         */
        OpenGLCamera(GLint originX, GLint originY, GLint width, GLint height, GLfloat horizon, GLuint spp, bool ao);
        
        //! A destructor.
        virtual ~OpenGLCamera();
        
        //! A method to render the low dynamic range (final) image to the screen.
        /*!
         \param destinationFBO the id of the framebuffer used as the destination for rendering
         */
        virtual void DrawLDR(GLuint destinationFBO);
        
        //! A method drawing the ambient occlusion effect.
        /*!
         \param intensity a factor defining how much of AO should be applied
         */
        void DrawAO(GLfloat intensity);
        
        //! A method drawing the screen-space refflections effect.
        void DrawSSR();
        
        //! A method to set current projection matrix.
        void SetProjection();
        
        //! A method to set current view matrix.
        void SetViewTransform();
        
        //! A method to generate a ray in world space (picking).
        /*!
         \param x the x coordinate in window frame
         \param y the y coordinate in window frame
         \return a unit vector constituting the ray
         */
        glm::vec3 Ray(GLint x, GLint y);
        
        //! A method to generate the linearised depth textures.
        /*!
         \param sampleId the index of the sample to read (important for multisampled rendering buffers)
         \param frontFace a flag defining if the depth should be generated for front or back faces
         */
        void GenerateLinearDepth(int sampleId, bool frontFace);
         
        //! A method which blits the rendering buffer to screen for postprocessing.
        void EnterPostprocessing();
        
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
        
        //! A method to show the view/normal texture.
        /*!
         \param rect the rectangle in which to render the texture on screen
         */
        void ShowViewNormalTexture(glm::vec4 rect);
        
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
        
        //! A method that returns the infinite projection matrix.
        glm::mat4 GetInfiniteProjectionMatrix() const;
        
        //! A method that returns the horizontal field of view.
        GLfloat GetFOVX() const;
        
        //! A method that returns the vertical field of view.
        GLfloat GetFOVY() const;
        
        //! A method that returns the near clip plane distance.
        GLfloat GetNearClip();
        
        //! A method that returns the far clip plane distance.
        GLfloat GetFarClip();
        
        //! A method to set the exposure compensation factor.
        /*!
         \param ec exposure compensation factor
         */
        void setExposureCompensation(GLfloat ec);
        
        //! A method returning the exposure compensation factor.
        GLfloat getExposureCompensation();
        
        //! A method returning the id of the color texture.
        GLuint getColorTexture();
        
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
        
        //! A method returning the id of a postprocessing texture
        /*!
         \param index the id of the texture in the list
         \return OpenGL id of the texture
         */
        GLuint getPostprocessTexture(unsigned int index);
        
        //! A method informing if view is using ambient occlusion.
        bool hasAO();
        
        //! A static method to load common data.
        static void Init();
        
        //! A static method to destroy common data.
        static void Destroy();
        
    protected:
        //Multisampled float textures
        GLuint renderColorTex;
        GLuint renderViewNormalTex;
        GLuint renderDepthStencilTex;
        
        //Float texture
        GLuint lightMeterFBO;
        GLuint lightMeterTex;
        GLfloat exposureComp;
        
        //Postprocessing
        GLuint postprocessFBO;
        GLuint postprocessTex[2];
        GLuint postprocessStencilTex;
        int activePostprocessTexture;
        
        GLuint linearDepthFBO;
        GLuint linearDepthTex[2];
        
        //HBAO Cache-aware (NVIDIA designworks)
        GLuint aoBlurTex;
        GLuint aoResultTex;
        GLuint aoDepthArrayTex;
        GLuint aoResultArrayTex;
        GLuint aoDepthViewTex[HBAO_RANDOM_ELEMENTS];
        GLuint aoFinalFBO;
        GLuint aoDeinterleaveFBO;
        GLuint aoCalcFBO;
        GLuint aoDataUBO;
        AOData aoData;
        
        //Data
        GLuint aoFactor;
        GLuint samples;
        GLfloat fovx;
        GLfloat near;
        GLfloat far;
        glm::mat4 projection;
        
        //Shaders
        static GLSLShader** depthAwareBlurShader;
        static GLSLShader* lightMeterShader;
        static GLSLShader* tonemapShader;
        static GLSLShader** depthLinearizeShader; //Two shaders -> no msaa/msaa
        static GLSLShader* aoDeinterleaveShader;
        static GLSLShader** aoCalcShader;         //Two shaders -> no msaa/msaa
        static GLSLShader* aoReinterleaveShader;
        static GLSLShader** aoBlurShader;		  //Two shaders -> first and second pass
        static GLSLShader** ssrShader;            //Two shaders -> no msaa/msaa
    };
}

#endif
