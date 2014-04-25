//
//  OpenGLSun.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 20/04/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "OpenGLSun.h"
#include "OpenGLUtil.h"
#include "OpenGLSky.h"
#include "OpenGLLight.h"
#include "OpenGLSolids.h"
#include "SimulationApp.h"

OpenGLView* OpenGLSun::activeView = NULL;
GLhandleARB OpenGLSun::sunShader = 0;
GLhandleARB OpenGLSun::shadowmapShader = 0;
GLint OpenGLSun::diffuseTextureUnit = 0;
GLint OpenGLSun::normalTextureUnit = 0;
GLint OpenGLSun::positionTextureUnit = 0;
GLint OpenGLSun::shadowTextureUnit = 0;
GLint OpenGLSun::uniColor = -1;
GLint OpenGLSun::uniDiffuse = -1;
GLint OpenGLSun::uniNormal = -1;
GLint OpenGLSun::uniPosition = -1;
GLint OpenGLSun::uniLightDir = -1;
GLint OpenGLSun::uniShadowmapLayer = -1;
GLint OpenGLSun::uniShadowmapArray = -1;
GLint OpenGLSun::uniShadowArray = -1;
GLint OpenGLSun::uniFrustum = -1;
GLint OpenGLSun::uniLightClipSpace[] = {-1,-1,-1,-1};
GLfloat OpenGLSun::sunElevation = 45.f;
GLfloat OpenGLSun::sunOrientation = 0.f;
GLuint OpenGLSun::shadowmapArray = 0;
GLuint OpenGLSun::shadowmapSplits = 4;
GLuint OpenGLSun::shadowmapSize = 1024;
GLuint OpenGLSun::shadowFBO = 0;
btVector3 OpenGLSun::sunDirection = btVector3();
glm::mat4x4 OpenGLSun::sunModelview = glm::mat4x4(0);
ViewFrustum* OpenGLSun::frustum = NULL;
glm::mat4x4* OpenGLSun::shadowCPM = NULL;

void OpenGLSun::Init()
{
    //load shaders
    GLint compiled = 0;
    GLhandleARB vs = LoadShader(GL_VERTEX_SHADER, "saq.vert", &compiled);
    GLhandleARB fs = LoadShader(GL_FRAGMENT_SHADER, "deferredSun.frag", &compiled);
    sunShader = CreateProgramObject(vs, fs);
    LinkProgram(sunShader, &compiled);
    
    glUseProgramObjectARB(sunShader);
    uniDiffuse = glGetUniformLocationARB(sunShader, "texDiffuse");
    uniPosition = glGetUniformLocationARB(sunShader, "texPosition");
    uniNormal = glGetUniformLocationARB(sunShader, "texNormal");
    uniLightDir = glGetUniformLocationARB(sunShader, "lightDirection");
    uniColor = glGetUniformLocationARB(sunShader, "lightColor");
    uniShadowArray = glGetUniformLocationARB(sunShader, "texShadowArray");
    uniFrustum = glGetUniformLocationARB(sunShader, "frustumFar");
    uniLightClipSpace[0] = glGetUniformLocationARB(sunShader, "lightClipSpace[0]");
    uniLightClipSpace[1] = glGetUniformLocationARB(sunShader, "lightClipSpace[1]");
    uniLightClipSpace[2] = glGetUniformLocationARB(sunShader, "lightClipSpace[2]");
    uniLightClipSpace[3] = glGetUniformLocationARB(sunShader, "lightClipSpace[3]");
    glUseProgramObjectARB(0);
    
    fs = LoadShader(GL_FRAGMENT_SHADER, "cascadedShadowMap.frag", &compiled);
    shadowmapShader = CreateProgramObject(vs, fs);
    LinkProgram(shadowmapShader, &compiled);
    
    glUseProgramObjectARB(shadowmapShader);
    uniShadowmapArray = glGetUniformLocationARB(shadowmapShader, "shadowmapArray");
    uniShadowmapLayer = glGetUniformLocationARB(shadowmapShader, "shadowmapLayer");
    glUseProgramObjectARB(0);
    
    //Create shadowmap texture array
    glGenTextures(1, &shadowmapArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, shadowmapArray);
	glTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, GL_DEPTH_COMPONENT24, shadowmapSize, shadowmapSize, shadowmapSplits, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_NEAREST - may be needed for more than 8 bits
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, 0);
    
    //Create shadowmap framebuffer
    glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowmapArray, 0, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
        printf("FBO initialization failed.\n");
    
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    frustum = new ViewFrustum[shadowmapSplits];
    shadowCPM = new glm::mat4x4[shadowmapSplits];
}

