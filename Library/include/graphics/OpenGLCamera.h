//
//  OpenGLCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
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
    //!
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
    
    //!
    typedef enum {NORMAL, REFLECTED, REFRACTED} SceneComponent;
    
    class GLSLShader;
    class SolidEntity;
    
    //!
    class OpenGLCamera : public OpenGLView
    {
    public:
        OpenGLCamera(GLint originX, GLint originY, GLint width, GLint height, GLfloat horizon, GLuint spp, bool ao);
        virtual ~OpenGLCamera();
        
        virtual void DrawLDR(GLuint destinationFBO);
        void DrawAO(GLfloat intensity);
        void DrawSSR();
        
        virtual glm::mat4 GetViewMatrix() const = 0;
        virtual glm::vec3 GetEyePosition() const = 0;
        virtual glm::vec3 GetLookingDirection() const = 0;
        virtual glm::vec3 GetUpDirection() const = 0;
        virtual ViewType getType() = 0;
        
        void SetupViewport(GLint x, GLint y, GLint width);
        void SetProjection();
        void SetViewTransform();
        glm::vec3 Ray(GLint x, GLint y);
        void SetReflectionViewport();
        void GenerateLinearDepth(int sampleId);
        void GenerateBlurArray();
        void EnterPostprocessing();
        
        void ShowSceneTexture(SceneComponent sc, glm::vec4 rect);
        void ShowLinearDepthTexture(glm::vec4 rect);
        void ShowViewNormalTexture(glm::vec4 rect);
        void ShowDeinterleavedDepthTexture(glm::vec4 rect, GLuint index);
        void ShowDeinterleavedAOTexture(glm::vec4 rect, GLuint index);
        void ShowAmbientOcclusion(glm::vec4 rect);
        
        glm::mat4 GetProjectionMatrix() const;
        glm::mat4 GetInfiniteProjectionMatrix() const;
        GLfloat GetFOVX() const;
        GLfloat GetFOVY() const;
        GLfloat GetNearClip();
        GLfloat GetFarClip();
        
        GLuint getReflectionFBO();
        GLuint getReflectionTexture();
        GLuint getFinalTexture();
        GLuint getAOTexture();
        GLuint getLinearDepthTexture();
        GLuint getPostprocessTexture(unsigned int id);
        bool hasAO();
        
        static void Init();
        static void Destroy();
        
    protected:
        //Multisampled float textures
        GLuint renderColorTex;
        GLuint renderViewNormalTex;
        GLuint renderDepthStencilTex;
        
        //Non-multisampled reflection textures
        GLuint reflectionFBO;
        GLuint reflectionColorTex;
        GLuint reflectionDepthStencilTex;
        
        //Float texture
        GLuint lightMeterFBO;
        GLuint lightMeterTex;
        
        //Postprocessing
        GLuint postprocessFBO;
        GLuint postprocessTex[3];
        int activePostprocessTexture;
        
        GLuint linearDepthFBO;
        GLuint linearDepthTex;
        
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
