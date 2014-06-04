//
//  OpenGLPipeline.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "OpenGLPipeline.h"

#include "SimulationManager.h"
#include "GLSLShader.h"
#include "OpenGLSolids.h"
#include "OpenGLGBuffer.h"
#include "OpenGLView.h"
#include "OpenGLSky.h"
#include "OpenGLSun.h"
#include "OpenGLLight.h"
#include "Console.h"

OpenGLPipeline* OpenGLPipeline::instance = NULL;

OpenGLPipeline* OpenGLPipeline::getInstance()
{
    if(instance == NULL)
        instance = new OpenGLPipeline();
    
    return instance;
}

OpenGLPipeline::OpenGLPipeline()
{
}

OpenGLPipeline::~OpenGLPipeline()
{
    OpenGLSolids::Destroy();
    OpenGLLight::Destroy();
    OpenGLSun::Destroy();
    OpenGLSky::Destroy();
    OpenGLSky::Destroy();
    OpenGLGBuffer::DeleteShaders();
}

bool OpenGLPipeline::isFluidRendered()
{
    return renderFluid;
}

bool OpenGLPipeline::isSAORendered()
{
    return renderSAO;
}

void OpenGLPipeline::Initialize(SimulationManager* sim)
{
    simulation = sim;
    
    //Load shaders and create rendering buffers
    cInfo("Loading shaders...");
    OpenGLSolids::Init();
    GLSLShader::Init(); //check if shaders available!!!
    OpenGLGBuffer::LoadShaders();
    OpenGLView::Init();
    OpenGLSky::Init();
    OpenGLSun::Init();
    OpenGLLight::Init();
    
    cInfo("Generating sky...");
    OpenGLSky::Generate(20.f,30.f);
    
    //Set default options
    cInfo("Setting up basic OpenGL parameters...");
    SetRenderingEffects(true, true, true, true);
    SetVisibleElements(true, true, true, true, false);
    
    //OpenGL flags and params
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glShadeModel(GL_SMOOTH);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glDisable(GL_LIGHTING);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glPointSize(5.f);
    glLineWidth(2.0f);
    glLineStipple(3, 0xE4E4);
    
    cInfo("OpenGL pipeline initialized.");
}

void OpenGLPipeline::SetRenderingEffects(bool sky, bool shadows, bool fluid, bool ssao)
{
    renderSky = sky;
    renderShadows = shadows;
    renderFluid = fluid;
    renderSAO = ssao;
}

void OpenGLPipeline::SetVisibleElements(bool coordSystems, bool joints, bool actuators, bool sensors, bool stickers)
{
    showCoordSys = coordSystems;
    showJoints = joints;
    showActuators = actuators;
    showSensors = sensors;
    showStickers = stickers;
}

void OpenGLPipeline::DrawStandardObjects()
{
    for(int h=0; h<simulation->entities.size(); h++) //Render entities
    {
        simulation->entities[h]->Render();
    }
}

void OpenGLPipeline::DrawSpecialObjects()
{
    
}

void OpenGLPipeline::Render()
{
    //Move lights attached to rigid bodies
    for(int i=0; i<simulation->lights.size(); i++)
    {
        simulation->lights[i]->UpdateLight();
        
        if(renderShadows)
            simulation->lights[i]->RenderShadowMap(this);
    }
    
    //Render all camera views
    for(int i=0; i<simulation->views.size(); i++)
    {
        if(simulation->views[i]->isActive()) //Render only if active
        {
            GLuint finalTexture = 0;
            
            //Setup and initialize lighting
            OpenGLSun::SetCamera(simulation->views[i]);
            if(renderShadows)
                OpenGLSun::RenderShadowMaps(this);
            OpenGLLight::SetCamera(simulation->views[i]);
            
            //Setup viewport
            GLint* viewport = simulation->views[i]->GetViewport();
            simulation->views[i]->SetViewport();
            
            //Fill plain G-buffer
            simulation->views[i]->getGBuffer()->Start(0);
            simulation->views[i]->SetProjection();
            simulation->views[i]->SetViewTransform();
            DrawStandardObjects();
            simulation->views[i]->getGBuffer()->Stop();
            
            //Prepare SAO
            if(renderSAO && simulation->views[i]->hasSSAO())
            {
                glActiveTextureARB(GL_TEXTURE0_ARB);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getGBuffer()->getNormalsTexture(0));
                
                glActiveTextureARB(GL_TEXTURE1_ARB);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, OpenGLView::getRandomTexture());
    
                glActiveTextureARB(GL_TEXTURE2_ARB);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getGBuffer()->getPositionTexture(0));
                
                OpenGLView::SetTextureUnits(2, 0, 1);
                simulation->views[i]->RenderSSAO();
            }
            
            if(!renderFluid || simulation->fluid == NULL)
            /////////////////////////////// N O R M A L  P I P E L I N E //////////////////////////////
            {
            normal_pipeline:
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, simulation->views[i]->getSceneFBO());
                glDrawBuffer(SCENE_ATTACHMENT);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                //1. Create stencil mask
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_STENCIL_TEST);
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                glDisable(GL_BLEND);
                
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                glStencilMask(0xFF);
                glClear(GL_STENCIL_BUFFER_BIT);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                
                simulation->views[i]->SetProjection();
                btScalar openglTrans[16];
                simulation->views[i]->GetViewTransform().getOpenGLMatrix(openglTrans);
                glMatrixMode(GL_MODELVIEW);
