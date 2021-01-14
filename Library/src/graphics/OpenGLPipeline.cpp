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
//  OpenGLPipeline.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/03/2014.
//  Copyright(c) 2014-2020 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLPipeline.h"

#include <algorithm>
#include "core/SimulationManager.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLOcean.h"
#include "graphics/OpenGLTrackball.h"
#include "graphics/OpenGLCamera.h"
#include "graphics/OpenGLRealCamera.h"
#include "graphics/OpenGLDepthCamera.h"
#include "graphics/OpenGLSonar.h"
#include "graphics/OpenGLAtmosphere.h"
#include "graphics/OpenGLLight.h"
#include "graphics/OpenGLOceanParticles.h"
#include "utils/SystemUtil.hpp"
#include "entities/forcefields/Ocean.h"
#include "entities/forcefields/Atmosphere.h"
#include "core/GraphicalSimulationApp.h"

namespace sf
{

OpenGLPipeline::OpenGLPipeline(RenderSettings s, HelperSettings h) : rSettings(s), hSettings(h)
{
    drawingQueueMutex = SDL_CreateMutex();
    
    //Set default OpenGL options
    cInfo("Initialising OpenGL rendering pipeline...");
     
    //Load shaders and create rendering buffers
    cInfo("Loading shaders...");
    OpenGLAtmosphere::Init();
    OpenGLCamera::Init(rSettings);
    OpenGLDepthCamera::Init();
    OpenGLSonar::Init();
    OpenGLOceanParticles::Init();
    content = new OpenGLContent();
    
    //Create display framebuffer
    glGenFramebuffers(1, &screenFBO);
    OpenGLState::BindFramebuffer(screenFBO);
    glGenTextures(1, &screenTex);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, screenTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, rSettings.windowW, rSettings.windowH, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //Cheaper/better gaussian blur for GUI background
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //Cheaper/better gaussian blur for GUI background
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTex, 0);
    OpenGLState::UnbindTexture(TEX_BASE);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Display FBO initialization failed!");
    OpenGLState::BindFramebuffer(0);
    lastSimTime = Scalar(0);
}

OpenGLPipeline::~OpenGLPipeline()
{
    OpenGLCamera::Destroy();
    OpenGLDepthCamera::Destroy();
    OpenGLSonar::Destroy();
    OpenGLOceanParticles::Destroy();
    OpenGLLight::Destroy();
    delete content;
    
    glDeleteTextures(1, &screenTex);
    glDeleteFramebuffers(1, &screenFBO);
    SDL_DestroyMutex(drawingQueueMutex);
}

RenderSettings OpenGLPipeline::getRenderSettings() const
{
    return rSettings;
}
    
HelperSettings& OpenGLPipeline::getHelperSettings()
{
    return hSettings;
}
    
GLuint OpenGLPipeline::getScreenTexture()
{
    return screenTex;
}

SDL_mutex* OpenGLPipeline::getDrawingQueueMutex()
{
    return drawingQueueMutex;
}
    
OpenGLContent* OpenGLPipeline::getContent()
{
    return content;
}

void OpenGLPipeline::AddToDrawingQueue(const Renderable& r)
{
    drawingQueue.push_back(r);
}

void OpenGLPipeline::AddToDrawingQueue(const std::vector<Renderable>& r)
{
    drawingQueue.insert(drawingQueue.end(), r.begin(), r.end());
}

void OpenGLPipeline::AddToSelectedDrawingQueue(const std::vector<Renderable>& r)
{
    selectedDrawingQueue.insert(selectedDrawingQueue.end(), r.begin(), r.end());
}

bool OpenGLPipeline::isDrawingQueueEmpty()
{
    return drawingQueue.empty();
}
    
