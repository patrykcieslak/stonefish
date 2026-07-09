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
//  OpenGLFlatOcean.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/05/2020.
//  Copyright (c) 2020-2026 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLFlatOcean.h"

#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLView.h"
#include "graphics/OpenGLAtmosphere.h"

namespace sf
{

OpenGLFlatOcean::OpenGLFlatOcean(GLfloat size) : OpenGLOcean(200000.f)
{
    params_.wind = 5.f;
    params_.A = 1.0f;
    params_.omega = 2.f;

    GLint compiled;
    GLuint pcssFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "lighting.frag", "", &compiled);
	std::vector<GLuint> precompiled;
    precompiled.push_back(OpenGLAtmosphere::getAtmosphereAPI());
    precompiled.push_back(pcssFragment);

    //Surface rendering
    std::vector<GLSLSource> sources;
    sources.push_back(GLSLSource(GL_VERTEX_SHADER, "oceanSurface.vert"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanSurface.frag"));
    oceanShaders_["surface"] = std::make_unique<GLSLShader>(sources, precompiled);
    oceanShaders_["surface"]->AddUniform("size", ParameterType::FLOAT);
    oceanShaders_["surface"]->AddUniform("texWaveFFT", ParameterType::INT);
    oceanShaders_["surface"]->AddUniform("texSlopeVariance", ParameterType::INT);
    oceanShaders_["surface"]->AddUniform("MVP", ParameterType::MAT4);
    oceanShaders_["surface"]->AddUniform("gridSizes", ParameterType::VEC4);
    oceanShaders_["surface"]->AddUniform("eyePos", ParameterType::VEC3);
    oceanShaders_["surface"]->AddUniform("viewDir", ParameterType::VEC3);
    oceanShaders_["surface"]->AddUniform("MV", ParameterType::MAT3);
    oceanShaders_["surface"]->AddUniform("FC", ParameterType::FLOAT);
    oceanShaders_["surface"]->AddUniform("viewport", ParameterType::VEC2);
    oceanShaders_["surface"]->AddUniform("transmittance_texture", ParameterType::INT);
    oceanShaders_["surface"]->AddUniform("scattering_texture", ParameterType::INT);
    oceanShaders_["surface"]->AddUniform("irradiance_texture", ParameterType::INT);
    oceanShaders_["surface"]->AddUniform("sunShadowMap", ParameterType::INT);
    oceanShaders_["surface"]->AddUniform("sunDepthMap", ParameterType::INT);
    oceanShaders_["surface"]->BindUniformBlock("SunSky", UBO_SUNSKY);

    oceanShaders_["surface"]->Use();
    oceanShaders_["surface"]->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    oceanShaders_["surface"]->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    oceanShaders_["surface"]->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);
    oceanShaders_["surface"]->SetUniform("sunDepthMap", TEX_SUN_DEPTH);
    oceanShaders_["surface"]->SetUniform("sunShadowMap", TEX_SUN_SHADOW);    
    OpenGLState::UseProgram(0);

    //Surface temeperature rendering
    sources.pop_back();
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanSurfaceTemp.frag"));
    oceanShaders_["surfaceTemp"] = std::make_unique<GLSLShader>(sources, precompiled);
    oceanShaders_["surfaceTemp"]->AddUniform("size", ParameterType::FLOAT);
    oceanShaders_["surfaceTemp"]->AddUniform("texWaveFFT", ParameterType::INT);
    oceanShaders_["surfaceTemp"]->AddUniform("texSlopeVariance", ParameterType::INT);
    oceanShaders_["surfaceTemp"]->AddUniform("MVP", ParameterType::MAT4);
    oceanShaders_["surfaceTemp"]->AddUniform("gridSizes", ParameterType::VEC4);
    oceanShaders_["surfaceTemp"]->AddUniform("eyePos", ParameterType::VEC3);
    oceanShaders_["surfaceTemp"]->AddUniform("viewDir", ParameterType::VEC3);
    oceanShaders_["surfaceTemp"]->AddUniform("MV", ParameterType::MAT3);
    oceanShaders_["surfaceTemp"]->AddUniform("FC", ParameterType::FLOAT);
    oceanShaders_["surfaceTemp"]->AddUniform("viewport", ParameterType::VEC2);
    oceanShaders_["surfaceTemp"]->AddUniform("transmittance_texture", ParameterType::INT);
    oceanShaders_["surfaceTemp"]->AddUniform("scattering_texture", ParameterType::INT);
    oceanShaders_["surfaceTemp"]->AddUniform("irradiance_texture", ParameterType::INT);
    oceanShaders_["surfaceTemp"]->AddUniform("sunShadowMap", ParameterType::INT);
    oceanShaders_["surfaceTemp"]->AddUniform("sunDepthMap", ParameterType::INT);
    oceanShaders_["surfaceTemp"]->AddUniform("waterTemperature", ParameterType::FLOAT);
    oceanShaders_["surfaceTemp"]->BindUniformBlock("SunSky", UBO_SUNSKY);

    oceanShaders_["surfaceTemp"]->Use();
    oceanShaders_["surfaceTemp"]->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    oceanShaders_["surfaceTemp"]->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    oceanShaders_["surfaceTemp"]->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);
    oceanShaders_["surfaceTemp"]->SetUniform("sunDepthMap", TEX_SUN_DEPTH);
    oceanShaders_["surfaceTemp"]->SetUniform("sunShadowMap", TEX_SUN_SHADOW);
    OpenGLState::UseProgram(0);
    