#ifdef BT_USE_DOUBLE_PRECISION
                glLoadMatrixd(openglTrans);
#else
                glLoadMatrixf(openglTrans);
#endif
                DrawStandardObjects();
                
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                glStencilMask(0x00);
                
                //2. Enter deferred rendering
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_CULL_FACE);
                
                OpenGLSolids::SetupOrtho();
                
                //3. Draw sky where stencil = 0
                if(renderSky)
                {
                    glStencilFunc(GL_EQUAL, 0, 0xFF);
                    OpenGLSky::Render(simulation->views[i], simulation->views[i]->GetViewTransform(), simulation->zUp);
                }
                
                //4. Bind deferred textures to texture units
                glStencilFunc(GL_EQUAL, 1, 0xFF);
                
                glActiveTextureARB(GL_TEXTURE0_ARB);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getGBuffer()->getDiffuseTexture()); //Diffuse is reused
                
                glActiveTextureARB(GL_TEXTURE1_ARB);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getGBuffer()->getPositionTexture(0));
                
                glActiveTextureARB(GL_TEXTURE2_ARB);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getGBuffer()->getNormalsTexture(0));
                
                glActiveTextureARB(GL_TEXTURE3_ARB);
                glEnable(GL_TEXTURE_CUBE_MAP);
                glBindTexture(GL_TEXTURE_CUBE_MAP, OpenGLSky::getDiffuseCubemap());
                
                glActiveTextureARB(GL_TEXTURE4_ARB);
                glEnable(GL_TEXTURE_CUBE_MAP);
                glBindTexture(GL_TEXTURE_CUBE_MAP, OpenGLSky::getReflectionCubemap());
                
                //5. Bind SSAO texture if needed
                glActiveTextureARB(GL_TEXTURE5_ARB);
                glEnable(GL_TEXTURE_2D);
                if(renderSAO && simulation->views[i]->hasSSAO())
                    glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getSSAOTexture());
                else
                    glBindTexture(GL_TEXTURE_2D, 0);
                
                OpenGLSun::SetTextureUnits(0, 2, 1, 6);
                OpenGLLight::SetTextureUnits(0, 2, 1, 3, 4, 5, 6);
                
                //5. Render ambient pass - sky, ssao
                OpenGLLight::RenderAmbientLight(simulation->views[i]->GetViewTransform(), simulation->zUp);
                
                //6. Render lights
                glEnable(GL_BLEND);
                glBlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_ONE, GL_ONE); //accumulate light
                
                OpenGLSun::Render(simulation->views[i]->GetViewTransform()); //Render sun pass
                
                for(int h=0; h<simulation->lights.size(); h++) //Render light passes
                    simulation->lights[h]->Render();
                
                //7. Reset OpenGL
                glUseProgramObjectARB(0);
                glDisable(GL_STENCIL_TEST);
                
                glActiveTextureARB(GL_TEXTURE0_ARB);
                glBindTexture(GL_TEXTURE_2D, 0);
                
                glActiveTextureARB(GL_TEXTURE1_ARB);
                glDisable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, 0);
                
                glActiveTextureARB(GL_TEXTURE2_ARB);
                glDisable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, 0);
                
                glActiveTextureARB(GL_TEXTURE3_ARB);
                glDisable(GL_TEXTURE_CUBE_MAP);
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                
                glActiveTextureARB(GL_TEXTURE4_ARB);
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                glDisable(GL_TEXTURE_CUBE_MAP);
                
                glActiveTextureARB(GL_TEXTURE5_ARB);
                glBindTexture(GL_TEXTURE_2D, 0);
                glDisable(GL_TEXTURE_2D);
                
                glActiveTextureARB(GL_TEXTURE6_ARB);
                glBindTexture(GL_TEXTURE_2D, 0);
                glDisable(GL_TEXTURE_2D);
                
                //8. Finish rendering
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                finalTexture = simulation->views[i]->getSceneTexture();
            }
            /////////////////////////////////////////////////////////////////////////////////////
            else
            {
                btVector3 normal, position;
                simulation->fluid->GetSurface(normal, position);
                bool underwater = distanceFromCenteredPlane(normal, position - simulation->views[i]->GetEyePosition()) >= 0;
                
                if(underwater) //Under the surface
                {
                    bool inside = simulation->fluid->IsInsideFluid(simulation->views[i]->GetEyePosition()
                                                                   + simulation->views[i]->GetLookingDirection() * simulation->views[i]->GetNearClip());
                    if(inside)
            ////////////////////// U N D E R W A T E R  P I P E L I N E /////////////////////////
                    {
                    
                        
                        
                        
                        
                        
                        
                       
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                    }
            /////////////////////////////////////////////////////////////////////////////////////
                    else
                        goto normal_pipeline;
                }
                else
            //////////////////////// A B O V E  W A T E R  P I P E L I N E //////////////////////
                {
                    /////////// Render normal scene with special stencil masking ////////////////
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, simulation->views[i]->getSceneFBO());
                    glDrawBuffer(SCENE_ATTACHMENT);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    
                    //1. Create stencil mask
                    glEnable(GL_DEPTH_TEST);
                    glEnable(GL_STENCIL_TEST);
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_BACK);
                    glDisable(GL_BLEND);
                    
                    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                    glStencilMask(0xFF);
                    glClear(GL_STENCIL_BUFFER_BIT);
                    
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    simulation->views[i]->SetProjection();
                    btScalar openglTrans[16];
                    simulation->views[i]->GetViewTransform().getOpenGLMatrix(openglTrans);
                    glMatrixMode(GL_MODELVIEW);
