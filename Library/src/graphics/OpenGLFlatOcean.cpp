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
//  Copyright (c) 2020-2024 Patryk Cieslak. All rights reserved.
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
    params.wind = 5.f;
    params.A = 1.0f;
    params.omega = 2.f;

    GLint compiled;
    GLuint pcssFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "lighting.frag", "", &compiled);
	std::vector<GLuint> precompiled;
    precompiled.push_back(OpenGLAtmosphere::getAtmosphereAPI());
    precompiled.push_back(GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "cookTorrance.frag", "", &compiled));
    precompiled.push_back(pcssFragment);

    //Surface rendering
    std::vector<GLSLSource> sources;
    sources.push_back(GLSLSource(GL_VERTEX_SHADER, "oceanSurface.vert"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanSurface.frag"));
    oceanShaders["surface"] = new GLSLShader(sources, precompiled);
    oceanShaders["surface"]->AddUniform("size", ParameterType::FLOAT);
    oceanShaders["surface"]->AddUniform("texWaveFFT", ParameterType::INT);
    oceanShaders["surface"]->AddUniform("texSlopeVariance", ParameterType::INT);
    oceanShaders["surface"]->AddUniform("MVP", ParameterType::MAT4);
    oceanShaders["surface"]->AddUniform("gridSizes", ParameterType::VEC4);
    oceanShaders["surface"]->AddUniform("eyePos", ParameterType::VEC3);
    oceanShaders["surface"]->AddUniform("viewDir", ParameterType::VEC3);
    oceanShaders["surface"]->AddUniform("MV", ParameterType::MAT3);
    oceanShaders["surface"]->AddUniform("FC", ParameterType::FLOAT);
    oceanShaders["surface"]->AddUniform("viewport", ParameterType::VEC2);
    oceanShaders["surface"]->AddUniform("transmittance_texture", ParameterType::INT);
    oceanShaders["surface"]->AddUniform("scattering_texture", ParameterType::INT);
    oceanShaders["surface"]->AddUniform("irradiance_texture", ParameterType::INT);
    oceanShaders["surface"]->AddUniform("sunShadowMap", ParameterType::INT);
    oceanShaders["surface"]->AddUniform("sunDepthMap", ParameterType::INT);
    oceanShaders["surface"]->BindUniformBlock("SunSky", UBO_SUNSKY);

    oceanShaders["surface"]->Use();
    oceanShaders["surface"]->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    oceanShaders["surface"]->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    oceanShaders["surface"]->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);
    oceanShaders["surface"]->SetUniform("sunDepthMap", TEX_SUN_DEPTH);
    oceanShaders["surface"]->SetUniform("sunShadowMap", TEX_SUN_SHADOW);    
    OpenGLState::UseProgram(0);

    //Surface temeperature rendering
    sources.pop_back();
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanSurfaceTemp.frag"));
    oceanShaders["surfaceTemp"] = new GLSLShader(sources, precompiled);
    oceanShaders["surfaceTemp"]->AddUniform("size", ParameterType::FLOAT);
    oceanShaders["surfaceTemp"]->AddUniform("texWaveFFT", ParameterType::INT);
    oceanShaders["surfaceTemp"]->AddUniform("texSlopeVariance", ParameterType::INT);
    oceanShaders["surfaceTemp"]->AddUniform("MVP", ParameterType::MAT4);
    oceanShaders["surfaceTemp"]->AddUniform("gridSizes", ParameterType::VEC4);
    oceanShaders["surfaceTemp"]->AddUniform("eyePos", ParameterType::VEC3);
    oceanShaders["surfaceTemp"]->AddUniform("viewDir", ParameterType::VEC3);
    oceanShaders["surfaceTemp"]->AddUniform("MV", ParameterType::MAT3);
    oceanShaders["surfaceTemp"]->AddUniform("FC", ParameterType::FLOAT);
    oceanShaders["surfaceTemp"]->AddUniform("viewport", ParameterType::VEC2);
    oceanShaders["surfaceTemp"]->AddUniform("transmittance_texture", ParameterType::INT);
    oceanShaders["surfaceTemp"]->AddUniform("scattering_texture", ParameterType::INT);
    oceanShaders["surfaceTemp"]->AddUniform("irradiance_texture", ParameterType::INT);
    oceanShaders["surfaceTemp"]->AddUniform("sunShadowMap", ParameterType::INT);
    oceanShaders["surfaceTemp"]->AddUniform("sunDepthMap", ParameterType::INT);
    oceanShaders["surfaceTemp"]->AddUniform("waterTemperature", ParameterType::FLOAT);
    oceanShaders["surfaceTemp"]->BindUniformBlock("SunSky", UBO_SUNSKY);

    oceanShaders["surfaceTemp"]->Use();
    oceanShaders["surfaceTemp"]->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    oceanShaders["surfaceTemp"]->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    oceanShaders["surfaceTemp"]->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);
    oceanShaders["surfaceTemp"]->SetUniform("sunDepthMap", TEX_SUN_DEPTH);
    oceanShaders["surfaceTemp"]->SetUniform("sunShadowMap", TEX_SUN_SHADOW);
    OpenGLState::UseProgram(0);
    
    //Backsurface rendering
	GLuint oceanOpticsFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "oceanOptics.frag", "", &compiled);
    precompiled.pop_back();
    precompiled.push_back(oceanOpticsFragment);

    sources.pop_back();
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanBacksurface.frag"));
    oceanShaders["backsurface"] = new GLSLShader(sources, precompiled);
    oceanShaders["backsurface"]->AddUniform("size", ParameterType::FLOAT);
    oceanShaders["backsurface"]->AddUniform("texWaveFFT", ParameterType::INT);
    oceanShaders["backsurface"]->AddUniform("texSlopeVariance", ParameterType::INT);
    oceanShaders["backsurface"]->AddUniform("MVP", ParameterType::MAT4);
    oceanShaders["backsurface"]->AddUniform("gridSizes", ParameterType::VEC4);
    oceanShaders["backsurface"]->AddUniform("eyePos", ParameterType::VEC3);
    oceanShaders["backsurface"]->AddUniform("MV", ParameterType::MAT3);
    oceanShaders["backsurface"]->AddUniform("FC", ParameterType::FLOAT);
    oceanShaders["backsurface"]->AddUniform("viewport", ParameterType::VEC2);
    oceanShaders["backsurface"]->AddUniform("cWater", ParameterType::VEC3);
    oceanShaders["backsurface"]->AddUniform("bWater", ParameterType::VEC3);
    oceanShaders["backsurface"]->AddUniform("transmittance_texture", ParameterType::INT);
    oceanShaders["backsurface"]->AddUniform("scattering_texture", ParameterType::INT);
    oceanShaders["backsurface"]->AddUniform("irradiance_texture", ParameterType::INT);
    oceanShaders["backsurface"]->BindUniformBlock("SunSky", UBO_SUNSKY);
    oceanShaders["backsurface"]->BindUniformBlock("Lights", UBO_LIGHTS);
    glDeleteShader(oceanOpticsFragment);

    oceanShaders["backsurface"]->Use();
    oceanShaders["backsurface"]->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    oceanShaders["backsurface"]->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    oceanShaders["backsurface"]->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);
    OpenGLState::UseProgram(0);

    //Mask rendering
    sources.pop_back();
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "flat.frag"));
    oceanShaders["mask"] = new GLSLShader(sources);
    oceanShaders["mask"]->AddUniform("size", ParameterType::FLOAT);
    oceanShaders["mask"]->AddUniform("MVP", ParameterType::MAT4);
    oceanShaders["mask"]->AddUniform("FC", ParameterType::FLOAT);    
    
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
    
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    OpenGLState::BindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(surfData), surfData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    OpenGLState::BindVertexArray(0);

    InitializeSimulation();
}

