//
//  OpenGLPipeline.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/03/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "OpenGLPipeline.h"

#include "SimulationManager.h"
#include "GLSLShader.h"
#include "GeometryUtil.hpp"
#include "OpenGLContent.h"
#include "OpenGLView.h"
#include "OpenGLSky.h"
#include "OpenGLSun.h"
#include "OpenGLLight.h"
#include "Console.h"
#include "PathGenerator.h"
#include "PathFollowingController.h"

OpenGLPipeline* OpenGLPipeline::instance = NULL;

OpenGLPipeline* OpenGLPipeline::getInstance()
{
    if(instance == NULL)
        instance = new OpenGLPipeline();
    
    return instance;
}

OpenGLPipeline::OpenGLPipeline()
{
	renderShadows = renderFluid = renderSAO = false;
    showCoordSys = showJoints = showActuators = showSensors = false;
    showLightMeshes = showCameraFrustums = false;
}

OpenGLPipeline::~OpenGLPipeline()
{
    OpenGLView::Destroy();
    OpenGLLight::Destroy();
	OpenGLContent::Destroy();
	
    glDeleteTextures(1, &screenTex);
    glDeleteFramebuffers(1, &screenFBO);
}

void OpenGLPipeline::setRenderingEffects(bool shadows, bool fluid, bool ambientOcclusion)
{
    renderShadows = shadows;
    renderFluid = fluid;
    renderSAO = ambientOcclusion;
}

void OpenGLPipeline::setVisibleHelpers(bool coordSystems, bool joints, bool actuators, bool sensors, bool lights, bool cameras)
{
    showCoordSys = coordSystems;
    showJoints = joints;
    showActuators = actuators;
    showSensors = sensors;
    showLightMeshes = lights;
    showCameraFrustums = cameras;
}

void OpenGLPipeline::setDebugSimulation(bool enabled)
{
    drawDebug = enabled;
}

bool OpenGLPipeline::isFluidRendered()
{
    return renderFluid;
}

bool OpenGLPipeline::isSAORendered()
{
    return renderSAO;
}

GLuint OpenGLPipeline::getScreenTexture()
{
    return screenTex;
}