#ifdef BT_USE_DOUBLE_PRECISION
                    glLoadMatrixd(openglTrans);
#else
                    glLoadMatrixf(openglTrans);
#endif
                    //Render normal objects and set stencil to 1
                    glStencilFunc(GL_ALWAYS, 1, 0xFF);
                    DrawStandardObjects();
                    //Render fluid surface and change stencil to 2
                    glStencilFunc(GL_ALWAYS, 2, 0xFF);
                    simulation->fluid->RenderSurface();
                    
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                    glStencilMask(0x00);
                    
                    //2. Enter deferred rendering
                    glDisable(GL_DEPTH_TEST);
                    glDisable(GL_CULL_FACE);
                    
                    OpenGLSolids::SetupOrtho();
                    
                    //3. Draw sky where stencil = 0
                    if(renderSky)
                    {
                        glStencilFunc(GL_EQUAL, 0, 0xFF);
                        OpenGLSky::Render(simulation->views[i], simulation->views[i]->GetViewTransform(), simulation->zUp);
                    }
                    
                    //4. Bind deferred textures to texture units
                    glStencilFunc(GL_EQUAL, 1, 0xFF);
                    
                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getGBuffer()->getDiffuseTexture()); //Diffuse is reused
                    
                    glActiveTextureARB(GL_TEXTURE1_ARB);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getGBuffer()->getPositionTexture(0));
                    
                    glActiveTextureARB(GL_TEXTURE2_ARB);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getGBuffer()->getNormalsTexture(0));
                    
                    glActiveTextureARB(GL_TEXTURE3_ARB);
                    glEnable(GL_TEXTURE_CUBE_MAP);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, OpenGLSky::getDiffuseCubemap());
                    
                    glActiveTextureARB(GL_TEXTURE4_ARB);
                    glEnable(GL_TEXTURE_CUBE_MAP);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, OpenGLSky::getReflectionCubemap());
                    
                    //5. Bind SSAO texture if needed
                    glActiveTextureARB(GL_TEXTURE5_ARB);
                    glEnable(GL_TEXTURE_2D);
                    if(renderSAO && simulation->views[i]->hasSSAO())
                        glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getSSAOTexture());
                    else
                        glBindTexture(GL_TEXTURE_2D, 0);
                    
                    OpenGLSun::SetTextureUnits(0, 2, 1, 6);
                    OpenGLLight::SetTextureUnits(0, 2, 1, 3, 4, 5, 6);
                    
                    //5. Render ambient pass - sky, ssao
                    OpenGLLight::RenderAmbientLight(simulation->views[i]->GetViewTransform(), simulation->zUp);
                    
                    glEnable(GL_BLEND);
                    glBlendEquation(GL_FUNC_ADD);
                    glBlendFunc(GL_ONE, GL_ONE); //accumulate light
                    
                    //6. Render lights
                    OpenGLSun::Render(simulation->views[i]->GetViewTransform()); //Render sun pass
                    
                    for(int h=0; h<simulation->lights.size(); h++) //Render light passes
                        simulation->lights[h]->Render();
                    
                    //7. Reset OpenGL
                    glUseProgramObjectARB(0);
                    glDisable(GL_STENCIL_TEST);
                    
                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    
                    glActiveTextureARB(GL_TEXTURE1_ARB);
                    glDisable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    
                    glActiveTextureARB(GL_TEXTURE2_ARB);
                    glDisable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    
                    glActiveTextureARB(GL_TEXTURE3_ARB);
                    glDisable(GL_TEXTURE_CUBE_MAP);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                    
                    glActiveTextureARB(GL_TEXTURE4_ARB);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                    glDisable(GL_TEXTURE_CUBE_MAP);
                    
                    glActiveTextureARB(GL_TEXTURE5_ARB);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glDisable(GL_TEXTURE_2D);
                    
                    glActiveTextureARB(GL_TEXTURE6_ARB);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glDisable(GL_TEXTURE_2D);
                    
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                    
                    //////////////// Fill reflected G-buffer ////////////////////////
                    double surface[4];
                    surface[0] = normal.x();
                    surface[1] = normal.y();
                    surface[2] = normal.z();
                    surface[3] = -normal.dot(position);
                    
                    simulation->views[i]->getGBuffer()->Start(1);
                    simulation->views[i]->SetProjection();
                    simulation->views[i]->SetReflectedViewTransform(simulation->fluid);
                    DrawStandardObjects();
                    simulation->views[i]->getGBuffer()->Stop();
                    
                    ///////////////// Render reflected scene with special stencil masking /////////////////////
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, simulation->views[i]->getSceneFBO());
                    glDrawBuffer(REFLECTION_ATTACHMENT);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    
                    //1. Create stencil mask
                    glEnable(GL_DEPTH_TEST);
                    glEnable(GL_STENCIL_TEST);
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_BACK);
                    glDisable(GL_BLEND);
                    
                    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                    glStencilFunc(GL_ALWAYS, 0, 0xFF);
                    glStencilMask(0xFF);
                    glClear(GL_STENCIL_BUFFER_BIT);
                    
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    simulation->views[i]->SetProjection();
                    simulation->views[i]->GetReflectedViewTransform(simulation->fluid).getOpenGLMatrix(openglTrans);
                    glMatrixMode(GL_MODELVIEW);