    //Backsurface rendering
	GLuint oceanOpticsFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "oceanOptics.frag", "", &compiled);
    precompiled.pop_back();
    precompiled.push_back(oceanOpticsFragment);

    sources.pop_back();
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanBacksurface.frag"));
    oceanShaders_["backsurface"] = std::make_unique<GLSLShader>(sources, precompiled);
    oceanShaders_["backsurface"]->AddUniform("size", ParameterType::FLOAT);
    oceanShaders_["backsurface"]->AddUniform("texWaveFFT", ParameterType::INT);
    oceanShaders_["backsurface"]->AddUniform("texSlopeVariance", ParameterType::INT);
    oceanShaders_["backsurface"]->AddUniform("MVP", ParameterType::MAT4);
    oceanShaders_["backsurface"]->AddUniform("gridSizes", ParameterType::VEC4);
    oceanShaders_["backsurface"]->AddUniform("eyePos", ParameterType::VEC3);
    oceanShaders_["backsurface"]->AddUniform("MV", ParameterType::MAT3);
    oceanShaders_["backsurface"]->AddUniform("FC", ParameterType::FLOAT);
    oceanShaders_["backsurface"]->AddUniform("viewport", ParameterType::VEC2);
    oceanShaders_["backsurface"]->AddUniform("cWater", ParameterType::VEC3);
    oceanShaders_["backsurface"]->AddUniform("bWater", ParameterType::VEC3);
    oceanShaders_["backsurface"]->AddUniform("transmittance_texture", ParameterType::INT);
    oceanShaders_["backsurface"]->AddUniform("scattering_texture", ParameterType::INT);
    oceanShaders_["backsurface"]->AddUniform("irradiance_texture", ParameterType::INT);
    oceanShaders_["backsurface"]->BindUniformBlock("SunSky", UBO_SUNSKY);
    oceanShaders_["backsurface"]->BindUniformBlock("Lights", UBO_LIGHTS);
    glDeleteShader(oceanOpticsFragment);

    oceanShaders_["backsurface"]->Use();
    oceanShaders_["backsurface"]->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    oceanShaders_["backsurface"]->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    oceanShaders_["backsurface"]->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);
    OpenGLState::UseProgram(0);

    //Mask rendering
    sources.pop_back();
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "flat.frag"));
    oceanShaders_["mask"] = std::make_unique<GLSLShader>(sources);
    oceanShaders_["mask"]->AddUniform("size", ParameterType::FLOAT);
    oceanShaders_["mask"]->AddUniform("MVP", ParameterType::MAT4);
    oceanShaders_["mask"]->AddUniform("FC", ParameterType::FLOAT);    
    
    //Surface (infinite plane)
    GLfloat surfData[12][3] = 
    {
        {0.f, 0.f, 0.f},
        {0.f, 0.5f, 0.f},
        {0.5f, 0.f, 0.f},
        {0.f, 0.f, 0.f},
        {-0.5f, 0.f, 0.f},
        {0.f, 0.5f, 0.f},
        {0.f, 0.f, 0.f},
        {0.f, -0.5f, 0.f},
        {-0.5f, 0.f, 0.f},
        {0.f, 0.f, 0.f},
        {0.5f, 0.f, 0.f},
        {0.f, -0.5f, 0.f}
    };
    
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    OpenGLState::BindVertexArray(vao_);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(surfData), surfData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    OpenGLState::BindVertexArray(0);

    InitializeSimulation();
}

OpenGLFlatOcean::~OpenGLFlatOcean()
{
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
}
    
void OpenGLFlatOcean::UpdateSurface(OpenGLView* view)
{
}
        
