//
//  OpenGLPipeline.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "OpenGLPipeline.h"

#include "SimulationManager.h"
#include "OpenGLSolids.h"
#include "OpenGLGBuffer.h"
#include "OpenGLView.h"
#include "OpenGLSky.h"
#include "OpenGLSun.h"
#include "OpenGLLight.h"

OpenGLPipeline::OpenGLPipeline(SimulationManager* sim)
{
    simulation = sim;
}

OpenGLPipeline::~OpenGLPipeline()
{
    OpenGLLight::Destroy();
    OpenGLSun::Destroy();
    OpenGLSky::Destroy();
    OpenGLSky::Destroy();
    OpenGLGBuffer::DeleteShaders();
}

void OpenGLPipeline::Initialize()
{
    //Load shaders and create rendering buffers
    OpenGLGBuffer::LoadShaders();
    OpenGLView::Init();
    OpenGLSky::Init();
    OpenGLSky::Generate(40.f,0.f);
    OpenGLSun::Init();
    OpenGLLight::Init();
    
    //Set default options
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
    
    printf("OpenGL pipeline initialized.\n");
}

void OpenGLPipeline::SetRenderingEffects(bool sky, bool shadows, bool water, bool ssao)
{
    renderSky = sky;
    renderShadows = shadows;
    renderWater = water;
    renderSSAO = ssao;
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
        simulation->entities[h]->Render();
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
            
            //Render scene
            //SSAO
            if(renderSSAO)
            {
                glActiveTextureARB(GL_TEXTURE0_ARB);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getGBuffer()->getNormalsTexture(0));
                
                glActiveTextureARB(GL_TEXTURE1_ARB);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, OpenGLView::getRandomTexture());
                
                OpenGLView::SetTextureUnits(0, 1);
                simulation->views[i]->RenderSSAO();
            }
            
            //Scene
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, simulation->views[i]->getSceneFBO());
            glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            RenderView(simulation->views[i], simulation->views[i]->GetViewTransform());
            //Render water fog
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
            
            //Render water surface
            if(renderWater)
            {
                for(int h=0; h<simulation->fluids.size(); h++)
                {
                    /*btVector3 surfN, surfP;
                    simulation->fluids[h]->GetSurface(surfN, surfP);
                    
                    //Fill reflected G-buffer
                    simulation->views[i]->getGBuffer()->Start(1);
                    simulation->views[i]->SetProjection();
                    simulation->views[i]->SetReflectedViewTransform(simulation->fluids[h]);
                    
                    for(int j=0; j<simulation->entities.size(); j++) //render simple entities
                        simulation->entities[j]->Render();
                    
                    simulation->views[i]->getGBuffer()->Stop();
                    
                    //Render reflection map
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, simulation->views[i]->getSceneFBO());
                    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                    RenderView(simulation->views[i], simulation->views[i]->GetReflectedViewTransform(surfN, surfP), false);
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                    */
                    //Fill refracted G-buffer
                    /*views[i]->getGBuffer()->Start(1);
                     views[i]->SetProjection();
                     views[i]->SetRefractedViewTransform(fluids[h]);
                     
                     for(int j=0; j<entities.size(); j++) //render simple entities
                     entities[j]->Render();
                     
                     for(int j=0; j<machines.size(); j++) //render machines
                     machines[j]->Render();
                     
                     views[i]->getGBuffer()->Stop();
                     
                     //Render refraction map
                     glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, views[i]->getSceneFBO());
                     glDrawBuffer(GL_COLOR_ATTACHMENT2_EXT);
                     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                     RenderView(views[i], views[i]->GetRefractedViewTransform(surfN, surfP, fluids[h]->getFluid()) ,false);
                     glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);*/
                    
                    /////////Render fluid surface
                    
                    /* glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                     glEnable(GL_STENCIL_TEST);
                     glStencilFunc(GL_ALWAYS, 1, 1);
                     glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                     glDisable(GL_DEPTH_TEST);
                     
                     fluids[i]->Render();
                     
                     glEnable(GL_DEPTH_TEST);
                     glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                     glStencilFunc(GL_EQUAL, 1, 1);
                     glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                     */
                    
                    //Water surface shader
                    //DrawScreenAlignedQuad();*/
                    
                    /*glDisable(GL_DEPTH_TEST);
                    glDisable(GL_CULL_FACE);
                    glDisable(GL_BLEND);
                    //glEnable(GL_BLEND);
                    //glBlendEquation(GL_FUNC_);
                    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, simulation->views[i]->getSceneFBO());
                    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
                    simulation->views[i]->RenderFluid(simulation->fluids[h]);
                    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);*/
                }
            }
            
            //////////FINAL RENDER///////
            glEnable(GL_SCISSOR_TEST);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glActiveTextureARB(GL_TEXTURE0_ARB);
            
            glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
            glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
            SetupOrtho();
            
            glBindTexture(GL_TEXTURE_2D, simulation->views[i]->getSceneTexture());
            glColor4f(1.f, 1.f, 1.f, 1.f);
            DrawScreenAlignedQuad(); //introduce distortion?
            
            /////////OVERLAY DUMMIES////////
            glBindTexture(GL_TEXTURE_2D, 0);
            simulation->views[i]->SetProjection();
            simulation->views[i]->SetViewTransform();
            
            //Coordinate systems
            DrawCoordSystem(2.f);
            //OpenGLSun::ShowFrustumSplits();
            
            if(showCoordSys)
            {
                for(int h=0; h<simulation->entities.size(); h++)
                    if(simulation->entities[h]->getType() == SOLID)
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
                        DrawCoordSystem(0.1f);
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
            
            //Stickers
            if(showStickers)
            {
                glm::vec4 viewp(viewport[0], viewport[1], viewport[2], viewport[3]);
                glm::mat4 proj = simulation->views[i]->GetProjectionMatrix();
                glm::mat4 model;
                glGetFloatv(GL_MODELVIEW, glm::value_ptr(model));
                
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                glOrtho(0,viewport[2],0,viewport[3],-1,1);
                
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
            //simulation->lights[0]->RenderShadowMap(this);
            //simulation->lights[0]->ShowShadowMap(0, 0, 0.5f);
            //OpenGLSun::ShowShadowMaps(0, 0, 0.2);
            //OpenGLSky::ShowCubemap(CONVOLUTION_DIFFUSE, 0, 0, 400, 400);
            //OpenGLSky::ShowCubemap(CONVOLUTION_REFLECT, 400, 0, 400, 400);
            //views[i]->getGBuffer()->ShowTexture(POSITION1, 0,0,250,200); // FBO debugging
            //views[i]->getGBuffer()->ShowTexture(POSITION2, 0,200,250,200); // FBO debugging
            
            delete viewport;
        }
    }
}