void OpenGLPipeline::PerformDrawingQueueCopy(SimulationManager* sim)
{
    if(!drawingQueue.empty())
    {
        drawingQueueCopy.clear();
        selectedDrawingQueueCopy.clear();

        SDL_LockMutex(drawingQueueMutex);
        //Double buffering
        drawingQueueCopy.insert(drawingQueueCopy.end(), drawingQueue.begin(), drawingQueue.end());
        selectedDrawingQueueCopy.insert(selectedDrawingQueueCopy.end(), selectedDrawingQueue.begin(), selectedDrawingQueue.end());

        //Update vision sensor transforms and copy generated data to ensure consistency
        glMemoryBarrier(GL_PIXEL_BUFFER_BARRIER_BIT);
        for(unsigned int i=0; i < content->getViewsCount(); ++i)
            content->getView(i)->UpdateTransform();
        //Update light transforms to ensure consistency
        for(unsigned int i=0; i < content->getLightsCount(); ++i)
            content->getLight(i)->UpdateTransform();
        //Update ocean currents for particle systems
        Ocean* ocean = sim->getOcean();
        if(ocean != NULL) ocean->UpdateCurrentsData();

        //Enable update of drawing queue by clearing old queue
        drawingQueue.clear(); 
        selectedDrawingQueue.clear();
        SDL_UnlockMutex(drawingQueueMutex);
			
		//Sort objects by material to reduce uniform/texture switching
        std::sort(drawingQueueCopy.begin(), drawingQueueCopy.end(), Renderable::SortByMaterial);
    }
}

void OpenGLPipeline::DrawDisplay()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, screenFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, rSettings.windowW, rSettings.windowH, 0, 0, rSettings.windowW, rSettings.windowH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void OpenGLPipeline::DrawObjects()
{
    for(size_t i=0; i<drawingQueueCopy.size(); ++i)
    {
		if(drawingQueueCopy[i].type == RenderableType::SOLID)
			content->DrawObject(drawingQueueCopy[i].objectId, drawingQueueCopy[i].lookId, drawingQueueCopy[i].model);
    }
}

void OpenGLPipeline::DrawLights()
{
    for(unsigned int i=0; i<content->getLightsCount(); ++i)
        content->DrawLightSource(i);
}
    