void OpenGLFlatOcean::DrawSurface(OpenGLView* view)
{
    std::vector<GLint> viewport = view->GetViewport();
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures_[3]);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_3D, oceanTextures_[2]);

    oceanShaders_["surface"]->Use();
    oceanShaders_["surface"]->SetUniform("MVP", view->GetProjectionMatrix() * view->GetViewMatrix());
    oceanShaders_["surface"]->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view->GetViewMatrix()))));
    oceanShaders_["surface"]->SetUniform("FC", view->GetLogDepthConstant());
    oceanShaders_["surface"]->SetUniform("size", oceanSize_);
    oceanShaders_["surface"]->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
    oceanShaders_["surface"]->SetUniform("eyePos", view->GetEyePosition());
    oceanShaders_["surface"]->SetUniform("viewDir", view->GetLookingDirection());
    oceanShaders_["surface"]->SetUniform("gridSizes", params_.gridSizes);
    oceanShaders_["surface"]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
    oceanShaders_["surface"]->SetUniform("texSlopeVariance", TEX_POSTPROCESS2);
    OpenGLState::BindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    OpenGLState::BindVertexArray(0);

    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
}

void OpenGLFlatOcean::DrawSurfaceTemperature(OpenGLView* view)
{
    std::vector<GLint> viewport = view->GetViewport();
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures_[3]);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_3D, oceanTextures_[2]);

    oceanShaders_["surfaceTemp"]->Use();
    oceanShaders_["surfaceTemp"]->SetUniform("MVP", view->GetProjectionMatrix() * view->GetViewMatrix());
    oceanShaders_["surfaceTemp"]->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view->GetViewMatrix()))));
    oceanShaders_["surfaceTemp"]->SetUniform("FC", view->GetLogDepthConstant());
    oceanShaders_["surfaceTemp"]->SetUniform("size", oceanSize_);
    oceanShaders_["surfaceTemp"]->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
    oceanShaders_["surfaceTemp"]->SetUniform("eyePos", view->GetEyePosition());
    oceanShaders_["surfaceTemp"]->SetUniform("viewDir", view->GetLookingDirection());
    oceanShaders_["surfaceTemp"]->SetUniform("gridSizes", params_.gridSizes);
    oceanShaders_["surfaceTemp"]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
    oceanShaders_["surfaceTemp"]->SetUniform("texSlopeVariance", TEX_POSTPROCESS2);
    oceanShaders_["surfaceTemp"]->SetUniform("waterTemperature", waterTemperature_);

    OpenGLState::BindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    OpenGLState::BindVertexArray(0);

    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
}
        
void OpenGLFlatOcean::DrawBacksurface(OpenGLView* view)
{
    std::vector<GLint> viewport = view->GetViewport();
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures_[3]);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_3D, oceanTextures_[2]);

    oceanShaders_["backsurface"]->Use();
    oceanShaders_["backsurface"]->SetUniform("MVP", view->GetProjectionMatrix() * view->GetViewMatrix());
    oceanShaders_["backsurface"]->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view->GetViewMatrix()))));
    oceanShaders_["backsurface"]->SetUniform("FC", view->GetLogDepthConstant());
    oceanShaders_["backsurface"]->SetUniform("size", oceanSize_);
    oceanShaders_["backsurface"]->SetUniform("eyePos", view->GetEyePosition());
    oceanShaders_["backsurface"]->SetUniform("gridSizes", params_.gridSizes);
    oceanShaders_["backsurface"]->SetUniform("cWater", getLightAttenuation());
    oceanShaders_["backsurface"]->SetUniform("bWater", getLightScattering());
    oceanShaders_["backsurface"]->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
    oceanShaders_["backsurface"]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
    oceanShaders_["backsurface"]->SetUniform("texSlopeVariance", TEX_POSTPROCESS2);
    OpenGLState::BindVertexArray(vao_);
    glCullFace(GL_FRONT);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    glCullFace(GL_BACK);
    OpenGLState::BindVertexArray(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    OpenGLState::UseProgram(0);
}
        
void OpenGLFlatOcean::DrawUnderwaterMask(OpenGLView* view)
{
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    oceanShaders_["mask"]->Use();
    oceanShaders_["mask"]->SetUniform("MVP", view->GetProjectionMatrix() * view->GetViewMatrix());
    oceanShaders_["mask"]->SetUniform("FC", view->GetLogDepthConstant());
    oceanShaders_["mask"]->SetUniform("size", oceanSize_);
    
    //1. Draw surface to depth buffer
    OpenGLState::BindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    OpenGLState::BindVertexArray(0);
    
    //2. Draw backsurface to depth and stencil buffer
    OpenGLState::EnableStencilTest();
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);
    glCullFace(GL_FRONT);
    
    OpenGLState::BindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    OpenGLState::BindVertexArray(0);

    //3. Draw box around whole ocean
    OpenGLOcean::DrawUnderwaterMask(view);

    glCullFace(GL_BACK);
    OpenGLState::DisableStencilTest();
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);
}

}