void OpenGLSun::Destroy()
{
    glDeleteTextures(1, &shadowmapArray);
    glDeleteFramebuffers(1, &shadowFBO);
    glDeleteObjectARB(&sunShader);
    glDeleteObjectARB(&shadowmapShader);
}

//Computes the near and far distances for every frustum slice in camera eye space
void OpenGLSun::UpdateSplitDist(GLfloat nd, GLfloat fd)
{
	GLfloat lambda = 0.8f;
	GLfloat ratio = fd/nd;
	frustum[0].near = nd;
    
	for(int i=1; i < shadowmapSplits; i++)
	{
		GLfloat si = i / (GLfloat)shadowmapSplits;
        
		frustum[i].near = lambda*(nd*powf(ratio, si)) + (1.f-lambda)*(nd + (fd - nd)*si);
		frustum[i-1].far = frustum[i].near * 1.005f;
	}
    
	frustum[shadowmapSplits-1].far = fd;
    
    //for(int i=0; i<shadowmapSplits; i++) printf("Frustum%d Near: %f Far: %f\n", i, frustum[i].near, frustum[i].far);
}

//Computes the 8 corner points of the current view frustum
void OpenGLSun::UpdateFrustumCorners(ViewFrustum &f, glm::vec3 center, glm::vec3 dir, glm::vec3 up)
{
    glm::vec3 right = glm::cross(dir, up);
    right = glm::normalize(right);
	//up = glm::normalize(glm::cross(right, dir));
    
    glm::vec3 fc = center + dir * f.far;
    glm::vec3 nc = center + dir * f.near;
    
	// these heights and widths are half the heights and widths of
	// the near and far plane rectangles
	GLfloat nearHeight = tan(f.fov/2.0f) * f.near;
	GLfloat nearWidth = nearHeight * f.ratio;
	GLfloat farHeight = tan(f.fov/2.0f) * f.far;
	GLfloat farWidth = farHeight * f.ratio;
    
	f.corners[0] = nc - up * nearHeight - right * nearWidth;
	f.corners[1] = nc + up * nearHeight - right * nearWidth;
	f.corners[2] = nc + up * nearHeight + right * nearWidth;
	f.corners[3] = nc - up * nearHeight + right * nearWidth;
	f.corners[4] = fc - up * farHeight - right * farWidth;
	f.corners[5] = fc + up * farHeight - right * farWidth;
	f.corners[6] = fc + up * farHeight + right * farWidth;
	f.corners[7] = fc - up * farHeight + right * farWidth;
}

// this function builds a projection matrix for rendering from the shadow's POV.
// First, it computes the appropriate z-range and sets an orthogonal projection.
// Then, it translates and scales it, so that it exactly captures the bounding box
// of the current frustum slice
glm::mat4 OpenGLSun::BuildCropProjMatrix(ViewFrustum &f)
{
	GLfloat maxX = -1000.0f;
    GLfloat maxY = -1000.0f;
	GLfloat maxZ;
    GLfloat minX =  1000.0f;
    GLfloat minY =  1000.0f;
	GLfloat minZ;
    glm::vec4 transf;
	
	//Find the z-range of the current frustum as seen from the light in order to increase precision
	glm::mat4 shad_mv = sunModelview;
    
	//Note: only the z-component is needed and thus the multiplication can be simplified
	//transf.z = shad_modelview[2] * f.point[0].x + shad_modelview[6] * f.point[0].y + shad_modelview[10] * f.point[0].z + shad_modelview[14]
	transf = shad_mv * glm::vec4(f.corners[0], 1.0f);
	minZ = transf.z;
	maxZ = transf.z;
    
	for(int i = 1; i <8 ; i++)
	{
		transf = shad_mv * glm::vec4(f.corners[i], 1.0f);
		if(transf.z > maxZ) maxZ = transf.z;
		if(transf.z < minZ) minZ = transf.z;
	}
    
	//Make sure all relevant shadow casters are included - use object bounding boxes!
    /*btVector3 aabbMin, aabbMax;
    SimulationApp::getApp()->getSimulationManager()->getWorldAABB(aabbMin, aabbMax);
    transf = shad_mv * glm::vec4(aabbMin.x(), aabbMin.y(), aabbMin.z(), 1.f);
    if(transf.z > maxZ) maxZ = transf.z;
    if(transf.z < minZ) minZ = transf.z;
    
    transf = shad_mv * glm::vec4(aabbMax.x(), aabbMax.y(), aabbMax.z(), 1.f);
    if(transf.z > maxZ) maxZ = transf.z;
    if(transf.z < minZ) minZ = transf.z;*/
    
	//Set the projection matrix with the new z-bounds
	//Note: there is inversion because the light looks at the neg z axis
    glm::mat4 shad_proj = glm::ortho(-1.f, 1.f, -1.f, 1.f, -maxZ, -minZ);
	glm::mat4 shad_mvp =  shad_proj * shad_mv;
    
	//Find the extends of the frustum slice as projected in light's homogeneous coordinates
    for(int i = 0; i < 8; i++)
	{
		transf = shad_mvp * glm::vec4(f.corners[i], 1.0f);

		transf.x /= transf.w;
		transf.y /= transf.w;
        
		if(transf.x > maxX) maxX = transf.x;
		if(transf.x < minX) minX = transf.x;
		if(transf.y > maxY) maxY = transf.y;
		if(transf.y < minY) minY = transf.y;
	}
    
    //Build crop matrix
    glm::mat4 shad_crop = glm::mat4(1.f);
	GLfloat scaleX = 2.0f/(maxX - minX);
	GLfloat scaleY = 2.0f/(maxY - minY);
	GLfloat offsetX = -0.5f*(maxX + minX)*scaleX;
	GLfloat offsetY = -0.5f*(maxY + minY)*scaleY;
    shad_crop[0][0] = scaleX;
    shad_crop[1][1] = scaleY;
    shad_crop[3][0] = offsetX;
    shad_crop[3][1] = offsetY;
    
    //Build crop projection matrix
    glm::mat4 shad_crop_proj = shad_crop * shad_proj;
    
	return shad_crop_proj;
}