void OpenGLPipeline::DrawHelpers()
{
    //Coordinate systems
    if(hSettings.showCoordSys)
    {
        content->DrawCoordSystem(glm::mat4(1.f), 1.f);
        
        for(size_t h=0; h<drawingQueueCopy.size(); ++h)
        {
            if(drawingQueueCopy[h].type == RenderableType::SOLID_CS)
                content->DrawCoordSystem(drawingQueueCopy[h].model, 0.25f);
        }
    }
    
    //Discrete and multibody joints
    if(hSettings.showJoints)
    {
        for(size_t h=0; h<drawingQueueCopy.size(); ++h)
        {
            if(drawingQueueCopy[h].type == RenderableType::MULTIBODY_AXIS)
                content->DrawPrimitives(PrimitiveType::LINES, drawingQueueCopy[h].points, glm::vec4(1.f,0.5f,1.f,1.f), drawingQueueCopy[h].model);
            else if(drawingQueueCopy[h].type == RenderableType::JOINT_LINES)
                content->DrawPrimitives(PrimitiveType::LINES, drawingQueueCopy[h].points, glm::vec4(1.f,0.5f,1.f,1.f), drawingQueueCopy[h].model);
            else if(drawingQueueCopy[h].type == RenderableType::PATH_LINE_STRIP)
                content->DrawPrimitives(PrimitiveType::LINE_STRIP, drawingQueueCopy[h].points, glm::vec4(1.f,0.5f,1.f,1.f), drawingQueueCopy[h].model);
        }
    }
    
    //Sensors
    if(hSettings.showSensors)
    {
        for(size_t h=0; h<drawingQueueCopy.size(); ++h)
        {
            if(drawingQueueCopy[h].type == RenderableType::SENSOR_CS)
                content->DrawCoordSystem(drawingQueueCopy[h].model, 0.25f);
            else if(drawingQueueCopy[h].type == RenderableType::SENSOR_POINTS)
                content->DrawPrimitives(PrimitiveType::POINTS, drawingQueueCopy[h].points, glm::vec4(1.f,1.f,0,1.f), drawingQueueCopy[h].model);
            else if(drawingQueueCopy[h].type == RenderableType::SENSOR_LINES)
                content->DrawPrimitives(PrimitiveType::LINES, drawingQueueCopy[h].points, glm::vec4(1.f,1.f,0,1.f), drawingQueueCopy[h].model);
            else if(drawingQueueCopy[h].type == RenderableType::SENSOR_LINE_STRIP)
                content->DrawPrimitives(PrimitiveType::LINE_STRIP, drawingQueueCopy[h].points, glm::vec4(1.f,1.f,0,1.f), drawingQueueCopy[h].model);
        }
    }
    
    //Actuators
    if(hSettings.showActuators)
    {
        for(size_t h=0; h<drawingQueueCopy.size(); ++h)
        {
            if(drawingQueueCopy[h].type == RenderableType::ACTUATOR_LINES)
                content->DrawPrimitives(PrimitiveType::LINES, drawingQueueCopy[h].points, glm::vec4(1.f,0.5f,0,1.f), drawingQueueCopy[h].model);
        }
    }
    
    //Fluid dynamics
    if(hSettings.showFluidDynamics)
    {
        for(size_t h=0; h<drawingQueueCopy.size(); ++h)
        {
            switch(drawingQueueCopy[h].type)
            {
                case RenderableType::HYDRO_CS:
                    content->DrawEllipsoid(drawingQueueCopy[h].model, glm::vec3(0.005f), glm::vec4(0.3f, 0.7f, 1.f, 1.f));
                    break;
                    
                case RenderableType::HYDRO_CYLINDER:
                    content->DrawCylinder(drawingQueueCopy[h].model, drawingQueueCopy[h].points[0], glm::vec4(0.2f, 0.5f, 1.f, 1.f));
                    break;
                    
                case RenderableType::HYDRO_ELLIPSOID:
                    content->DrawEllipsoid(drawingQueueCopy[h].model, drawingQueueCopy[h].points[0], glm::vec4(0.2f, 0.5f, 1.f, 1.f));
                    break;
                    
                case RenderableType::HYDRO_POINTS:
                    content->DrawPrimitives(PrimitiveType::POINTS, drawingQueueCopy[h].points, glm::vec4(0.3f, 0.7f, 1.f, 1.f), drawingQueueCopy[h].model);
                    break;
                    
                case RenderableType::HYDRO_LINES:
                    content->DrawPrimitives(PrimitiveType::LINES, drawingQueueCopy[h].points, glm::vec4(0.2f, 0.5f, 1.f, 1.f), drawingQueueCopy[h].model);
                    break;
                    
                case RenderableType::HYDRO_LINE_STRIP:
                    content->DrawPrimitives(PrimitiveType::LINE_STRIP, drawingQueueCopy[h].points, glm::vec4(0.2f, 0.5f, 1.f, 1.f), drawingQueueCopy[h].model);
                    break;
                    
                default:
                    break;
            }
        }
    }
    
    //Forces
    if(hSettings.showForces)
    {
        for(size_t h=0; h<drawingQueueCopy.size(); ++h)
        {
            switch(drawingQueueCopy[h].type)
            {
                case RenderableType::FORCE_BUOYANCY:
                    content->DrawPrimitives(PrimitiveType::LINES, drawingQueueCopy[h].points, glm::vec4(0.f,0.f,1.f,1.f), drawingQueueCopy[h].model);
                    break;
        
                case RenderableType::FORCE_LINEAR_DRAG:
                    content->DrawPrimitives(PrimitiveType::LINES, drawingQueueCopy[h].points, glm::vec4(0.f,1.f,1.f,1.f), drawingQueueCopy[h].model);
                    break;
                    
                case RenderableType::FORCE_QUADRATIC_DRAG:
                    content->DrawPrimitives(PrimitiveType::LINES, drawingQueueCopy[h].points, glm::vec4(1.f,0.f,1.f,1.f), drawingQueueCopy[h].model);
                    break;
        
                default:
                    break;
            }
        }
    }
}