#ifdef BT_USE_DOUBLE_PRECISION
                    glLoadMatrixd(openglTrans);
#else
                    glLoadMatrixf(openglTrans);
#endif
                    //Render fluid surface and set stencil = 1
                    glDepthMask(GL_FALSE);
                    glDisable(GL_CULL_FACE);
                    glStencilFunc(GL_ALWAYS, 1, 0xFF);
                    simulation->fluid->RenderSurface();
                    glDepthMask(GL_TRUE);
                    
                    //Render clipped normal objects and set stencil = 2
                    glStencilFunc(GL_ALWAYS, 2, 0xFF);
                    glClipPlane(GL_CLIP_PLANE0, surface);
                    glEnable(GL_CLIP_PLANE0);
                    DrawStandardObjects();
                    glDisable(GL_CLIP_PLANE0);
                    
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                    glStencilMask(0x00);
                    
                    //2. Enter deferred rendering
                    glDisable(GL_DEPTH_TEST);
                    
                    OpenGLSolids::SetupOrtho();
                    
                    //3. Draw sky where stencil = 1 -> only surface rendered there
                    if(renderSky)
                    {
                        glStencilFunc(GL_EQUAL, 1, 0xFF);
                        OpenGLSky::Render(simulation->views[i], simulation->views[i]->GetReflectedViewTransform(simulation->fluid), simulation->zUp);
                    }
                    
                    //4. Bind deferred textures to texture units
                    glStencilFunc(GL_EQUAL, 2, 0xFF); //Draw objects where not only surface was drawn
                    
                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getGBuffer()->getDiffuseTexture()); //Diffuse is reused
                    
                    glActiveTextureARB(GL_TEXTURE1_ARB);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getGBuffer()->getPositionTexture(1));
                    
                    glActiveTextureARB(GL_TEXTURE2_ARB);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getGBuffer()->getNormalsTexture(1));
                    
                    glActiveTextureARB(GL_TEXTURE3_ARB);
                    glEnable(GL_TEXTURE_CUBE_MAP);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, OpenGLSky::getDiffuseCubemap());
                    
                    glActiveTextureARB(GL_TEXTURE4_ARB);
                    glEnable(GL_TEXTURE_CUBE_MAP);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, OpenGLSky::getReflectionCubemap());
                    
                    //5. Bind empty texture as SAO - no SAO on reflections
                    glActiveTextureARB(GL_TEXTURE5_ARB);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    
                    OpenGLSun::SetTextureUnits(0, 2, 1, 6);
                    OpenGLLight::SetTextureUnits(0, 2, 1, 3, 4, 5, 6);
                    
                    //5. Render ambient pass - sky
                    OpenGLLight::RenderAmbientLight(simulation->views[i]->GetReflectedViewTransform(simulation->fluid), simulation->zUp);
                    
                    glEnable(GL_BLEND);
                    glBlendEquation(GL_FUNC_ADD);
                    glBlendFunc(GL_ONE, GL_ONE); //accumulate light
                    
                    //6. Render lights
                    OpenGLSun::Render(simulation->views[i]->GetReflectedViewTransform(simulation->fluid)); //Render sun pass
                    
                    for(int h=0; h<simulation->lights.size(); h++) //Render light passes
                        simulation->lights[h]->Render();
                    
                    //7. Reset OpenGL
                    glUseProgramObjectARB(0);
                    glDisable(GL_STENCIL_TEST);
                    
                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    
                    glActiveTextureARB(GL_TEXTURE1_ARB);
                    glDisable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    
                    glActiveTextureARB(GL_TEXTURE2_ARB);
                    glDisable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    
                    glActiveTextureARB(GL_TEXTURE3_ARB);
                    glDisable(GL_TEXTURE_CUBE_MAP);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                    
                    glActiveTextureARB(GL_TEXTURE4_ARB);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                    glDisable(GL_TEXTURE_CUBE_MAP);
                    
                    glActiveTextureARB(GL_TEXTURE5_ARB);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glDisable(GL_TEXTURE_2D);
                    
                    glActiveTextureARB(GL_TEXTURE6_ARB);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glDisable(GL_TEXTURE_2D);
                    
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                    
                    ////////////////// Fill refracted G-buffer //////////////////////////////////////
                    surface[0] = -surface[0];
                    surface[1] = -surface[1];
                    surface[2] = -surface[2];
                    surface[3] = -surface[3];
                    
                    simulation->views[i]->getGBuffer()->Start(1);
                    simulation->views[i]->SetProjection();
                    simulation->views[i]->SetRefractedViewTransform(simulation->fluid);
                    DrawStandardObjects();
                    simulation->views[i]->getGBuffer()->Stop();
           
                    ///////////////// Render refracted scene with special stencil masking /////////////////////
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, simulation->views[i]->getSceneFBO());
                    glDrawBuffer(REFRACTION_ATTACHMENT);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    
                    //1. Create stencil mask
                    glEnable(GL_DEPTH_TEST);
                    glEnable(GL_STENCIL_TEST);
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_BACK);
                    
                    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                    glStencilFunc(GL_ALWAYS, 0, 0xFF);
                    glStencilMask(0xFF);
                    glClear(GL_STENCIL_BUFFER_BIT);
                    
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    simulation->views[i]->SetProjection();
                    simulation->views[i]->GetRefractedViewTransform(simulation->fluid).getOpenGLMatrix(openglTrans);
                    glMatrixMode(GL_MODELVIEW);