void OpenGLSun::Render()
{
    //Rendering is done in screen space! (camera eye space)
    
    //calculate eye space sun direction
    btVector3 sunDirectionEye = (activeView->GetViewTransform()).getBasis() * sunDirection;
	GLfloat sunDirEye[3] = {(GLfloat)sunDirectionEye.getX(),(GLfloat)sunDirectionEye.getY(),(GLfloat)sunDirectionEye.getZ()};
    
    //calculate sun color
    GLfloat* sunColor = OpenGLLight::ColorFromTemperature(5000.f + 20.f*sunElevation, 3.f);
    glm::mat4 invCamView = glm::inverse(activeView->GetViewMatrix());
    glm::mat4 bias(0.5f, 0.f, 0.f, 0.f,
                   0.f, 0.5f, 0.f, 0.f,
                   0.f, 0.f, 0.5f, 0.f,
                   0.5f, 0.5f, 0.5f, 1.f);
    GLfloat frustumFar[4];
    glm::mat4 lightClipSpace[4];
    
    for(int i = shadowmapSplits; i < 4; i++)
    {
		frustumFar[i] = 0;
        lightClipSpace[i] = glm::mat4();
    }
    
	// for every active split
	for(int i = 0; i < shadowmapSplits; i++)
	{
		frustumFar[i] = frustum[i].far;
        
		// compute a matrix that transforms from camera eye space to light clip space
		lightClipSpace[i] = (bias * shadowCPM[i]) * invCamView;
	}
    
    //use sun shader
    glActiveTextureARB(GL_TEXTURE0_ARB + shadowTextureUnit);
    glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, shadowmapArray);
    glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    
    glUseProgramObjectARB(sunShader);
    glUniform1iARB(uniDiffuse, diffuseTextureUnit);
    glUniform1iARB(uniPosition, positionTextureUnit);
    glUniform1iARB(uniNormal, normalTextureUnit);
    glUniform3fvARB(uniLightDir, 1, sunDirEye);
    glUniform4fvARB(uniColor, 1, sunColor);
    glUniform1iARB(uniShadowArray, shadowTextureUnit);
    glUniform4fvARB(uniFrustum, 1, frustumFar);
    glUniformMatrix4fvARB(uniLightClipSpace[0], 1, GL_FALSE, glm::value_ptr(lightClipSpace[0]));
    glUniformMatrix4fvARB(uniLightClipSpace[1], 1, GL_FALSE, glm::value_ptr(lightClipSpace[1]));
    glUniformMatrix4fvARB(uniLightClipSpace[2], 1, GL_FALSE, glm::value_ptr(lightClipSpace[2]));
    glUniformMatrix4fvARB(uniLightClipSpace[3], 1, GL_FALSE, glm::value_ptr(lightClipSpace[3]));
    DrawScreenAlignedQuad();
    glUseProgramObjectARB(0);
    
    glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, 0);
}