void OpenGLPipeline::Render(SimulationManager* sim)
{	
    //Update time step for animation purposes
    Scalar now = sim->getSimulationTime();
    Scalar dt = now-lastSimTime;
    lastSimTime = now;

    //Double-buffering of drawing queue
    PerformDrawingQueueCopy(sim);
	
    //Choose rendering mode
    unsigned int renderMode = 0; //Defaults to rendering without ocean
    Ocean* ocean = sim->getOcean();
    if(ocean != NULL)
    {
        ocean->getOpenGLOcean()->Simulate(dt);
        renderMode = rSettings.ocean > RenderQuality::DISABLED && ocean->isRenderable() ? 1 : 0;
    }
    Atmosphere* atm = sim->getAtmosphere();
    OpenGLState::EnableDepthTest();
    OpenGLState::EnableCullFace();
    
    //Bake shadow maps for lights (independent of view)
    content->SetupLights();
    if(rSettings.shadows > RenderQuality::DISABLED)
    {
        glCullFace(GL_FRONT);
        glDisable(GL_DEPTH_CLAMP);
        content->SetDrawingMode(DrawingMode::SHADOW);
        for(unsigned int i=0; i<content->getLightsCount(); ++i)
            content->getLight(i)->BakeShadowmap(this);
        glEnable(GL_DEPTH_CLAMP);
        glCullFace(GL_BACK);
    }
    
    //Clear display framebuffer
    OpenGLState::BindFramebuffer(screenFBO);
    glClear(GL_COLOR_BUFFER_BIT);

    //Update the queue of views needing update
    unsigned int updateCount = 0;
    std::vector<unsigned int> viewsNoUpdate;
    for(int i=content->getViewsCount()-1; i >= 0; --i)
    {
        OpenGLView* view = content->getView(i);
        if(view->needsUpdate())
        {
            if(view->isContinuous())
            {
                viewsQueue.push_front(i);
                ++updateCount;
            }
            else
            {
                viewsQueue.push_back(i);
                viewsNoUpdate.push_back(i);
            }
        }
        else
        {
            viewsNoUpdate.push_back(i);
        }
    }
    
    if(viewsQueue.size() > content->getViewsCount())
    {
        updateCount = viewsQueue.size();
        viewsNoUpdate.clear();
    }
    else if(updateCount < (unsigned int)viewsQueue.size())
    {
        ++updateCount;
        viewsNoUpdate.erase(std::find(viewsNoUpdate.begin(), viewsNoUpdate.end(), viewsQueue[updateCount-1]));
    }
   
    //Loop through all views -> trackballs, cameras, depth cameras...
    for(unsigned int i=0; i<updateCount; ++i)
    {
        OpenGLState::EnableDepthTest();
        OpenGLState::EnableCullFace();
        OpenGLState::DisableBlend();
        OpenGLView* view = content->getView(viewsQueue[i]);
            
        if(view->getType() == ViewType::DEPTH_CAMERA)
        {
            OpenGLDepthCamera* camera = static_cast<OpenGLDepthCamera*>(view);
            //Draw objects and compute depth data
            camera->ComputeOutput(drawingQueueCopy);
            //Draw camera output
            camera->DrawLDR(screenFBO, true);
        }
        else if(view->getType() == ViewType::SONAR)
        {
            OpenGLSonar* sonar = static_cast<OpenGLSonar*>(view);
            //Draw objects and compute sonar data
            sonar->ComputeOutput(drawingQueueCopy);
            //Draw sonar output
            sonar->DrawLDR(screenFBO, true);
        }
        else if(view->getType() == ViewType::CAMERA || view->getType() == ViewType::TRACKBALL)
        {
            //Apply view properties
            OpenGLCamera* camera = static_cast<OpenGLCamera*>(view);
            OpenGLLight::SetCamera(camera);
            GLint* viewport = camera->GetViewport();
            content->SetViewportSize(viewport[2],viewport[3]);
        
            //Bake parallel-split shadowmaps for sun
            if(rSettings.shadows > RenderQuality::DISABLED)
            {
                content->SetDrawingMode(DrawingMode::SHADOW);
                atm->getOpenGLAtmosphere()->BakeShadowmaps(this, camera);
            }

            atm->getOpenGLAtmosphere()->SetupMaterialShaders();
        
            //Clear main framebuffer and setup camera
            OpenGLState::BindFramebuffer(camera->getRenderFBO());
            camera->SetRenderBuffers(0, true, false);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            
            camera->SetViewport();
            content->SetCurrentView(camera);
            
            //Draw scene
            if(renderMode == 0) //NO OCEAN
            {
                //Render all objects
                content->SetDrawingMode(DrawingMode::FULL);
                DrawObjects();
                DrawLights();

                //Ambient occlusion
                if(rSettings.ao > RenderQuality::DISABLED)
                    camera->DrawAO(1.0f);
                
                //Render sky (at the end to take profit of early bailing)
                atm->getOpenGLAtmosphere()->DrawSkyAndSun(camera);
            }
            else if(renderMode == 1) //OCEAN
            {
                OpenGLOcean* glOcean = ocean->getOpenGLOcean();
                
                //Update ocean for this camera
                if(ocean->hasWaves())
                    glOcean->UpdateSurface(camera);

                //Two separate rendering paths: above water and under water, 
                //possible because camera near plane is (virtually) removed with logarithmic depth buffer.
                glm::vec3 eye = camera->GetEyePosition();
                if(ocean->GetDepth(eye) > 0.0) //Underwater
                {  
                    content->SetDrawingMode(DrawingMode::UNDERWATER);
                    DrawObjects();
                    glOcean->DrawBackground(camera);
                    glOcean->DrawBacksurface(camera);
                    //camera->GenerateBloom();
                    DrawLights();
                    
                    if(rSettings.ssr > RenderQuality::DISABLED)
                    {
                        //Linear depth front faces
                        camera->GenerateLinearDepth(true);
                        
                        //Linear depth back faces
                        OpenGLState::BindFramebuffer(camera->getPostprocessFBO());
                        glClear(GL_DEPTH_BUFFER_BIT);
                        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                        glCullFace(GL_FRONT);
                        content->SetDrawingMode(DrawingMode::FLAT);
                        DrawObjects();
                        glCullFace(GL_BACK);
                        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                        camera->GenerateLinearDepth(false);
                        
                        //Draw screen-space reflections
                        camera->DrawSSR();
                    }

                    //Draw bloom effect simulating scattering
                    //camera->DrawBloom((GLfloat)ocean->getWaterType());

                    //Suspended particles only below surface
                    glDepthMask(GL_FALSE);
                    glOcean->DrawParticles(camera);
                    glDepthMask(GL_TRUE);
                    
                }
                else //Above water
                {
                    content->SetDrawingMode(DrawingMode::UNDERWATER);
                    DrawObjects();
                    DrawLights();
                    glOcean->DrawBackground(camera);

                    //Draw surface to back buffer
                    camera->SetRenderBuffers(1, false, true); //Clearing color buffer
                    camera->SetRenderBuffers(1, true, false); //Color + Normal
                    glOcean->DrawSurface(camera);
                    camera->SetRenderBuffers(0, false, false); //Color only
                    
                    //Blend surface on top of scene
                    OpenGLState::DisableDepthTest();
                    OpenGLState::EnableBlend();
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    content->DrawTexturedSAQ(camera->getColorTexture(1));
                    OpenGLState::DisableBlend();
                    OpenGLState::EnableDepthTest();
                    
                    //Draw all objects as above surface 
                    //(depth testing will secure drawing only what is above water)
                    camera->SetRenderBuffers(0, true, false); //Color + Normal
                    content->SetDrawingMode(DrawingMode::FULL);
                    DrawObjects();
                    DrawLights();
                
                    //Render sky (left for the end to only fill empty spaces)
                    atm->getOpenGLAtmosphere()->DrawSkyAndSun(camera);    

                     //Postprocess
                    if(rSettings.ssr > RenderQuality::DISABLED)
                    {
                        //Linear depth front faces
                        camera->GenerateLinearDepth(true);
                    
                        //Linear depth back faces
                        OpenGLState::BindFramebuffer(camera->getPostprocessFBO());
                        glClear(GL_DEPTH_BUFFER_BIT);
                        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                        glCullFace(GL_FRONT);
                        content->SetDrawingMode(DrawingMode::FLAT);
                        DrawObjects();
                        glCullFace(GL_BACK);
                        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                        camera->GenerateLinearDepth(false);
                        
                        //Draw screen-space reflections
                        camera->DrawSSR();
                    }
                }
            }
        
            //Tone mapping
            camera->DrawLDR(screenFBO, true);
        
            //Helper objects
            if(camera->getType() == ViewType::TRACKBALL)
            {
                //Overlay debugging info
                OpenGLState::BindFramebuffer(screenFBO); //No depth buffer, just one color buffer
                content->SetProjectionMatrix(camera->GetProjectionMatrix());
                content->SetViewMatrix(camera->GetViewMatrix());
                OpenGLState::DisableCullFace();
                
                //Simulation debugging
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                //if(sim->getSolidDisplayMode() == DisplayMode::PHYSICAL) DrawObjects();
                DrawHelpers();
                if(hSettings.showOceanVelocityField && ocean != NULL) ocean->getOpenGLOcean()->DrawVelocityField(camera, 5.f);
                if(hSettings.showBulletDebugInfo) sim->RenderBulletDebug();
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                
                //Overlay selection outline
                ((OpenGLTrackball*)camera)->DrawSelection(selectedDrawingQueueCopy, screenFBO);
                 
                //Graphics debugging
                //if(ocean != NULL)
                //{
                    //ocean->getOpenGLOcean()->ShowSpectrum(glm::vec2(512.f,512.f), glm::vec4(0.f,0.f,512.f,512.f));
                    //ocean->getOpenGLOcean()->ShowTexture(3, glm::vec4(0,0,512,512));
                //}
        
                //atm->getOpenGLAtmosphere()->ShowSunShadowmaps(0, 0, 0.1f);
                
                //content->DrawTexturedQuad(0,0,800,600,camera->getPostprocessTexture(0));
                //camera->ShowViewNormalTexture(glm::vec4(0,0,400,300));
                //camera->ShowSceneTexture(glm::vec4(0,200,300,200));
                //camera->ShowLinearDepthTexture(glm::vec4(0,0,600,500), true);
                //camera->ShowLinearDepthTexture(glm::vec4(0,400,300,200), false);
                
                //camera->ShowDeinterleavedDepthTexture(glm::vec4(0,400,300,200), 0);
                //camera->ShowDeinterleavedDepthTexture(glm::vec4(0,400,300,200), 8);
                //camera->ShowDeinterleavedDepthTexture(glm::vec4(0,600,300,200), 9);
                //camera->ShowDeinterleavedAOTexture(glm::vec4(0,600,300,200), 0);
                //camera->ShowAmbientOcclusion(glm::vec4(0,800,300,200));
                //camera->ShowDepthStencilTexture(glm::vec4(0,400,300,200)); 
                                
                OpenGLState::BindFramebuffer(0);
            }
        
            delete [] viewport;
        }
    }
    //Draw views that are displayed but not updated
    for(size_t i=0; i<viewsNoUpdate.size(); ++i)
        content->getView(viewsNoUpdate[i])->DrawLDR(screenFBO, false);
    //Remove views drawn in this frame
    viewsQueue.erase(viewsQueue.begin(), viewsQueue.begin() + updateCount);
}

}