void OpenGLPipeline::Initialize(GLint windowWidth, GLint windowHeight)
{
    windowW = windowWidth;
    windowH = windowHeight;
    
    //Set default options
    cInfo("Setting up basic OpenGL parameters...");
    setRenderingEffects(true, true, true);
    setVisibleHelpers(false, false, false, false, false, false);
    setDebugSimulation(false);
    
    //OpenGL flags and params
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glPointSize(5.f);
    glLineWidth(1.0f);
    glLineStipple(3, 0xE4E4);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, new GLfloat[2]{1,1});
	glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, new GLfloat[4]{1,1,1,1});
    
	GLint texUnits;
	GLint maxTexLayers;
	GLint maxUniforms;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texUnits);
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxTexLayers);
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &maxUniforms);
	cInfo("%d texture units available.", texUnits);
	cInfo("%d texture layers allowed.", maxTexLayers);
	cInfo("%d uniforms in fragment shader allowed.", maxUniforms);
	
	//Load shaders and create rendering buffers
    cInfo("Loading shaders...");
	OpenGLContent::getInstance()->Init();
	OpenGLSky::getInstance()->Init();
    OpenGLSun::getInstance()->Init();
    OpenGLView::Init();
    OpenGLLight::Init();
    
    cInfo("Generating sky...");
    OpenGLSky::getInstance()->Generate(40.f,90.f);
    
    //Create display framebuffer
    glGenFramebuffers(1, &screenFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
    
    glGenTextures(1, &screenTex);
    glBindTexture(GL_TEXTURE_2D, screenTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, windowW, windowH, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //Cheaper/better gaussian blur for GUI background
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //Cheaper/better gaussian blur for GUI background 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTex, 0);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Display FBO initialization failed!");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    cInfo("OpenGL pipeline initialized.");
}

void OpenGLPipeline::DrawDisplay()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, screenFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, windowW, windowH, 0, 0, windowW, windowH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void OpenGLPipeline::DrawObjects(SimulationManager* sim)
{
    for(int i=0; i<sim->entities.size(); ++i)
        sim->entities[i]->Render();
}

void OpenGLPipeline::Render(SimulationManager* sim)
{
	Ocean* ocean = sim->getOcean();
	if(ocean != NULL)
		ocean->getOpenGLOcean().SimulateOcean();
	
    //==============Bake shadow maps (independent of view)================
    if(renderShadows)
	{
		OpenGLContent::getInstance()->SetDrawFlatObjects(true);
        for(unsigned int i=0; i<OpenGLContent::getInstance()->getLightsCount(); ++i)
            OpenGLContent::getInstance()->getLight(i)->RenderShadowMap(this, sim);
	}
    
    //==============Clear display framebuffer====================
    glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    
    //==================Loop through all views==========================
    for(unsigned int i=0; i<OpenGLContent::getInstance()->getViewsCount(); ++i)
    {
		OpenGLView* view = OpenGLContent::getInstance()->getView(i);
        
        if(view->isActive())
        {
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            
            //=================Apply view properties======================
            OpenGLLight::SetCamera(view);
			GLint* viewport = view->GetViewport();
            OpenGLContent::getInstance()->SetViewportSize(viewport[2],viewport[3]);
			
            //=================Bake sun shadows========================
            if(renderShadows)
			{
                OpenGLContent::getInstance()->SetDrawFlatObjects(true);
                OpenGLSun::getInstance()->RenderShadowMaps(this, view, sim);
			}
			
            //================Setup rendering scene======================
			glBindFramebuffer(GL_FRAMEBUFFER, view->getRenderFBO());
			GLenum renderBuffs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
			glDrawBuffers(2, renderBuffs);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			if(ocean != NULL)
			{
				//================Draw precomputed sky=======================
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				OpenGLSky::getInstance()->Render(view, sim->zUp);
				glEnable(GL_CULL_FACE);
				glEnable(GL_DEPTH_TEST);
				
				//=================Draw objects===============================
				//Bind standard textures
				glActiveTexture(GL_TEXTURE0 + TEX_SKY_DIFFUSE);
				glEnable(GL_TEXTURE_CUBE_MAP);
				glBindTexture(GL_TEXTURE_CUBE_MAP, OpenGLSky::getInstance()->getDiffuseCubemap());
			
				//Render all objects
				view->SetViewport();
				OpenGLContent::getInstance()->SetCurrentView(view);
				OpenGLContent::getInstance()->SetDrawFlatObjects(false);
				glDrawBuffers(2, renderBuffs);
				DrawObjects(sim);
            
				//Draw ocean surface
				//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				ocean->getOpenGLOcean().DrawOceanSurface(view->GetEyePosition(), view->GetProjectionMatrix() * view->GetViewMatrix());
				//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				
				//Ambient occlusion
				//glDisable(GL_DEPTH_TEST);
				//glDisable(GL_CULL_FACE);
				//view->DrawAO();
				
				/*glBindFramebuffer(GL_FRAMEBUFFER, view->getRenderFBO());
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				glEnable(GL_DEPTH_TEST);
				glEnable(GL_CULL_FACE);
				
				*/
			}
			else
			{
				//================Draw precomputed sky=======================
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				OpenGLSky::getInstance()->Render(view, sim->zUp);
				glEnable(GL_CULL_FACE);
				glEnable(GL_DEPTH_TEST);
            
				//=================Draw objects===============================
				//Bind standard textures
				glActiveTexture(GL_TEXTURE0 + TEX_SKY_DIFFUSE);
				glEnable(GL_TEXTURE_CUBE_MAP);
				glBindTexture(GL_TEXTURE_CUBE_MAP, OpenGLSky::getInstance()->getDiffuseCubemap());
			
				//Render all objects
				view->SetViewport();
				OpenGLContent::getInstance()->SetCurrentView(view);
				OpenGLContent::getInstance()->SetDrawFlatObjects(false);
				glDrawBuffers(2, renderBuffs);
				DrawObjects(sim);
            
				//Ambient occlusion
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);
				view->DrawAO();
			}
			
            //================Post-processing=============================
            glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
            view->DrawHDR(screenFBO);
           
            //================Helper objects===================
            glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
			glBindTexture(GL_TEXTURE_2D, 0);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_DEPTH_BUFFER_BIT);
			
            glm::mat4 proj = view->GetProjectionMatrix();
			glm::mat4 viewM = view->GetViewMatrix();
			OpenGLContent::getInstance()->SetProjectionMatrix(proj);
			OpenGLContent::getInstance()->SetViewMatrix(viewM);
			
            //Bullet debug draw
            if(drawDebug)
                sim->dynamicsWorld->debugDrawWorld();
            
            //Coordinate systems
            if(showCoordSys)
            {
				OpenGLContent::getInstance()->DrawCoordSystem(glm::mat4(), 2.f);
                
                for(int h = 0; h < sim->entities.size(); h++)
                    if(sim->entities[h]->getType() == ENTITY_SOLID)
                    {
						SolidEntity* solid = (SolidEntity*)sim->entities[h];
                        btTransform comT = solid->getTransform();
                        OpenGLContent::getInstance()->DrawCoordSystem(glMatrixFromBtTransform(comT), 0.1f);
                    }
                    else if(sim->entities[h]->getType() == ENTITY_FEATHERSTONE)
                    {
                        FeatherstoneEntity* fe = (FeatherstoneEntity*)sim->entities[h];
                        fe->RenderStructure();
                    }
                    else if(sim->entities[h]->getType() == ENTITY_SYSTEM)
                    {
                        SystemEntity* system = (SystemEntity*)sim->entities[h];
                        btTransform comT = system->getTransform();
                        OpenGLContent::getInstance()->DrawCoordSystem(glMatrixFromBtTransform(comT), 0.1f);
                    }
            }
            
            //Joints
			for(int h=0; h<sim->joints.size(); h++)
				if(sim->joints[h]->isRenderable())
					sim->joints[h]->Render();
            
            //Contact points
            for(int h = 0; h < sim->contacts.size(); h++)
                sim->contacts[h]->Render();
            
            //Sensors
            for(int h = 0; h < sim->sensors.size(); h++)
                if(sim->sensors[h]->isRenderable())
                    sim->sensors[h]->Render();
            
            //Paths
            for(int h = 0; h < sim->controllers.size(); h++)
                if(sim->controllers[h]->getType() == CONTROLLER_PATHFOLLOWING)
                    ((PathFollowingController*)sim->controllers[h])->RenderPath();

            //Lights
            if(showLightMeshes)
                for(unsigned int h = 0; h < OpenGLContent::getInstance()->getLightsCount(); ++h)
                    OpenGLContent::getInstance()->getLight(h)->RenderDummy();
            
            //Cameras
            if(showCameraFrustums)
                for(unsigned int h = 0; h < OpenGLContent::getInstance()->getViewsCount(); ++h)
				{
					OpenGLView* otherView = OpenGLContent::getInstance()->getView(h);
					if(i != h && otherView->getType() == CAMERA)
                    {
                        OpenGLCamera* cam = (OpenGLCamera*)otherView;
                        cam->RenderDummy();
                    }
				}
                        
            //Debugging
			if(ocean != NULL)
			{	
				//ocean->getOpenGLOcean().ShowOceanSpectrum(glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]), glm::vec4(0,200,300,300));
				//ocean->getOpenGLOcean().ShowOceanTexture(3, glm::vec4(0,500,300,300));
			}
			//view->ShowLinearDepthTexture(glm::vec4(0,200,300,200));
			//view->ShowViewNormalTexture(glm::vec4(0,400,300,200));
			//view->ShowDeinterleavedDepthTexture(glm::vec4(0,400,300,200), 0);
			//view->ShowDeinterleavedDepthTexture(glm::vec4(0,400,300,200), 8);
			//view->ShowDeinterleavedDepthTexture(glm::vec4(0,600,300,200), 9);
			//view->ShowDeinterleavedAOTexture(glm::vec4(0,600,300,200), 0);
			//view->ShowAmbientOcclusion(glm::vec4(0,600,300,200));
			
			//sim->views[i]->getGBuffer()->ShowTexture(DIFFUSE, 0,0,300,200); // FBO debugging
            //sim->views[i]->getGBuffer()->ShowTexture(POSITION1,0,200,300,200); // FBO debugging
			//sim->views[i]->getGBuffer()->ShowTexture(NORMAL1,0,400,300,200); // FBO debugging
			
			//sim->views[i]->ShowSceneTexture(NORMAL, 0, 600, 300, 200);
            //sim->lights[0]->ShowShadowMap(0, 800, 300, 300);
			
			//OpenGLSky::getInstance()->ShowCubemap(SKY);
 			
			//sim->views[i]->ShowAmbientOcclusion(0, 0, 300, 200);		
            
			//OpenGLSun::getInstance()->ShowShadowMaps(0, 0, 0.05);
			//OpenGLSun::getInstance()->ShowFrustumSplits();
		   
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
            delete viewport;
        }
    }
}