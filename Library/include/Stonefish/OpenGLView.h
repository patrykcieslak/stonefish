//
//  OpenGLView.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLView__
#define __Stonefish_OpenGLView__

#include "OpenGLPipeline.h"
#include "GLSLShader.h"
#include "SolidEntity.h"

#define SCENE_ATTACHMENT        GL_COLOR_ATTACHMENT1
#define FINAL_ATTACHMENT        GL_COLOR_ATTACHMENT0
#define AO_RANDOMTEX_SIZE		4
#define HBAO_RANDOM_SIZE		AO_RANDOMTEX_SIZE
#define HBAO_RANDOM_ELEMENTS	(HBAO_RANDOM_SIZE*HBAO_RANDOM_SIZE)
#define NUM_MRT					8
#define MAX_SAMPLES				8

typedef struct
{
    GLfloat near;
    GLfloat far;
    GLfloat fov;
    GLfloat ratio;
    glm::vec3 corners[8];
}
ViewFrustum;

typedef struct
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
}
AOData;

typedef enum {CAMERA, TRACKBALL} ViewType;
typedef enum {NORMAL, REFLECTED, REFRACTED} SceneComponent;

class OpenGLView
{
public:
    OpenGLView(GLint originX, GLint originY, GLint width, GLint height, GLfloat horizon, GLuint spp, bool ao);
    virtual ~OpenGLView(void);
    
    virtual glm::mat4 GetViewTransform() const = 0;
    virtual glm::vec3 GetEyePosition() const = 0;
    virtual glm::vec3 GetLookingDirection() const = 0;
    virtual glm::vec3 GetUpDirection() const = 0;
    virtual ViewType getType() = 0;
    
    void SetupViewport(GLint x, GLint y, GLint width);
    void SetViewport();
    void SetProjection();
    void SetViewTransform();
	btVector3 Ray(GLint x, GLint y);

    void Activate();
    void Deactivate();
    
    void DrawAO();
	void DrawHDR(GLuint destinationFBO);
	
	void ShowSceneTexture(SceneComponent sc, glm::vec4 rect);
	void ShowLinearDepthTexture(glm::vec4 rect);
	void ShowViewNormalTexture(glm::vec4 rect);
	void ShowDeinterleavedDepthTexture(glm::vec4 rect, GLuint index);
	void ShowDeinterleavedAOTexture(glm::vec4 rect, GLuint index);
    void ShowAmbientOcclusion(glm::vec4 rect);
	
    GLint* GetViewport() const;
    glm::mat4 GetProjectionMatrix() const;
    glm::mat4 GetViewMatrix() const;
	GLfloat GetFOVX() const;
    GLfloat GetFOVY() const;
    GLfloat GetNearClip();
    GLfloat GetFarClip();
    
    GLuint getRenderFBO();
	GLuint getFinalTexture();
    GLuint getAOTexture();
    bool isActive();
    bool hasAO();
        
    static void Init();
    static void Destroy();
    
protected:
	//Multisampled float textures
    GLuint renderFBO;
    GLuint renderColorTex;
	GLuint renderViewNormalTex;
	GLuint renderDepthStencilTex;
	
	//Float texture
	GLuint lightMeterFBO;
    GLuint lightMeterTex;
	
	//Postprocessing
	GLuint postprocessFBO;
	GLuint postprocessTex[2];
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
    bool active;
	GLint originX;
    GLint originY;
    GLint viewportWidth;
    GLint viewportHeight;
    GLuint aoFactor;
	GLuint samples;
    GLfloat fovx;
    GLfloat near;
    GLfloat far;
    glm::mat4 projection;
    
    //Shaders
    static GLSLShader* lightMeterShader;
    static GLSLShader* tonemapShader;
	static GLSLShader** depthLinearizeShader; //Two shaders -> no msaa/msaa
	static GLSLShader* aoDeinterleaveShader;
	static GLSLShader** aoCalcShader;         //Two shaders -> no msaa/msaa
	static GLSLShader* aoReinterleaveShader;
	static GLSLShader** aoBlurShader;		  //Two shaders -> first and second pass
};

#endif