#ifdef BT_USE_DOUBLE_PRECISION
                    glLoadMatrixd(openglTrans);
#else
                    glLoadMatrixf(openglTrans);
#endif
                    //Render normal objects just to fill depth buffer
                    //glClipPlane(GL_CLIP_PLANE0, surface);
                    //glEnable(GL_CLIP_PLANE0);
                    DrawStandardObjects();
                    //glDisable(GL_CLIP_PLANE0);
                    //Render fluid surface and change stencil to 1
                    glStencilFunc(GL_ALWAYS, 1, 0xFF);
                    simulation->fluid->RenderSurface();
                    
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                    glStencilMask(0x00);
                    
                    //2. Enter deferred rendering
                    glDisable(GL_DEPTH_TEST);
                    glDisable(GL_CULL_FACE);
                    
                    OpenGLSolids::SetupOrtho();
                    
                    //3. Bind deferred textures to texture units
                    glStencilFunc(GL_EQUAL, 1, 0xFF);
                    glEnable(GL_BLEND);
                    glBlendEquation(GL_FUNC_ADD);
                    glBlendFunc(GL_ONE, GL_ONE); //accumulate light
                    
                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getGBuffer()->getDiffuseTexture()); //Diffuse is reused
                    
                    glActiveTextureARB(GL_TEXTURE1_ARB);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getGBuffer()->getPositionTexture(1));
                    
                    glActiveTextureARB(GL_TEXTURE2_ARB);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getGBuffer()->getNormalsTexture(1));
                    
                    glActiveTextureARB(GL_TEXTURE3_ARB);
                    glEnable(GL_TEXTURE_CUBE_MAP);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, OpenGLSky::getDiffuseCubemap());
                    
                    glActiveTextureARB(GL_TEXTURE4_ARB);
                    glEnable(GL_TEXTURE_CUBE_MAP);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, OpenGLSky::getReflectionCubemap());
                    
                    //5. Bind empty texture - no SAO
                    glActiveTextureARB(GL_TEXTURE5_ARB);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    
                    OpenGLSun::SetTextureUnits(0, 2, 1, 6);
                    OpenGLLight::SetTextureUnits(0, 2, 1, 3, 4, 5, 6);
                    
                    //5. Render ambient pass - sky, ssao
                    OpenGLLight::RenderAmbientLight(simulation->views[i]->GetRefractedViewTransform(simulation->fluid), simulation->zUp);
                    
                    //6. Render lights
                    OpenGLSun::Render(simulation->views[i]->GetRefractedViewTransform(simulation->fluid)); //Render sun pass
                    
                    for(int h=0; h<simulation->lights.size(); h++) //Render light passes
                        simulation->lights[h]->Render();
                    
                    //7. Reset OpenGL
                    glUseProgramObjectARB(0);
                    glDisable(GL_STENCIL_TEST);
                    
                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    
                    glActiveTextureARB(GL_TEXTURE1_ARB);
                    glDisable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    
                    glActiveTextureARB(GL_TEXTURE2_ARB);
                    glDisable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    
                    glActiveTextureARB(GL_TEXTURE3_ARB);
                    glDisable(GL_TEXTURE_CUBE_MAP);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                    
                    glActiveTextureARB(GL_TEXTURE4_ARB);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                    glDisable(GL_TEXTURE_CUBE_MAP);
                    
                    glActiveTextureARB(GL_TEXTURE5_ARB);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glDisable(GL_TEXTURE_2D);
                    
                    glActiveTextureARB(GL_TEXTURE6_ARB);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glDisable(GL_TEXTURE_2D);
                    
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                
                    ////////////////// Render water surface using last stencil mask ////////////////
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, simulation->views[i]->getSceneFBO());
                    glDrawBuffer(SCENE_ATTACHMENT);
                    glEnable(GL_STENCIL_TEST);
                    glStencilMask(0x00);
                    glStencilFunc(GL_EQUAL, 1, 0xFF);
                    
                    //Render surface without light
                    simulation->views[i]->RenderFluidSurface(simulation->fluid, false);

                    //Accumulate light
                    
                    
                    
                    
                    glDisable(GL_STENCIL_TEST);
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

                    //////////////// Finish rendering /////////////////////////
                    finalTexture = simulation->views[i]->getSceneTexture();
                }
            /////////////////////////////////////////////////////////////////////////////////////
            }
            
            /*
            
            //Render normal scene
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, simulation->views[i]->getSceneFBO());
            glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT); //SceneTexture
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            RenderView(simulation->views[i], 0, simulation->views[i]->GetViewTransform());
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
            
            finalTexture = simulation->views[i]->getSceneTexture();
            
            //Render fluid
            if((simulation->fluid != NULL) && renderFluid)
            {
                btVector3 surfN, surfP;
                simulation->fluid->GetSurface(surfN, surfP);
                bool underwater = distanceFromCenteredPlane(surfN, surfP - simulation->views[i]->GetEyePosition()) >= 0;
                
                if(underwater) //Under the surface
                {
                    bool inside = simulation->fluid->IsInsideFluid(simulation->views[i]->GetEyePosition()
                                                                   + simulation->views[i]->GetLookingDirection() * simulation->views[i]->GetNearClip());
                    if(inside) //Inside the fluid
                    {
                        double surface[4];
                        surface[0] = -surfN.x();
                        surface[1] = -surfN.y();
                        surface[2] = -surfN.z();
                        surface[3] = surfN.dot(surfP);
                        
                        //Fill reflected G-buffer
                        simulation->views[i]->getGBuffer()->Start(1);
                        simulation->views[i]->SetProjection();
                        simulation->views[i]->SetReflectedViewTransform(simulation->fluid);
                        DrawStandardObjects();
                        simulation->views[i]->getGBuffer()->Stop();
                        
                        //Render reflected scene
                        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, simulation->views[i]->getSceneFBO());
                        glDrawBuffer(GL_COLOR_ATTACHMENT2_EXT); //ReflectionTexture
                        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        RenderView(simulation->views[i], 1, simulation->views[i]->GetReflectedViewTransform(simulation->fluid), surface);
                        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                        
                        //Fill refracted G-buffer
                        surface[0] = -surface[0];
                        surface[1] = -surface[1];
                        surface[2] = -surface[2];
                        surface[3] = -surface[3];
                        
                        simulation->views[i]->getGBuffer()->Start(1);
                        simulation->views[i]->SetProjection();
                        simulation->views[i]->SetRefractedViewTransform(simulation->fluid);
                        DrawStandardObjects();
                        simulation->views[i]->getGBuffer()->Stop();
                        
                        //Render refracted scene
                        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, simulation->views[i]->getSceneFBO());
                        glDrawBuffer(GL_COLOR_ATTACHMENT3_EXT); //RefractionTexture
                        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        RenderView(simulation->views[i], 1, simulation->views[i]->GetRefractedViewTransform(simulation->fluid), surface);
                        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                       
                        //Render water surface on top of scene texture
                        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, simulation->views[i]->getSceneFBO());
                        glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT); //SceneTexture
                        simulation->views[i]->RenderFluidSurface(simulation->fluid, true);
                        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                        
                        //Render underwater fog
                        ///glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, simulation->views[i]->getSceneFBO());
                        //glDrawBuffer(GL_COLOR_ATTACHMENT2_EXT); //ReflectionTexture
                        //simulation->views[i]->RenderFluidVolume(simulation->fluid); //Fog, light shafts, volumetric lights, blur
                        //glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                        finalTexture = simulation->views[i]->getSceneTexture();//simulation->views[i]->getSceneReflectionTexture();
                    }
                    //else outside the fluid - DO NOTHING
                }
                else //Above the surface
                {
                    double surface[4];
                    surface[0] = surfN.x();
                    surface[1] = surfN.y();
                    surface[2] = surfN.z();
                    surface[3] = -surfN.dot(surfP);
                    
                    //Fill reflected G-buffer
                    simulation->views[i]->getGBuffer()->Start(1);
                    simulation->views[i]->SetProjection();
                    simulation->views[i]->SetReflectedViewTransform(simulation->fluid);
                    DrawStandardObjects();
                    simulation->views[i]->getGBuffer()->Stop();
                    
                    //Render reflected scene
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, simulation->views[i]->getSceneFBO());
                    glDrawBuffer(GL_COLOR_ATTACHMENT2_EXT); //ReflectionTexture
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    RenderView(simulation->views[i], 1, simulation->views[i]->GetReflectedViewTransform(simulation->fluid), surface);
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                    
                    //Fill refracted G-buffer
                    surface[0] = -surface[0];
                    surface[1] = -surface[1];
                    surface[2] = -surface[2];
                    surface[3] = -surface[3];
                    
                    simulation->views[i]->getGBuffer()->Start(1);
                    simulation->views[i]->SetProjection();
                    simulation->views[i]->SetRefractedViewTransform(simulation->fluid);
                    DrawStandardObjects();
                    simulation->views[i]->getGBuffer()->Stop();
                    
                    //Render refracted scene
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, simulation->views[i]->getSceneFBO());
                    glDrawBuffer(GL_COLOR_ATTACHMENT3_EXT); //RefractionTexture
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    RenderView(simulation->views[i], 1, simulation->views[i]->GetRefractedViewTransform(simulation->fluid), surface);
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                    
                    //Render water surface on top of scene texture
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, simulation->views[i]->getSceneFBO());
                    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT); //SceneTexture
                    simulation->views[i]->RenderFluidSurface(simulation->fluid, false);
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                    
                    finalTexture = simulation->views[i]->getSceneTexture();
                }
            }*/
            
            ///////////FINAL TONEMAPPED/DISTORTED RENDER///////
            glEnable(GL_SCISSOR_TEST);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            
            glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
            glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
            simulation->views[i]->RenderHDR();
            
            /////////OVERLAY DUMMIES////////
            glBindTexture(GL_TEXTURE_2D, 0);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_DEPTH_BUFFER_BIT);
            
            simulation->views[i]->SetProjection();
            simulation->views[i]->SetViewTransform();
            
            //Coordinate systems
            //OpenGLSolids::DrawCoordSystem(2.f);
            
            if(showCoordSys)
            {
                for(int h=0; h<simulation->entities.size(); h++)
                    if(simulation->entities[h]->getType() == ENTITY_SOLID)
                    {
                        SolidEntity* solid = (SolidEntity*)simulation->entities[h];
                        btTransform comT = solid->getTransform();
                        btScalar oglComT[16];
                        comT.getOpenGLMatrix(oglComT);
                        
                        glPushMatrix();
#ifdef BT_USE_DOUBLE_PRECISION
                        glMultMatrixd(oglComT);
#else
                        glMultMatrixf(oglComT);
#endif
                        OpenGLSolids::DrawCoordSystem(0.1f);
                        glPopMatrix();
                    }
            }
            
            //Joints, sensors and actuators
            std::vector<Sticker> stickers;
            
            //Joints
            if(showJoints)
            {
                for(int h=0; h<simulation->joints.size(); h++)
                    if(simulation->joints[h]->isRenderable())
                    {
                        Sticker stick;
                        stick.type = simulation->joints[h]->getType();
                        stick.location = simulation->joints[h]->Render();
                        if(showStickers)
                            stickers.push_back(stick);
                    }
            }
            
            //Contact points
            for(int h = 0; h < simulation->contacts.size(); h++)
                simulation->contacts[h]->Render();
            
            //Sensors
            for(int h = 0; h < simulation->sensors.size(); h++)
                if(simulation->sensors[h]->isRenderable())
                    simulation->sensors[h]->Render();
            
            //Stickers
            glDisable(GL_DEPTH_TEST);
            
            if(showStickers)
            {
                glm::vec4 viewp(viewport[0], viewport[1], viewport[2], viewport[3]);
                glm::mat4 proj = simulation->views[i]->GetProjectionMatrix();
                glm::mat4 model;
                glGetFloatv(GL_MODELVIEW, glm::value_ptr(model));
                
                glMatrixMode(GL_PROJECTION);
                glm::mat4 oproj = glm::ortho(0.f, (GLfloat)viewport[2], 0.f, (GLfloat)viewport[3], -1.f, 1.f);
                glLoadMatrixf(glm::value_ptr(oproj));
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                
                //glEnable(GL_BLEND);
                glBindTexture(GL_TEXTURE_2D, 0);
                
                glColor4f(1.f, 1.f, 1.f, 1.f);
                glBegin(GL_TRIANGLES);
                for(int h=0; h<stickers.size(); h++)
                {
                    btVector3 btobj = stickers[h].location;
                    glm::vec3 obj(btobj.x(), btobj.y(), btobj.z());
                    glm::vec3 point = glm::project(obj, model, proj, viewp);
                    if(point[2] < 1.f) //disable rendering stickers behind camera
                    {
                        glVertex2f(point[0]-10.f, point[1]-10.f);
                        glVertex2f(point[0]-10.f, point[1]+10.f);
                        glVertex2f(point[0]+10.f, point[1]+10.f);
                        glVertex2f(point[0]-10.f, point[1]-10.f);
                        glVertex2f(point[0]+10.f, point[1]+10.f);
                        glVertex2f(point[0]+10.f, point[1]-10.f);
                    }
                }
                glEnd();
            }
            
            glDisable(GL_SCISSOR_TEST);
            
            //Debugging
            //simulation->views[i]->ShowSceneTexture(REFLECTED, 0, 50, 300, 200);
            //simulation->views[i]->ShowSceneTexture(REFRACTED, 0, 250, 300, 200);
            //simulation->views[i]->ShowAmbientOcclusion();
            //simulation->lights[0]->RenderShadowMap(this);
            //simulation->lights[0]->ShowShadowMap(0, 0, 0.5f);
            //OpenGLSun::ShowShadowMaps(0, 0, 0.05);
            //OpenGLSky::ShowCubemap(SKY, 0, 0, 400, 400);
            //OpenGLSky::ShowCubemap(CONVOLUTION_REFLECT, 400, 0, 400, 400);
            //simulation->views[i]->getGBuffer()->ShowTexture(POSITION2, 0,450,300,200); // FBO debugging
            //simulation->views[i]->getGBuffer()->ShowTexture(POSITION2, 0,200,250,200); // FBO debugging
            
            delete viewport;
        }
    }
}