OpenGLFlatOcean::~OpenGLFlatOcean()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}
    
void OpenGLFlatOcean::UpdateSurface(OpenGLView* view)
{
}
        
void OpenGLFlatOcean::DrawSurface(OpenGLView* view)
{
    GLint* viewport = view->GetViewport();
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_3D, oceanTextures[2]);

    oceanShaders["surface"]->Use();
    oceanShaders["surface"]->SetUniform("MVP", view->GetProjectionMatrix() * view->GetViewMatrix());
    oceanShaders["surface"]->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view->GetViewMatrix()))));
    oceanShaders["surface"]->SetUniform("FC", view->GetLogDepthConstant());
    oceanShaders["surface"]->SetUniform("size", oceanSize);
    oceanShaders["surface"]->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
    oceanShaders["surface"]->SetUniform("eyePos", view->GetEyePosition());
    oceanShaders["surface"]->SetUniform("viewDir", view->GetLookingDirection());
    oceanShaders["surface"]->SetUniform("gridSizes", params.gridSizes);
    oceanShaders["surface"]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
    oceanShaders["surface"]->SetUniform("texSlopeVariance", TEX_POSTPROCESS2);
    OpenGLState::BindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    OpenGLState::BindVertexArray(0);

    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    delete [] viewport;
}