void OpenGLPipeline::RenderView(OpenGLView *view, const btTransform& viewTransform)
{
    //1. Enter deferred rendering
    SetupOrtho();
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    //2. Draw sky
    if(renderSky)
        OpenGLSky::Render(view, viewTransform, simulation->zUp);
    
    //3. Bind deferred textures to texture units
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, view->getGBuffer()->getDiffuseTexture());
    
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, view->getGBuffer()->getPositionTexture(0));
    
    glActiveTextureARB(GL_TEXTURE2_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, view->getGBuffer()->getNormalsTexture(0));
    
    glActiveTextureARB(GL_TEXTURE3_ARB);
    glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, OpenGLSky::getDiffuseCubemap());
    
    glActiveTextureARB(GL_TEXTURE4_ARB);
    glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, OpenGLSky::getReflectionCubemap());
    
    //4. Bind SSAO texture if needed
    glActiveTextureARB(GL_TEXTURE5_ARB);
    glEnable(GL_TEXTURE_2D);
    if(renderSSAO)
        glBindTexture(GL_TEXTURE_2D, view->getSSAOTexture());
    else
        glBindTexture(GL_TEXTURE_2D, 0);
    
    OpenGLSun::SetTextureUnits(0, 2, 1, 6);
    OpenGLLight::SetTextureUnits(0, 2, 1, 3, 4, 5, 6);
    
    //5. Render ambient pass - sky, ssao
    OpenGLLight::RenderAmbientLight(viewTransform, simulation->zUp);
    
    //6. Render lights
    glBlendFunc(GL_ONE, GL_ONE); //accumulate light
    
    OpenGLSun::Render(); //Render sun pass
    
    for(int h=0; h<simulation->lights.size(); h++) //Render light passes
        simulation->lights[h]->Render();
    
    //7. Reset OpenGL
    glUseProgramObjectARB(0);
    
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
}