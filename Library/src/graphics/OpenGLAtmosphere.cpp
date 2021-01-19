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
//  OpenGLAtmosphere.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 22/07/2017.
//  Copyright (c) 2017-2020 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLAtmosphere.h"

#include <fstream>
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLCamera.h"
#include "graphics/OpenGLContent.h"
#include "utils/SystemUtil.hpp"
#ifdef EMBEDDED_RESOURCES
#include <sstream>
#include "ResourceHandle.h"
#endif

namespace sf
{

GLuint OpenGLAtmosphere::atmosphereAPI = 0;

OpenGLAtmosphere::OpenGLAtmosphere(const std::string& modelFilename, RenderQuality shadow)
{
    //Prepare for rendering
    for(unsigned short i=0; i<AtmosphereTextures::TEXTURE_COUNT; ++i) textures[i] = 0;
    skySunShader = NULL;
    
    //Init shadow baking
    sunShadowmapShader = NULL;
    sunShadowmapArray = 0;
    sunDepthSampler = 0;
    sunShadowSampler = 0;
    sunShadowmapSplits = 4;
    sunShadowmapSize = 4096;
    sunShadowFBO = 0;
    sunSkyUBO = 0;
    sunDirection = glm::vec3(0,0,1.f);
    sunModelView = glm::mat4x4(0);
    sunShadowFrustum = NULL;
    sunShadowCPM = NULL;
    
    //Set standard sun position
    SetSunPosition(0.0, 45.0);

    //Set shadow quality
    switch(shadow)
    {
        case RenderQuality::DISABLED:
            sunShadowmapSize = 0;
            sunShadowmapSplits = 0;
            break;
        
        case RenderQuality::LOW:
            sunShadowmapSize = 1024;
            sunShadowmapSplits = 4;
            break;
        
        case RenderQuality::MEDIUM:
            sunShadowmapSize = 2048;
            sunShadowmapSplits = 4;
            break;
        
        case RenderQuality::HIGH:
            sunShadowmapSize = 4096;
            sunShadowmapSplits = 4;
            break;
    }

    //Load API and Data
    memset(&sunSkyUBOData, 0, sizeof(SunSkyUBO));
    LoadAtmosphereData(modelFilename);
    
    //Permanently bind atmosphere textures
    OpenGLState::BindTexture(TEX_ATM_TRANSMITTANCE, GL_TEXTURE_2D, textures[AtmosphereTextures::TRANSMITTANCE]);
    OpenGLState::BindTexture(TEX_ATM_SCATTERING, GL_TEXTURE_3D, textures[AtmosphereTextures::SCATTERING]);
    OpenGLState::BindTexture(TEX_ATM_IRRADIANCE, GL_TEXTURE_2D, textures[AtmosphereTextures::IRRADIANCE]);
    
    //Build rendering shaders
    std::vector<GLuint> compiledShaders;
    compiledShaders.push_back(atmosphereAPI);
    std::vector<GLSLSource> sources;
    sources.push_back(GLSLSource(GL_VERTEX_SHADER, "atmosphereSkySun.vert"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "atmosphereSkySun.frag"));
    skySunShader = new GLSLShader(sources, compiledShaders);
    skySunShader->AddUniform("transmittance_texture", ParameterType::INT);
    skySunShader->AddUniform("scattering_texture", ParameterType::INT);
    skySunShader->AddUniform("single_mie_scattering_texture", ParameterType::INT);
    skySunShader->AddUniform("eyePos", ParameterType::VEC3);
    skySunShader->AddUniform("sunDir", ParameterType::VEC3);
    skySunShader->AddUniform("invView", ParameterType::MAT4);
    skySunShader->AddUniform("invProj", ParameterType::MAT4);
    skySunShader->AddUniform("whitePoint", ParameterType::VEC3);
    skySunShader->AddUniform("cosSunSize", ParameterType::FLOAT);
	skySunShader->AddUniform("bottomRadius", ParameterType::FLOAT);
	skySunShader->AddUniform("groundAlbedo", ParameterType::FLOAT);

    //Initialize shadows
    sunShadowFrustum = new ViewFrustum[sunShadowmapSplits];
    sunShadowCPM = new glm::mat4x4[sunShadowmapSplits];

    //Create sun & sky UBO
    glGenBuffers(1, &sunSkyUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, sunSkyUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(SunSkyUBO), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, UBO_SUNSKY, sunSkyUBO, 0, sizeof(SunSkyUBO));

    if(shadow > RenderQuality::DISABLED)
    {
        sunShadowmapShader = new GLSLShader("sunCSM.frag");
        sunShadowmapShader->AddUniform("shadowmapArray", ParameterType::INT);
        sunShadowmapShader->AddUniform("shadowmapLayer", ParameterType::FLOAT);
        
        //Generate shadowmap array
        glGenTextures(1, &sunShadowmapArray);
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D_ARRAY, sunShadowmapArray);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, sunShadowmapSize, sunShadowmapSize, sunShadowmapSplits, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        OpenGLState::UnbindTexture(TEX_BASE);
        
        //Generate samplers
        glGenSamplers(1, &sunDepthSampler);
        glSamplerParameteri(sunDepthSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glSamplerParameteri(sunDepthSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameteri(sunDepthSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(sunDepthSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glGenSamplers(1, &sunShadowSampler);
        glSamplerParameteri(sunShadowSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glSamplerParameteri(sunShadowSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameteri(sunShadowSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(sunShadowSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(sunShadowSampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glSamplerParameteri(sunShadowSampler, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        
        //Create shadowmap framebuffer
        glGenFramebuffers(1, &sunShadowFBO);
        OpenGLState::BindFramebuffer(sunShadowFBO);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, sunShadowmapArray, 0, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status != GL_FRAMEBUFFER_COMPLETE)
            cError("Sun shadow FBO initialization failed!");
        
        OpenGLState::BindFramebuffer(0);

        //Permanently bind shadow textures and samplers
        OpenGLState::BindTexture(TEX_SUN_SHADOW, GL_TEXTURE_2D_ARRAY, sunShadowmapArray);
        glBindSampler(TEX_SUN_SHADOW, sunShadowSampler);

        OpenGLState::BindTexture(TEX_SUN_DEPTH, GL_TEXTURE_2D_ARRAY, sunShadowmapArray);
        glBindSampler(TEX_SUN_DEPTH, sunDepthSampler);
    }
}

OpenGLAtmosphere::~OpenGLAtmosphere()
{
    for(unsigned short i=0; i< AtmosphereTextures::TEXTURE_COUNT; ++i)
        if(textures[i] != 0) glDeleteTextures(1, &textures[i]);

    if(skySunShader != NULL) delete skySunShader;
    //if(atmosphereAPI > 0) glDeleteShader(atmosphereAPI);

    if(sunShadowmapArray != 0) glDeleteTextures(1, &sunShadowmapArray);

    delete [] sunShadowFrustum;
    delete [] sunShadowCPM;
    glDeleteBuffers(1, &sunSkyUBO);

    if(sunShadowmapSize > 0)
    {
        glDeleteSamplers(1, &sunDepthSampler);
        glDeleteSamplers(1, &sunShadowSampler);
        glDeleteFramebuffers(1, &sunShadowFBO);
        delete sunShadowmapShader;
    }
}

void OpenGLAtmosphere::SetSunPosition(float azimuthDeg, float elevationDeg)
{
    azimuthDeg = azimuthDeg < -180.f ? -180.f : (azimuthDeg > 180.f ? 180.f : azimuthDeg);
    elevationDeg = elevationDeg < -90.f ? -90.f : (elevationDeg > 90.f ? 90.f : elevationDeg);

    sunAzimuth = azimuthDeg;
    sunElevation = elevationDeg;

    //Calculate sun dir vector
    float sunAzimuthAngle = -sunAzimuth/180.f*M_PI + M_PI;
    float sunZenithAngle =  (90.f - sunElevation)/180.f*M_PI;
    sunDirection = glm::normalize(glm::rotate(glm::vec3(cos(sunAzimuthAngle) * sin(sunZenithAngle), sin(sunAzimuthAngle) * sin(sunZenithAngle), cos(sunZenithAngle)), (float)M_PI, glm::vec3(0,1.f,0)));
    
    //Build sun modelview matrix
    glm::vec3 up(0,0,-1.f);
    glm::vec3 right = glm::cross(sunDirection, up);
    right = glm::normalize(right);
    up = glm::normalize(glm::cross(right, sunDirection));
    sunModelView = glm::lookAt(glm::vec3(0,0,0), -sunDirection, up) * glm::mat4(1.f);
}

void OpenGLAtmosphere::GetSunPosition(GLfloat& azimuthDeg, GLfloat& elevationDeg)
{
    azimuthDeg = sunAzimuth;
    elevationDeg = sunElevation;
}

glm::vec3 OpenGLAtmosphere::GetSunDirection()
{
    return sunDirection;
}

GLuint OpenGLAtmosphere::getAtmosphereTexture(AtmosphereTextures id)
{
    return textures[id];
}

void OpenGLAtmosphere::LoadAtmosphereData(const std::string& filename)
{
    //Load parameters used to generate atmosphere data
    glm::uvec3 transmittanceSize;
    glm::uvec3 irradianceSize;
    glm::uvec3 scatteringSize;
    GLfloat lengthUnitInMeters;
    GLfloat bottomRadius;
#ifdef EMBEDDED_RESOURCES
    ResourceHandle rh(filename);
    std::istringstream dataString(rh.string());
    std::istream& data(dataString);
#else
    std::ifstream dataFile(filename, std::ios::binary | std::ios::in);
    std::istream& data(dataFile);
#endif    
    data.read((char*)&transmittanceSize, sizeof(transmittanceSize));
    data.read((char*)&irradianceSize, sizeof(irradianceSize));
    data.read((char*)&scatteringSize, sizeof(scatteringSize));
    data.read((char*)&lengthUnitInMeters, sizeof(lengthUnitInMeters));
    data.read((char*)&bottomRadius, sizeof(bottomRadius));
    data.read((char*)&sunSkyUBOData.whitePoint, sizeof(sunSkyUBOData.whitePoint));
    data.read((char*)&nPrecomputedWavelengths, sizeof(nPrecomputedWavelengths));
    data.read((char*)&nScatteringOrders, sizeof(nScatteringOrders));

    sunSkyUBOData.atmLengthUnitInMeters = lengthUnitInMeters;
    sunSkyUBOData.planetRadiusInUnits = bottomRadius/lengthUnitInMeters;

    //Load atmosphere textures
    glm::vec4* pixels;
    pixels = new glm::vec4[transmittanceSize.x * transmittanceSize.y];
    data.read((char*)pixels, sizeof(glm::vec4) * transmittanceSize.x * transmittanceSize.y);
    textures[AtmosphereTextures::TRANSMITTANCE] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, transmittanceSize, GL_RGBA32F, GL_RGBA, GL_FLOAT, pixels, FilteringMode::BILINEAR, false);
    delete [] pixels;
    pixels = new glm::vec4[irradianceSize.x * irradianceSize.y];
    data.read((char*)pixels, sizeof(glm::vec4) * irradianceSize.x * irradianceSize.y);
    textures[AtmosphereTextures::IRRADIANCE] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, irradianceSize, GL_RGBA32F, GL_RGBA, GL_FLOAT, pixels, FilteringMode::BILINEAR, false);
    delete [] pixels;
    pixels = new glm::vec4[scatteringSize.x * scatteringSize.y * scatteringSize.z];
    data.read((char*)pixels, sizeof(glm::vec4) * scatteringSize.x * scatteringSize.y * scatteringSize.z);
    textures[AtmosphereTextures::SCATTERING] = OpenGLContent::GenerateTexture(GL_TEXTURE_3D, scatteringSize, GL_RGBA16F, GL_RGBA, GL_FLOAT, pixels, FilteringMode::BILINEAR, false);
    delete [] pixels;
#ifndef EMBEDDED_RESOURCES
    dataFile.close();
#endif
    cInfo("Loaded precomputed atmosphere model (%u wavelengths, %u scattering orders)", 
           nPrecomputedWavelengths, nScatteringOrders);
}

void OpenGLAtmosphere::DrawSkyAndSun(const OpenGLCamera* view)
{
	glm::vec3 eyePos = view->GetEyePosition()/sunSkyUBOData.atmLengthUnitInMeters;
	eyePos.z = eyePos.z > -0.5/sunSkyUBOData.atmLengthUnitInMeters ? -0.5/sunSkyUBOData.atmLengthUnitInMeters : eyePos.z;
    glm::mat4 invViewMatrix = glm::inverse(view->GetViewMatrix());
	invViewMatrix[3].x = eyePos.x;
	invViewMatrix[3].y = eyePos.y;
	invViewMatrix[3].z = eyePos.z;
    glm::mat4 invProjectionMatrix = glm::inverse(view->GetProjectionMatrix());

    skySunShader->Use();
	skySunShader->SetUniform("bottomRadius", sunSkyUBOData.planetRadiusInUnits);
	skySunShader->SetUniform("groundAlbedo", 0.7f);
    skySunShader->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    skySunShader->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    skySunShader->SetUniform("single_mie_scattering_texture", TEX_ATM_SCATTERING);
    skySunShader->SetUniform("eyePos", eyePos);
    skySunShader->SetUniform("sunDir", GetSunDirection());
    skySunShader->SetUniform("invProj", invProjectionMatrix);
    skySunShader->SetUniform("invView", invViewMatrix);
    skySunShader->SetUniform("whitePoint", sunSkyUBOData.whitePoint);
    skySunShader->SetUniform("cosSunSize", (GLfloat)cosf(0.00935f/2.f));
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    OpenGLState::UseProgram(0);
}

void OpenGLAtmosphere::BakeShadowmaps(OpenGLPipeline* pipe, OpenGLCamera* view)
{
    if(sunShadowmapSize == 0)
        return;
    
    //Pre-set splits
    for(unsigned int i = 0; i < sunShadowmapSplits; ++i)
    {
        sunShadowFrustum[i].fov = view->GetFOVY() + 0.2f; //avoid artifacts in the borders
        GLint* viewport = view->GetViewport();
        sunShadowFrustum[i].ratio = (GLfloat)viewport[2]/(GLfloat)viewport[3];
        delete [] viewport;
    }

    //Compute the z-distances for each split as seen in camera space
    UpdateSplitDist(view->GetNearClip(), view->GetFarClip());

    //Render maps
    glCullFace(GL_FRONT); //GL_FRONT -> no shadow acne but problems with filtering
    glDisable(GL_DEPTH_CLAMP);

    OpenGLState::BindFramebuffer(sunShadowFBO);
    OpenGLState::Viewport(0, 0, sunShadowmapSize, sunShadowmapSize);

    glm::vec3 camPos = view->GetEyePosition();
    glm::vec3 camDir = view->GetLookingDirection();
    glm::vec3 camUp = view->GetUpDirection();

    // for all shadow splits
    for(unsigned int i = 0; i < sunShadowmapSplits; ++i)
    {
        //Compute the camera frustum slice boundary points in world space
        UpdateFrustumCorners(sunShadowFrustum[i], camPos, camDir, camUp);

        //Adjust the view frustum of the light, so that it encloses the camera frustum slice fully.
        //note that this function sets the projection matrix as it sees best fit
        glm::mat4 cp = BuildCropProjMatrix(sunShadowFrustum[i]);
        sunShadowCPM[i] =  cp * sunModelView;

        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->SetProjectionMatrix(cp);
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->SetViewMatrix(sunModelView);
        //Draw current depth map
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, sunShadowmapArray, 0, i);
        glClear(GL_DEPTH_BUFFER_BIT);
        pipe->DrawObjects();
    }

    OpenGLState::BindFramebuffer(0);

    glEnable(GL_DEPTH_CLAMP);
    glCullFace(GL_BACK);
}

//Computes the near and far distances for every frustum slice in camera eye space
void OpenGLAtmosphere::UpdateSplitDist(GLfloat nd, GLfloat fd)
{
    fd = fd > 250.f ? 250.f : fd; //Limit camera frustum to max 250.0 m (to avoid shadow blocks)
    nd = 0.1f;
    GLfloat lambda = 0.95f;
    GLfloat ratio = fd/nd;
    sunShadowFrustum[0].near = nd;

    for(unsigned int i = 1; i < sunShadowmapSplits; ++i) 
    {
        GLfloat si = i / (GLfloat)sunShadowmapSplits;

        sunShadowFrustum[i].near = lambda*(nd*powf(ratio, si)) + (1.f-lambda)*(nd + (fd - nd)*si);
        sunShadowFrustum[i-1].far = sunShadowFrustum[i].near * 1.05f;
    }

    sunShadowFrustum[sunShadowmapSplits-1].far = fd;
    //for(int i=0; i<sunShadowmapSplits; i++) printf("Frustum%d Near: %f Far: %f\n", i, sunShadowFrustum[i].near, sunShadowFrustum[i].far);
}

//Computes the 8 corner points of the current view frustum
void OpenGLAtmosphere::UpdateFrustumCorners(ViewFrustum &f, glm::vec3 center, glm::vec3 dir, glm::vec3 up)
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
glm::mat4 OpenGLAtmosphere::BuildCropProjMatrix(ViewFrustum &f)
{
    GLfloat maxX = -1000.0f;
    GLfloat maxY = -1000.0f;
    GLfloat maxZ;
    GLfloat minX =  1000.0f;
    GLfloat minY =  1000.0f;
    GLfloat minZ;
    glm::vec4 transf;

    //Find the z-range of the current frustum as seen from the light in order to increase precision
    glm::mat4 shad_mv = sunModelView;

    //Note: only the z-component is needed and thus the multiplication can be simplified
    //transf.z = shad_modelview[2] * f.point[0].x + shad_modelview[6] * f.point[0].y + shad_modelview[10] * f.point[0].z + shad_modelview[14]
    transf = shad_mv * glm::vec4(f.corners[0], 1.0f);
    minZ = transf.z;
    maxZ = transf.z;

    for(int i = 1; i <8 ; i++) {
        transf = shad_mv * glm::vec4(f.corners[i], 1.0f);
        if(transf.z > maxZ) maxZ = transf.z;
        if(transf.z < minZ) minZ = transf.z;
    }

    //Make sure all relevant shadow casters are included - use object bounding boxes!
    Vector3 aabbMin;
    Vector3 aabbMax;
    SimulationApp::getApp()->getSimulationManager()->getWorldAABB(aabbMin, aabbMax);

    transf = shad_mv * glm::vec4(aabbMin.x(), aabbMin.y(), aabbMin.z(), 1.f);
    if(transf.z > maxZ) maxZ = transf.z;
    if(transf.z < minZ) minZ = transf.z;

    transf = shad_mv * glm::vec4(aabbMax.x(), aabbMax.y(), aabbMax.z(), 1.f);
    if(transf.z > maxZ) maxZ = transf.z;
    if(transf.z < minZ) minZ = transf.z;

    //Set the projection matrix with the new z-bounds
    //Note: there is inversion because the light looks at the neg z axis
    glm::mat4 shad_proj = glm::ortho(-1.f, 1.f, -1.f, 1.f, -maxZ, -minZ);
    glm::mat4 shad_mvp =  shad_proj * shad_mv;

    //Find the extends of the frustum slice as projected in light's homogeneous coordinates
    for(int i = 0; i < 8; i++) {
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

void OpenGLAtmosphere::SetupMaterialShaders()
{
    //Calculate shadow splits
    glm::mat4 bias(0.5f, 0.f, 0.f, 0.f,
                   0.f, 0.5f, 0.f, 0.f,
                   0.f, 0.f, 0.5f, 0.f,
                   0.5f, 0.5f, 0.5f, 1.f);
    GLfloat frustumFar[4];
    GLfloat frustumNear[4];

    //For every inactive split
    for(unsigned int i = sunShadowmapSplits; i<4; ++i)
    {
        frustumNear[i] = 0;
        frustumFar[i] = 0;
        sunSkyUBOData.sunClipSpace[i] = glm::mat4();
    }

    //For every active split
    for(unsigned int i = 0; i < sunShadowmapSplits; ++i)
    {
        frustumNear[i] = sunShadowFrustum[i].near;
        frustumFar[i] = sunShadowFrustum[i].far;
        sunSkyUBOData.sunClipSpace[i] = (bias * sunShadowCPM[i]); // compute a matrix that transforms from world space to light clip space
    }

    sunSkyUBOData.sunFrustumNear = glm::vec4(frustumNear[0], frustumNear[1], frustumNear[2], frustumNear[3]);
    sunSkyUBOData.sunFrustumFar = glm::vec4(frustumFar[0], frustumFar[1], frustumFar[2], frustumFar[3]);
    sunSkyUBOData.sunDirection = GetSunDirection();

    glBindBuffer(GL_UNIFORM_BUFFER, sunSkyUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SunSkyUBO), &sunSkyUBOData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLAtmosphere::ShowSunShadowmaps(GLfloat x, GLfloat y, GLfloat scale)
{
    //Texture setup
    OpenGLState::BindTexture(TEX_SUN_SHADOW, GL_TEXTURE_2D_ARRAY, sunShadowmapArray);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    OpenGLState::DisableBlend();
    OpenGLState::DisableDepthTest();

    //Render the shadowmaps
    sunShadowmapShader->Use();
    sunShadowmapShader->SetUniform("shadowmapArray", TEX_SUN_SHADOW);
    for(unsigned int i = 0; i < sunShadowmapSplits; ++i) {
        OpenGLState::Viewport(x + sunShadowmapSize * scale * i, y, sunShadowmapSize * scale, sunShadowmapSize * scale);
        sunShadowmapShader->SetUniform("shadowmapLayer", (GLfloat)i);
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    }
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_SUN_SHADOW);
}
    
void OpenGLAtmosphere::Init()
{
    GLint compiled;
    atmosphereAPI = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "atmosphereApi.glsl", "", &compiled);
}

GLuint OpenGLAtmosphere::getAtmosphereAPI()
{
    return atmosphereAPI;
}

}