void OpenGLFlatOcean::DrawSurfaceTemperature(OpenGLView* view)
{
    GLint* viewport = view->GetViewport();
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_3D, oceanTextures[2]);

    oceanShaders["surfaceTemp"]->Use();
    oceanShaders["surfaceTemp"]->SetUniform("MVP", view->GetProjectionMatrix() * view->GetViewMatrix());
    oceanShaders["surfaceTemp"]->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view->GetViewMatrix()))));
    oceanShaders["surfaceTemp"]->SetUniform("FC", view->GetLogDepthConstant());
    oceanShaders["surfaceTemp"]->SetUniform("size", oceanSize);
    oceanShaders["surfaceTemp"]->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
    oceanShaders["surfaceTemp"]->SetUniform("eyePos", view->GetEyePosition());
    oceanShaders["surfaceTemp"]->SetUniform("viewDir", view->GetLookingDirection());
    oceanShaders["surfaceTemp"]->SetUniform("gridSizes", params.gridSizes);
    oceanShaders["surfaceTemp"]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
    oceanShaders["surfaceTemp"]->SetUniform("texSlopeVariance", TEX_POSTPROCESS2);
    oceanShaders["surfaceTemp"]->SetUniform("waterTemperature", waterTemperature);

    OpenGLState::BindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    OpenGLState::BindVertexArray(0);

    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    delete [] viewport;
}
        
void OpenGLFlatOcean::DrawBacksurface(OpenGLView* view)
{
    GLint* viewport = view->GetViewport();
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_3D, oceanTextures[2]);

    oceanShaders["backsurface"]->Use();
    oceanShaders["backsurface"]->SetUniform("MVP", view->GetProjectionMatrix() * view->GetViewMatrix());
    oceanShaders["backsurface"]->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view->GetViewMatrix()))));
    oceanShaders["backsurface"]->SetUniform("FC", view->GetLogDepthConstant());
    oceanShaders["backsurface"]->SetUniform("size", oceanSize);
    oceanShaders["backsurface"]->SetUniform("eyePos", view->GetEyePosition());
    oceanShaders["backsurface"]->SetUniform("gridSizes", params.gridSizes);
    oceanShaders["backsurface"]->SetUniform("cWater", getLightAttenuation());
    oceanShaders["backsurface"]->SetUniform("bWater", getLightScattering());
    oceanShaders["backsurface"]->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
    oceanShaders["backsurface"]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
    oceanShaders["backsurface"]->SetUniform("texSlopeVariance", TEX_POSTPROCESS2);
    OpenGLState::BindVertexArray(vao);
    glCullFace(GL_FRONT);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    glCullFace(GL_BACK);
    OpenGLState::BindVertexArray(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    OpenGLState::UseProgram(0);
    delete [] viewport;
}
        
void OpenGLFlatOcean::DrawUnderwaterMask(OpenGLView* view)
{
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    oceanShaders["mask"]->Use();
    oceanShaders["mask"]->SetUniform("MVP", view->GetProjectionMatrix() * view->GetViewMatrix());
    oceanShaders["mask"]->SetUniform("FC", view->GetLogDepthConstant());
    oceanShaders["mask"]->SetUniform("size", oceanSize);
    
    //1. Draw surface to depth buffer
    OpenGLState::BindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    OpenGLState::BindVertexArray(0);
    
    //2. Draw backsurface to depth and stencil buffer
    OpenGLState::EnableStencilTest();
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);
    glCullFace(GL_FRONT);
    
    OpenGLState::BindVertexArray(vao);
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
