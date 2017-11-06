//
//  OpenGLPipeline.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/03/2014.
//  Copyright(c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "OpenGLPipeline.h"

#include "SimulationManager.h"
#include "GLSLShader.h"
#include "MathsUtil.hpp"
#include "OpenGLContent.h"
#include "OpenGLView.h"
#include "OpenGLAtmosphere.h"
#include "OpenGLLight.h"
#include "Console.h"
#include "PathGenerator.h"
#include "PathFollowingController.h"
#include "Ocean.h"
#include "Pool.h"
#include "Manipulator.h"

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
    drawingQueueMutex = SDL_CreateMutex();
}

OpenGLPipeline::~OpenGLPipeline()
{
    OpenGLView::Destroy();
    OpenGLLight::Destroy();
	OpenGLContent::Destroy();
	
    glDeleteTextures(1, &screenTex);
    glDeleteFramebuffers(1, &screenFBO);
    SDL_DestroyMutex(drawingQueueMutex);
}

void OpenGLPipeline::setRenderingEffects(bool shadows, bool fluid, bool ambientOcclusion)
{
    renderShadows = shadows;
    renderFluid = fluid;
    renderSAO = ambientOcclusion;
}

void OpenGLPipeline::setVisibleHelpers(bool coordSystems, bool joints, bool actuators, bool sensors, bool lights, bool cameras, bool fluidDynamics)
{
    showCoordSys = coordSystems;
    showJoints = joints;
    showActuators = actuators;
    showSensors = sensors;
    showLightMeshes = lights;
    showCameraFrustums = cameras;
    showFluidDynamics = fluidDynamics;
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

SDL_mutex* OpenGLPipeline::getDrawingQueueMutex()
{
    return drawingQueueMutex;
}

void OpenGLPipeline::Initialize(GLint windowWidth, GLint windowHeight)
{
    windowW = windowWidth;
    windowH = windowHeight;
    
    //Set default options
    cInfo("Setting up basic OpenGL parameters...");
    setRenderingEffects(true, true, true);
    setVisibleHelpers(false, false, false, false, false, false, false);
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
	OpenGLAtmosphere::getInstance()->Init();
	OpenGLContent::getInstance()->Init();
    OpenGLView::Init();
    
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
}

void OpenGLPipeline::AddToDrawingQueue(Renderable r)
{
	drawingQueue.push_back(r);
}

void OpenGLPipeline::ClearDrawingQueue()
{
	drawingQueue.clear();
}

void OpenGLPipeline::DrawDisplay()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, screenFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, windowW, windowH, 0, 0, windowW, windowH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void OpenGLPipeline::DrawObjects()
{
    for(unsigned int i=0; i<drawingQueueCopy.size(); ++i)
    {
        if(drawingQueueCopy[i].type == RenderableType::SOLID)
            OpenGLContent::getInstance()->DrawObject(drawingQueueCopy[i].objectId, drawingQueueCopy[i].lookId, drawingQueueCopy[i].model);
    }
}

void OpenGLPipeline::Render(SimulationManager* sim)
{
    //Copy drawing queue -> double buffering to enable threading
    drawingQueueCopy.clear();
    SDL_LockMutex(drawingQueueMutex);
    drawingQueueCopy.insert(drawingQueueCopy.end(), drawingQueue.begin(), drawingQueue.end());
    SDL_UnlockMutex(drawingQueueMutex);
    
    unsigned int renderMode = 0;
    
    //Simulate ocean
	Liquid* liquid = sim->getLiquid();
    
    if(liquid != NULL)
    {
        if(liquid->getForcefieldType() == ForcefieldType::FORCEFIELD_OCEAN)
        {
            ((Ocean*)liquid)->getOpenGLOcean().SimulateOcean();
            renderMode = renderFluid ? 2 : 0;
        }
        else //POOL
        {
            renderMode = renderFluid ? 1 : 0;
        }
    }
		
	/*GLfloat az,elev;
	OpenGLAtmosphere::getInstance()->GetSunPosition(az, elev);
	az += 0.05f;
	elev = sin(az/180.f*M_PI) * 45.f+35.f;
	OpenGLAtmosphere::getInstance()->SetSunPosition(az, elev);*/
	
    //==============Bake shadow maps (independent of view)================
    if(renderShadows)
	{
		OpenGLContent::getInstance()->SetDrawingMode(DrawingMode::FLAT);
        for(unsigned int i=0; i<OpenGLContent::getInstance()->getLightsCount(); ++i)
            OpenGLContent::getInstance()->getLight(i)->BakeShadowmap(this);
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
                OpenGLContent::getInstance()->SetDrawingMode(DrawingMode::FLAT);
                OpenGLAtmosphere::getInstance()->BakeShadowmaps(this, view);
			}
			
            //================Setup rendering scene======================
            if(renderMode == 0)
            {
				glBindFramebuffer(GL_FRAMEBUFFER, view->getRenderFBO());
				GLenum renderBuffs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
				glDrawBuffers(2, renderBuffs);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
                //Render all objects
				view->SetViewport();
				OpenGLContent::getInstance()->SetCurrentView(view);
				OpenGLContent::getInstance()->SetDrawingMode(DrawingMode::FULL);
				glDrawBuffers(2, renderBuffs);
				DrawObjects();
            
				//Ambient occlusion
				view->DrawAO();
			
				//Render sky
				OpenGLAtmosphere::getInstance()->DrawSkyAndSun(view);
				
				//Go to postprocessing stage
				view->EnterPostprocessing();
            }
            else if(renderMode == 1)
            {
				OpenGLPool& glPool = ((Pool*)liquid)->getOpenGLPool();
                
				if(view->GetEyePosition().z >= 0.f)
				{
					glBindFramebuffer(GL_FRAMEBUFFER, view->getRenderFBO());
					GLenum renderBuffs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
					glDrawBuffers(2, renderBuffs);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					view->SetViewport();
					OpenGLContent::getInstance()->SetCurrentView(view);
					
					//Render all objects
					OpenGLContent::getInstance()->SetDrawingMode(DrawingMode::UNDERWATER);
					OpenGLContent::getInstance()->EnableClipPlane(glm::vec4(0,0,-1.f,0));
					DrawObjects();
					OpenGLContent::getInstance()->DisableClipPlane();
					
					OpenGLContent::getInstance()->SetDrawingMode(DrawingMode::FULL);
					OpenGLContent::getInstance()->EnableClipPlane(glm::vec4(0,0,1.f,0));
					DrawObjects();
					OpenGLContent::getInstance()->DisableClipPlane();
					
					//Ambient occlusion
					view->DrawAO();
			
					//Generate reflection texture
					glBindFramebuffer(GL_FRAMEBUFFER, view->getReflectionFBO());
					glDrawBuffer(GL_COLOR_ATTACHMENT0);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					view->SetReflectionViewport();
					OpenGLContent::getInstance()->SetCurrentView(view, true);
					
					//Render all objects
					OpenGLContent::getInstance()->SetDrawingMode(DrawingMode::FULL);
					OpenGLContent::getInstance()->EnableClipPlane(glm::vec4(0,0,1.f,0));
					glCullFace(GL_FRONT);
					DrawObjects();
					glCullFace(GL_BACK);
					OpenGLContent::getInstance()->DisableClipPlane();
					
					//Draw surface
					glBindFramebuffer(GL_FRAMEBUFFER, view->getRenderFBO());
					glDrawBuffers(2, renderBuffs);
					view->SetViewport();
					OpenGLContent::getInstance()->SetCurrentView(view);
					
					//Draw water surface
					glPool.DrawSurface(view->GetEyePosition(), view->GetViewMatrix(), view->GetInfiniteProjectionMatrix(), view->getReflectionTexture());
		
					//Render sky
					OpenGLAtmosphere::getInstance()->DrawSkyAndSun(view);
					
					//Go to postprocessing stage
					view->EnterPostprocessing();
				}
				else //Underwater
				{
					glBindFramebuffer(GL_FRAMEBUFFER, view->getRenderFBO());
					GLenum renderBuffs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
					glDrawBuffers(2, renderBuffs);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
					//Render all objects
					view->SetViewport();
					OpenGLContent::getInstance()->SetCurrentView(view);
					
					//OpenGLContent::getInstance()->SetDrawingMode(DrawingMode::UNDERWATER);
					//DrawObjects();
					OpenGLContent::getInstance()->SetDrawingMode(DrawingMode::UNDERWATER);
					OpenGLContent::getInstance()->EnableClipPlane(glm::vec4(0,0,-1.f,0));
					DrawObjects();
					OpenGLContent::getInstance()->DisableClipPlane();
					
					OpenGLContent::getInstance()->SetDrawingMode(DrawingMode::FULL);
					OpenGLContent::getInstance()->EnableClipPlane(glm::vec4(0,0,1.f,0));
					DrawObjects();
					OpenGLContent::getInstance()->DisableClipPlane();
					
					//Ambient occlusion
					view->DrawAO();
					
					//Render planar reflection
					glBindFramebuffer(GL_FRAMEBUFFER, view->getReflectionFBO());
					glDrawBuffer(GL_COLOR_ATTACHMENT0);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					view->SetReflectionViewport();
					OpenGLContent::getInstance()->SetCurrentView(view, true);
					
					//Render all objects
					OpenGLContent::getInstance()->SetDrawingMode(DrawingMode::UNDERWATER);
					OpenGLContent::getInstance()->EnableClipPlane(glm::vec4(0,0,-1.f,0));
					glCullFace(GL_FRONT);
					DrawObjects();
					glCullFace(GL_BACK);
					glm::vec3 eyePos = view->GetEyePosition();
					eyePos.z = -eyePos.z;
					glPool.DrawBackground(eyePos, OpenGLContent::getInstance()->GetViewMatrix(), view->GetProjectionMatrix());
					OpenGLContent::getInstance()->DisableClipPlane();
					
					//Draw water surface
					glBindFramebuffer(GL_FRAMEBUFFER, view->getRenderFBO());
					glDrawBuffers(2, renderBuffs);
					view->SetViewport();
					OpenGLContent::getInstance()->SetCurrentView(view);
					glPool.DrawBacksurface(view->GetEyePosition(), view->GetViewMatrix(), view->GetProjectionMatrix(), view->getReflectionTexture());
					
					//Distant pool background
					glPool.DrawBackground(view->GetEyePosition(), view->GetViewMatrix(), view->GetProjectionMatrix());
					
					view->GenerateLinearDepth(0);
					
					//Go to postprocessing stage
					view->EnterPostprocessing();
					view->GenerateBlurArray();
					
					//Draw water volume
					glBindFramebuffer(GL_FRAMEBUFFER, view->getRenderFBO());
					glPool.DrawVolume(view->getPostprocessTexture(2), view->getLinearDepthTexture());
					
					//Render sky
					//OpenGLAtmosphere::getInstance()->DrawSkyAndSun(view);
					
					//Go to postprocessing again
					view->EnterPostprocessing();
				}
            }
			else if(renderMode == 2)
			{
                /*Ocean* ocean = (Ocean*)liquid;
				view->SetViewport();
				
				glBindFramebuffer(GL_FRAMEBUFFER, view->getRenderFBO());
				GLenum renderBuffs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
				glDrawBuffers(2, renderBuffs);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
				//Draw ocean surface
				ocean->getOpenGLOcean().DrawOceanSurface(view->GetEyePosition(), view->GetViewMatrix(), view->GetProjectionMatrix());
				
				//Draw stencil mask
				glEnable(GL_STENCIL_TEST);
				
				glStencilFunc(GL_ALWAYS, 1, 0xFF);
				glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
				glStencilMask(0xFF);
				glClear(GL_STENCIL_BUFFER_BIT);
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
				glDepthMask(GL_FALSE);
				
				ocean->getOpenGLOcean().DrawOceanVolumeMask(view->GetEyePosition(), view->GetLookingDirection(), view->GetViewMatrix(), view->GetProjectionMatrix());				
				
				glStencilFunc(GL_EQUAL, 0, 0xFF);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
				glStencilMask(0x00);
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glDepthMask(GL_TRUE);
				
				//Render all objects above ocean surface
				OpenGLContent::getInstance()->SetCurrentView(view);
				OpenGLContent::getInstance()->SetDrawingMode(DrawingMode::FULL);
				glDrawBuffers(2, renderBuffs);
				DrawObjects();
            
				//Ambient occlusion
				//view->DrawAO();
				
				//Draw sky
				OpenGLAtmosphere::getInstance()->DrawSkyAndSun(view);
				
				glStencilFunc(GL_EQUAL, 1, 0xFF);
				
				//Render all objects below ocean surface
				OpenGLContent::getInstance()->SetDrawingMode(DrawingMode::UNDERWATER);
				glDrawBuffers(2, renderBuffs);
				DrawObjects();
					
				//Ambient occlusion
				//view->DrawAO();
					
				//Draw ocean surface from below
				ocean->getOpenGLOcean().DrawOceanBacksurface(view->GetEyePosition(), view->GetViewMatrix(), view->GetProjectionMatrix());
				
				glDisable(GL_STENCIL_TEST);
				
				//Go to postprocessing stage
				view->EnterPostprocessing();*/
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
				if(sim->isZAxisUp())
					OpenGLContent::getInstance()->DrawCoordSystem(glm::mat4(), 2.f);
				else
					OpenGLContent::getInstance()->DrawCoordSystem(glm::rotate((float)M_PI, glm::vec3(0,1.f,0)), 2.f);
                
				for(unsigned int h=0; h<drawingQueueCopy.size(); ++h)
				{
					if(drawingQueueCopy[h].type == RenderableType::SOLID_CS)
                        OpenGLContent::getInstance()->DrawCoordSystem(drawingQueueCopy[h].model, 0.5f);
				}
            }
            
            //Sensors
            if(showSensors)
            {
                for(unsigned int h=0; h<drawingQueueCopy.size(); ++h)
                {
                    if(drawingQueueCopy[h].type == RenderableType::SENSOR_LINES)
                        OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINES, drawingQueueCopy[h].points, glm::vec4(0,1.f,0,1.f), drawingQueueCopy[h].model);
                    else if(drawingQueueCopy[h].type == RenderableType::SENSOR_LINE_STRIP)
                        OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINE_STRIP, drawingQueueCopy[h].points, glm::vec4(0,1.f,0,1.f), drawingQueueCopy[h].model);
                }
            }
            
            //Fluid dynamics
            if(showFluidDynamics)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                
                for(unsigned int h=0; h<drawingQueueCopy.size(); ++h)
                {
                    if(drawingQueueCopy[h].type == RenderableType::HYDRO_CS)
                        OpenGLContent::getInstance()->DrawEllipsoid(drawingQueueCopy[h].model, glm::vec3(0.02f));//drawingQueueCopy[h].scale*2.f);
                    else if(drawingQueueCopy[h].type == RenderableType::HYDRO)
                        OpenGLContent::getInstance()->DrawEllipsoid(drawingQueueCopy[h].model, drawingQueueCopy[h].points[0]);
                }
                
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
                
            //Debugging
			//if(ocean != NULL)
			//{	
				//ocean->getOpenGLOcean().ShowOceanSpectrum(glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]), glm::vec4(0,200,300,300));
				//ocean->getOpenGLOcean().ShowOceanTexture(3, glm::vec4(0,500,300,300));
			//}
			
			//OpenGLAtmosphere::getInstance()->ShowAtmosphereTexture(AtmosphereTextures::TRANSMITTANCE,glm::vec4(0,200,400,400));
			//OpenGLAtmosphere::getInstance()->ShowAtmosphereTexture(AtmosphereTextures::SCATTERING,glm::vec4(400,200,400,400));
			//OpenGLAtmosphere::getInstance()->ShowAtmosphereTexture(AtmosphereTextures::IRRADIANCE,glm::vec4(800,200,400,400));
			//OpenGLAtmosphere::getInstance()->ShowSunShadowmaps(0, 0, 0.05f);
			
			//view->ShowSceneTexture(SceneComponent::REFLECTED, glm::vec4(0,700,300,200));
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
			
			//sim->views[i]->ShowAmbientOcclusion(0, 0, 300, 200);		
            
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
            delete viewport;
        }
    }
}