void OpenGLSun::RenderShadowMaps(OpenGLPipeline* pipe)
{
    //Compute the z-distances for each split as seen in camera space
    UpdateSplitDist(activeView->GetNearClip(), activeView->GetFarClip());
    
    //Render maps
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
    glLoadMatrixf(glm::value_ptr(sunModelview));
    
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glViewport(0, 0, shadowmapSize, shadowmapSize);
    
    btVector3 camPos = activeView->GetEyePosition();
    btVector3 camDir = activeView->GetLookingDirection();
    btVector3 camUp = activeView->GetUpDirection();
    
	// for all shadow splits
	for(int i = 0; i < shadowmapSplits; i++)
	{
		//Compute the camera frustum slice boundary points in world space
        UpdateFrustumCorners(frustum[i], glm::vec3(camPos.x(), camPos.y(), camPos.z()), glm::vec3(camDir.x(), camDir.y(), camDir.z()), glm::vec3(camUp.x(), camUp.y(), camUp.z()));
		
        //Adjust the view frustum of the light, so that it encloses the camera frustum slice fully.
		//note that this function sets the projection matrix as it sees best fit
		glMatrixMode(GL_PROJECTION);
        glm::mat4 cp = BuildCropProjMatrix(frustum[i]);
        glLoadMatrixf(glm::value_ptr(cp));
        glMatrixMode(GL_MODELVIEW);
        shadowCPM[i] =  cp * sunModelview;

        //Draw current depth map
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowmapArray, 0, i);
		glClear(GL_DEPTH_BUFFER_BIT);
        pipe->DrawStandardObjects();
	}
    
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    //glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void OpenGLSun::ShowFrustumSplits()
{
    glColor4f(1.f, 0.f, 0.f, 1.f);
    for(int i = 0; i < shadowmapSplits; i++)
    {
        glBegin(GL_LINE_LOOP);
        for(int h = 4; h < 8; h++)
            glVertex3fv(glm::value_ptr(frustum[i].corners[h]));
        glEnd();
    }
}

void OpenGLSun::ShowShadowMaps(GLfloat x, GLfloat y, GLfloat scale)
{
    //save current state
    glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
    glPushAttrib(GL_VIEWPORT_BIT);
    
    //Set projection and modelview
    SetupOrtho();
    
	//Texture setup
	glActiveTextureARB(GL_TEXTURE0_ARB + shadowTextureUnit);
    glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, shadowmapArray);
    glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    
	//Render the shadowmaps
    glUseProgramObjectARB(shadowmapShader);
    glUniform1iARB(uniShadowmapArray, 6);
    for(int i = 0; i < shadowmapSplits; i++)
    {
        glViewport(x + shadowmapSize * scale * i, y, shadowmapSize * scale, shadowmapSize * scale);
        glUniform1fARB(uniShadowmapLayer, (GLfloat)i);
        glColor4f(1.f,1.f,1.f,1.f);
        DrawScreenAlignedQuad();
    }
    glUseProgramObjectARB(0);
    
	//Reset
	glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, 0);
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void OpenGLSun::SetCamera(OpenGLView* view)
{
    activeView = view;
    for(int i = 0; i < shadowmapSplits; i++)
    {
        frustum[i].fov = view->GetFOVY() + 0.2f; //avoid artifacts in the borders
        GLint* viewport = view->GetViewport();
        frustum[i].ratio = (GLfloat)viewport[2]/(GLfloat)viewport[3];
        delete [] viewport;
    }
}

void OpenGLSun::SetTextureUnits(GLint diffuse, GLint normal, GLint position, GLint shadow)
{
    diffuseTextureUnit = diffuse;
    normalTextureUnit = normal;
    positionTextureUnit = position;
    shadowTextureUnit = shadow;
}

void OpenGLSun::SetPosition(GLfloat elevation, GLfloat orientation)
{
    sunElevation = elevation;
    sunOrientation = orientation;
    
    //calculate sun direction
    sunDirection = btVector3(0.f, -1.f, 0.f);
    sunDirection = sunDirection.rotate(btVector3(1.f,0.f,0.f), sunElevation/180.f*M_PI);
    sunDirection = sunDirection.rotate(btVector3(0.f,0.f,1.f), -sunOrientation/180.f*M_PI);
    sunDirection.normalize();
    
    //build sun modelview
    glm::vec3 dir(sunDirection.x(), sunDirection.y(), sunDirection.z());
    glm::vec3 up(0.f, 0.f, 1.f);
    glm::vec3 right = glm::cross(dir, up);
	right = glm::normalize(right);
	up = glm::normalize(glm::cross(right, dir));
    sunModelview = glm::lookAt(glm::vec3(0,0,0), dir, up) * glm::mat4(1.f);
}