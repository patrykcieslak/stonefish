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
//  OpenGLRealOcean.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/05/2020.
//  Copyright (c) 2020-2024 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLRealOcean.h"

#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLView.h"
#include "graphics/OpenGLAtmosphere.h"
#include "graphics/OpenGLConsole.h"
#include "utils/SystemUtil.hpp"

namespace sf
{

OpenGLRealOcean::OpenGLRealOcean(GLfloat size, GLfloat state, SDL_mutex* hydrodynamics) : OpenGLOcean(size)
{
    hydroMutex = hydrodynamics;
    params.wind = state*5.f + 2.f;
    params.A = 1.f;
    params.omega = 5.f*expf(-state) + 0.2f;
    GLint layers = 4;    
    qtGridTessFactor = 8; // Patch tessellation [2, 256]
    qtGPUTessFactor = 0;  // GPU tessellation factor [0,5]
    qtPatchIndexCount = 0;
    wireframe = false;
    
    //Loading shaders
    std::vector<GLSLSource> sources;
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "qt.comp"));
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "oceanSurface.glsl"));
    oceanShaders["lod"] = new GLSLShader(sources);
    oceanShaders["lod"]->AddUniform("sceneSize", ParameterType::FLOAT);
	oceanShaders["lod"]->BindUniformBlock("View", UBO_VIEW);
	oceanShaders["lod"]->BindShaderStorageBlock("QTreeIn", SSBO_QTREE_IN);
	oceanShaders["lod"]->BindShaderStorageBlock("QTreeOut", SSBO_QTREE_OUT);
	oceanShaders["lod"]->BindShaderStorageBlock("QTreeCull", SSBO_QTREE_CULL);
    oceanShaders["lod"]->BindShaderStorageBlock("TreeSize", SSBO_QTREE_SIZE);
	oceanShaders["lod"]->AddUniform("gridSizes", ParameterType::VEC4);
    oceanShaders["lod"]->AddUniform("texWaveFFT", ParameterType::INT);

    sources.clear();
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "qtIndirect.comp"));
    oceanShaders["indirect"] = new GLSLShader(sources);
    oceanShaders["indirect"]->BindShaderStorageBlock("DispatchIndirect", SSBO_QTREE_INDIRECT);
    oceanShaders["indirect"]->BindShaderStorageBlock("TreeSize", SSBO_QTREE_SIZE);

    sources.clear();
    GLint compiled;
    GLuint pcssFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "lighting.frag", "", &compiled);
    std::vector<GLuint> precompiled;
    precompiled.push_back(OpenGLAtmosphere::getAtmosphereAPI());
    precompiled.push_back(GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "cookTorrance.frag", "", &compiled));
    precompiled.push_back(pcssFragment);

    //Surface rendering
    sources.push_back(GLSLSource(GL_VERTEX_SHADER, "qt.vert"));
    sources.push_back(GLSLSource(GL_TESS_CONTROL_SHADER, "qt.tesc"));
    sources.push_back(GLSLSource(GL_TESS_CONTROL_SHADER, "oceanSurface.glsl"));
    sources.push_back(GLSLSource(GL_TESS_EVALUATION_SHADER, "qt.tese"));
    sources.push_back(GLSLSource(GL_TESS_EVALUATION_SHADER, "oceanSurface.glsl"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanSurface.frag"));
    oceanShaders["surface"] = new GLSLShader(sources, precompiled);
    oceanShaders["surface"]->AddUniform("u_scene_size", ParameterType::FLOAT);
    oceanShaders["surface"]->AddUniform("eyePos", ParameterType::VEC3);
    oceanShaders["surface"]->AddUniform("viewDir", ParameterType::VEC3);
    oceanShaders["surface"]->AddUniform("MVP", ParameterType::MAT4);
    oceanShaders["surface"]->AddUniform("MV", ParameterType::MAT3);
    oceanShaders["surface"]->AddUniform("FC", ParameterType::FLOAT);
    oceanShaders["surface"]->AddUniform("gridSizes", ParameterType::VEC4);
    oceanShaders["surface"]->AddUniform("u_gpu_tess_factor", ParameterType::FLOAT);
    oceanShaders["surface"]->AddUniform("texWaveFFT", ParameterType::INT);
	oceanShaders["surface"]->AddUniform("texSlopeVariance", ParameterType::INT);
    oceanShaders["surface"]->AddUniform("viewport", ParameterType::VEC2);
    oceanShaders["surface"]->AddUniform("transmittance_texture", ParameterType::INT);
    oceanShaders["surface"]->AddUniform("scattering_texture", ParameterType::INT);
    oceanShaders["surface"]->AddUniform("irradiance_texture", ParameterType::INT);
    oceanShaders["surface"]->AddUniform("sunShadowMap", ParameterType::INT);
    oceanShaders["surface"]->AddUniform("sunDepthMap", ParameterType::INT);
    oceanShaders["surface"]->BindUniformBlock("SunSky", UBO_SUNSKY);
    oceanShaders["surface"]->BindShaderStorageBlock("QTreeCull", SSBO_QTREE_CULL);
    
    oceanShaders["surface"]->Use();
    oceanShaders["surface"]->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    oceanShaders["surface"]->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    oceanShaders["surface"]->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);
    oceanShaders["surface"]->SetUniform("sunDepthMap", TEX_SUN_DEPTH);
    oceanShaders["surface"]->SetUniform("sunShadowMap", TEX_SUN_SHADOW);    
    OpenGLState::UseProgram(0);

    //Surface temperature rendering
    sources.pop_back();
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanSurfaceTemp.frag"));
    oceanShaders["surfaceTemp"] = new GLSLShader(sources, precompiled);
    oceanShaders["surfaceTemp"]->AddUniform("u_scene_size", ParameterType::FLOAT);
    oceanShaders["surfaceTemp"]->AddUniform("eyePos", ParameterType::VEC3);
    oceanShaders["surfaceTemp"]->AddUniform("viewDir", ParameterType::VEC3);
    oceanShaders["surfaceTemp"]->AddUniform("MVP", ParameterType::MAT4);
    oceanShaders["surfaceTemp"]->AddUniform("MV", ParameterType::MAT3);
    oceanShaders["surfaceTemp"]->AddUniform("FC", ParameterType::FLOAT);
    oceanShaders["surfaceTemp"]->AddUniform("gridSizes", ParameterType::VEC4);
    oceanShaders["surfaceTemp"]->AddUniform("u_gpu_tess_factor", ParameterType::FLOAT);
    oceanShaders["surfaceTemp"]->AddUniform("texWaveFFT", ParameterType::INT);
	oceanShaders["surfaceTemp"]->AddUniform("texSlopeVariance", ParameterType::INT);
    oceanShaders["surfaceTemp"]->AddUniform("viewport", ParameterType::VEC2);
    oceanShaders["surfaceTemp"]->AddUniform("transmittance_texture", ParameterType::INT);
    oceanShaders["surfaceTemp"]->AddUniform("scattering_texture", ParameterType::INT);
    oceanShaders["surfaceTemp"]->AddUniform("irradiance_texture", ParameterType::INT);
    oceanShaders["surfaceTemp"]->AddUniform("sunShadowMap", ParameterType::INT);
    oceanShaders["surfaceTemp"]->AddUniform("sunDepthMap", ParameterType::INT);
    oceanShaders["surfaceTemp"]->AddUniform("waterTemperature", ParameterType::FLOAT);
    oceanShaders["surfaceTemp"]->BindUniformBlock("SunSky", UBO_SUNSKY);
    oceanShaders["surfaceTemp"]->BindShaderStorageBlock("QTreeCull", SSBO_QTREE_CULL);
    
    oceanShaders["surfaceTemp"]->Use();
    oceanShaders["surfaceTemp"]->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    oceanShaders["surfaceTemp"]->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    oceanShaders["surfaceTemp"]->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);
    oceanShaders["surfaceTemp"]->SetUniform("sunDepthMap", TEX_SUN_DEPTH);
    oceanShaders["surfaceTemp"]->SetUniform("sunShadowMap", TEX_SUN_SHADOW);    
    OpenGLState::UseProgram(0);    

    //Back surface rendering
    precompiled.pop_back();
    sources.pop_back();
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanBacksurface.frag"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanOptics.frag"));
    oceanShaders["backsurface"] = new GLSLShader(sources, precompiled);
    oceanShaders["backsurface"]->AddUniform("u_scene_size", ParameterType::FLOAT);
    oceanShaders["backsurface"]->AddUniform("eyePos", ParameterType::VEC3);
    oceanShaders["backsurface"]->AddUniform("MVP", ParameterType::MAT4);
    oceanShaders["backsurface"]->AddUniform("MV", ParameterType::MAT3);
    oceanShaders["backsurface"]->AddUniform("FC", ParameterType::FLOAT);
    oceanShaders["backsurface"]->AddUniform("gridSizes", ParameterType::VEC4);
    oceanShaders["backsurface"]->AddUniform("u_gpu_tess_factor", ParameterType::FLOAT);
    oceanShaders["backsurface"]->AddUniform("texWaveFFT", ParameterType::INT);
	oceanShaders["backsurface"]->AddUniform("texSlopeVariance", ParameterType::INT);
    oceanShaders["backsurface"]->AddUniform("viewport", ParameterType::VEC2);
    oceanShaders["backsurface"]->AddUniform("cWater", ParameterType::VEC3);
    oceanShaders["backsurface"]->AddUniform("bWater", ParameterType::VEC3);
    oceanShaders["backsurface"]->AddUniform("transmittance_texture", ParameterType::INT);
    oceanShaders["backsurface"]->AddUniform("scattering_texture", ParameterType::INT);
    oceanShaders["backsurface"]->AddUniform("irradiance_texture", ParameterType::INT);
    oceanShaders["backsurface"]->BindUniformBlock("SunSky", UBO_SUNSKY);
    oceanShaders["backsurface"]->BindShaderStorageBlock("QTreeCull", SSBO_QTREE_CULL);
    
    oceanShaders["backsurface"]->Use();
    oceanShaders["backsurface"]->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    oceanShaders["backsurface"]->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    oceanShaders["backsurface"]->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);
    OpenGLState::UseProgram(0);

    //Mask rendering
    sources.pop_back();
    sources.pop_back();
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "flat.frag"));
    oceanShaders["mask"] = new GLSLShader(sources);
    oceanShaders["mask"]->AddUniform("u_scene_size", ParameterType::FLOAT);
    oceanShaders["mask"]->AddUniform("eyePos", ParameterType::VEC3);
    oceanShaders["mask"]->AddUniform("texWaveFFT", ParameterType::INT);
    oceanShaders["mask"]->AddUniform("gridSizes", ParameterType::VEC4);
    oceanShaders["mask"]->AddUniform("MVP", ParameterType::MAT4);
    oceanShaders["mask"]->AddUniform("FC", ParameterType::FLOAT);    
    oceanShaders["mask"]->AddUniform("u_gpu_tess_factor", ParameterType::FLOAT);
    oceanShaders["mask"]->BindShaderStorageBlock("QTreeCull", SSBO_QTREE_CULL);

    //FFT data transfer
    size_t fftDataSize = params.fftSize * params.fftSize * 4 * layers;
    fftData = new GLfloat[fftDataSize];
    memset(fftData, 0, sizeof(GLfloat) * fftDataSize);
    
    glGenBuffers(1, &fftPBO);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, fftPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, params.fftSize * params.fftSize * 4 * layers * sizeof(GLfloat), fftData, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    //Quad tree buffers
    glGenBuffers(2, oceanBuffers);
	//Grid vertex data (ARRAY) x2
	size_t vertices_byte_size = sizeof(glm::vec2) * sqr(qtGridTessFactor);
	size_t indexes_byte_size = sizeof(uint16_t) * sqr(qtGridTessFactor - 1) * 4;
	glm::vec2 *vertices = (glm::vec2*)malloc(vertices_byte_size);
	uint16_t *indexes = (uint16_t*)malloc(indexes_byte_size);
	int i, j;

	for(i = 0; i < qtGridTessFactor; ++i)
		for(j = 0; j < qtGridTessFactor; ++j) 
		{
			glm::vec2 *vertex = vertices + i * qtGridTessFactor + j;

			(*vertex)[0] = (float)i / (qtGridTessFactor - 1) - 0.5f;
			(*vertex)[1] = (float)j / (qtGridTessFactor - 1) - 0.5f;
		}

	for(i = 0; i < qtGridTessFactor - 1; ++i)
		for(j = 0; j < qtGridTessFactor - 1; ++j) 
		{
			uint16_t *index = indexes + 4 * i * (qtGridTessFactor - 1) + 4 * j;

			index[0] = i     + qtGridTessFactor *  j;
			index[1] = i + 1 + qtGridTessFactor *  j;
			index[2] = i + 1 + qtGridTessFactor * (j + 1);
			index[3] = i     + qtGridTessFactor * (j + 1);
		}

	glBindBuffer(GL_ARRAY_BUFFER, oceanBuffers[0]); //GRID_VERTICES
	glBufferData(GL_ARRAY_BUFFER, vertices_byte_size, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oceanBuffers[1]); //GRID_INDEXES
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes_byte_size, indexes, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    qtPatchIndexCount = indexes_byte_size / sizeof(uint16_t);
	free(vertices);
	free(indexes);

	//Vertex arrays:
	// - Terrain
	glGenVertexArrays(1, &vao);
	OpenGLState::BindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, oceanBuffers[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oceanBuffers[1]);
	OpenGLState::BindVertexArray(0);

    InitializeSimulation();
}

OpenGLRealOcean::~OpenGLRealOcean()
{
    glDeleteBuffers(2, oceanBuffers);
    glDeleteBuffers(1, &fftPBO);
	glDeleteVertexArrays(1, &vao);
    for(std::map<OpenGLView*, OceanQT>::iterator it=oceanTrees.begin(); it!=oceanTrees.end(); ++it)
    {
        glDeleteBuffers(4, it->second.patchSSBO);
        glDeleteBuffers(1, &it->second.patchDEI);
        glDeleteBuffers(1, &it->second.patchDI);
        glDeleteBuffers(1, &it->second.patchAC);
    }
    oceanTrees.clear();
    delete fftData;
}

void OpenGLRealOcean::setWireframe(bool enabled)
{
    wireframe = enabled;
}

void OpenGLRealOcean::InitializeSimulation()
{
    OpenGLOcean::InitializeSimulation();
}

GLfloat OpenGLRealOcean::ComputeInterpolatedWaveData(GLfloat x, GLfloat y, GLuint channel)
{
    //BILINEAR INTERPOLATION ACCORDING TO OPENGL SPECIFICATION (4.5)
    //Calculate pixel cooridnates
    //x and y are already divided by the phyiscal dimensions of the texture (represented area in [m])
    //so they are directly texture coordinates
    float tmp;
    
    //First coordinate pair
    float i0f = modff(x - 0.5f/(float)params.fftSize, &tmp);
    float j0f = modff(y - 0.5f/(float)params.fftSize, &tmp);
    if(i0f < 0.f) i0f = 1.f - fabsf(i0f);
    if(j0f < 0.f) j0f = 1.f - fabsf(j0f);
    int i0 = (int)truncf(i0f * (float)params.fftSize);
    int j0 = (int)truncf(j0f * (float)params.fftSize);
    
    //Second coordinate pair
    float i1f = modff(x + 0.5f/(float)params.fftSize, &tmp);
    float j1f = modff(y + 0.5f/(float)params.fftSize, &tmp);
    if(i1f < 0.f) i1f = 1.f - fabsf(i1f);
    if(j1f < 0.f) j1f = 1.f - fabsf(j1f);
    int i1 = (int)truncf(i1f * (float)params.fftSize);
    int j1 = (int)truncf(j1f * (float)params.fftSize);
    
    //Calculate weigths
    float alpha = modff(i0f * (float)params.fftSize, &tmp);
    float beta = modff(j0f * (float)params.fftSize, &tmp);
    
    //Get texel values
    float t[4];
    t[0] = fftData[(j0 * params.fftSize + i0) * 4 + channel];
    t[1] = fftData[(j0 * params.fftSize + i1) * 4 + channel];
    t[2] = fftData[(j1 * params.fftSize + i0) * 4 + channel];
    t[3] = fftData[(j1 * params.fftSize + i1) * 4 + channel];
    
    //Interpolate
    float h = (1.f - alpha)*(1.f - beta)*t[0] + alpha*(1.f - beta)*t[1] + (1.f - alpha)*beta*t[2] + alpha*beta*t[3];
    
    return h;
}
    
GLfloat OpenGLRealOcean::ComputeWaveHeight(GLfloat x, GLfloat y)
{
    //Z,X are reversed because the coordinate system used to draw ocean has Z axis pointing up!
    GLfloat z = 0.f;
    z -= ComputeInterpolatedWaveData(x/params.gridSizes.x, y/params.gridSizes.x, 0);
    z -= ComputeInterpolatedWaveData(x/params.gridSizes.y, y/params.gridSizes.y, 1);
    //The components below have low importance and were excluded to lower the computational cost
    //z -= ComputeInterpolatedWaveData(x/params.gridSizes.z, y/params.gridSizes.z, 2);
    //z -= ComputeInterpolatedWaveData(x/params.gridSizes.w, y/params.gridSizes.w, 3);
    return z;
}

void OpenGLRealOcean::Simulate(GLfloat dt)
{
    if(SDL_TryLockMutex(hydroMutex) == 0)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, fftPBO);
        GLfloat* src = (GLfloat*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src)
        {
            memcpy(fftData, src, params.fftSize * params.fftSize * 4 * 4 * sizeof(GLfloat));
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //Release pointer to the mapped buffer
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        SDL_UnlockMutex(hydroMutex);
    }

    OpenGLOcean::Simulate(dt);

    //Copy wave data to RAM for hydrodynamic computations
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, fftPBO);
    glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
}

void OpenGLRealOcean::ResetSurface(OpenGLView* view)
{
    auto it = oceanTrees.find(view); //Check if a quad tree exists for this camera
    if(it != oceanTrees.end())
    {
        //Delete buffers in GPU memory
        glDeleteBuffers(4, it->second.patchSSBO);
        glDeleteBuffers(1, &it->second.patchDEI);
        glDeleteBuffers(1, &it->second.patchDI);
        glDeleteBuffers(1, &it->second.patchAC);
        //Delete map entry
        oceanTrees.erase(it);
    }
}

void OpenGLRealOcean::UpdateSurface(OpenGLView* view)
{
    //Check if a quad tree exists for this camera
    OceanQT* tree;
    try
    {
        tree = &oceanTrees.at(view);
    }
    catch(const std::out_of_range& e)
    {
        OceanQT newTree;
        //Generate buffers
        glGenBuffers(4, newTree.patchSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, newTree.patchSSBO[0]);
	    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) << 16, NULL, GL_STATIC_DRAW);
	    //Init tree structure
        const GLuint dummy[] = {0,0,0,0};
	    GLuint* first = (GLuint*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * 4, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	    memcpy(first, dummy, sizeof(GLuint) * 4);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        
	    glBindBuffer(GL_SHADER_STORAGE_BUFFER, newTree.patchSSBO[1]);
	    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) << 16, NULL, GL_STATIC_DRAW);
	    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, newTree.patchSSBO[2]);
	    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) << 16, NULL, GL_STATIC_DRAW);
	    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        GLuint one = 1;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, newTree.patchSSBO[3]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint), &one, GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        GLuint zero = 0;
        glGenBuffers(1, &newTree.patchAC);
	    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, newTree.patchAC); 
	    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &zero, GL_DYNAMIC_DRAW);
	    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

        DrawElementsIndirectCommand cmd;
        cmd.count = qtPatchIndexCount;
        cmd.instanceCount = 0;
        cmd.firstIndex = 0;
        cmd.baseVertex = 0;
        cmd.baseInstance = 0;
        glGenBuffers(1, &newTree.patchDEI);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, newTree.patchDEI);
        glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawElementsIndirectCommand), &cmd, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

        DispatchIndirectCommand cmd2;
        cmd2.numGroupsX = 1;
        cmd2.numGroupsY = 1;
        cmd2.numGroupsZ = 1;
        glGenBuffers(1, &newTree.patchDI);
        glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, newTree.patchDI);
        glBufferData(GL_DISPATCH_INDIRECT_BUFFER, sizeof(DispatchIndirectCommand), &cmd2, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0);
        
        //Initialize
        newTree.pingpong = 1;
        oceanTrees[view] = newTree;
        tree = &oceanTrees[view];
    }
    
    GLuint zero = 0;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_QTREE_IN, tree->patchSSBO[1 - tree->pingpong]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_QTREE_OUT, tree->patchSSBO[tree->pingpong]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_QTREE_CULL, tree->patchSSBO[2]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_QTREE_SIZE, tree->patchSSBO[3]);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, AC_QTREE_LOD, tree->patchAC);
    glBindBufferRange(GL_ATOMIC_COUNTER_BUFFER, AC_QTREE_CULL, tree->patchDEI, sizeof(GLuint), sizeof(GLuint));
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, tree->patchDEI);
    glBufferSubData(GL_DRAW_INDIRECT_BUFFER, sizeof(GLuint), sizeof(GLuint), &zero);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    tree->pingpong = 1 - tree->pingpong;

    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
	oceanShaders["lod"]->Use();
    oceanShaders["lod"]->SetUniform("sceneSize", oceanSize);
	oceanShaders["lod"]->SetUniform("gridSizes", params.gridSizes);
	oceanShaders["lod"]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, tree->patchDI);
    glDispatchComputeIndirect(0);
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0);
	OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
    
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, SSBO_QTREE_INDIRECT, tree->patchDI, 0, sizeof(GLuint));
    oceanShaders["indirect"]->Use();
    glDispatchCompute(1, 1, 1);
    OpenGLState::UseProgram(0);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, tree->patchAC);
    glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}
        
void OpenGLRealOcean::DrawSurface(OpenGLView* view)
{
    OceanQT& tree = oceanTrees[view];
    GLint* viewport = view->GetViewport();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_QTREE_CULL, tree.patchSSBO[2]);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_3D, oceanTextures[2]);
    oceanShaders["surface"]->Use();
    oceanShaders["surface"]->SetUniform("MVP", view->GetProjectionMatrix() * view->GetViewMatrix());
    oceanShaders["surface"]->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view->GetViewMatrix()))));
    oceanShaders["surface"]->SetUniform("FC", view->GetLogDepthConstant());
    oceanShaders["surface"]->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
    oceanShaders["surface"]->SetUniform("eyePos", view->GetEyePosition());
    oceanShaders["surface"]->SetUniform("viewDir", view->GetLookingDirection());
    oceanShaders["surface"]->SetUniform("gridSizes", params.gridSizes);
    oceanShaders["surface"]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
    oceanShaders["surface"]->SetUniform("texSlopeVariance", TEX_POSTPROCESS2);
    oceanShaders["surface"]->SetUniform("u_scene_size", oceanSize);
    oceanShaders["surface"]->SetUniform("u_gpu_tess_factor", (GLfloat)qtGPUTessFactor);
    OpenGLState::BindVertexArray(vao);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, tree.patchDEI);
	if(wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElementsIndirect(GL_PATCHES, GL_UNSIGNED_SHORT, NULL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else
    {
        glDrawElementsIndirect(GL_PATCHES, GL_UNSIGNED_SHORT, NULL);
    }
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	OpenGLState::BindVertexArray(0);
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    delete [] viewport;
}

void OpenGLRealOcean::DrawSurfaceTemperature(OpenGLView* view)
{
    OceanQT& tree = oceanTrees[view];
    GLint* viewport = view->GetViewport();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_QTREE_CULL, tree.patchSSBO[2]);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_3D, oceanTextures[2]);
    oceanShaders["surfaceTemp"]->Use();
    oceanShaders["surfaceTemp"]->SetUniform("MVP", view->GetProjectionMatrix() * view->GetViewMatrix());
    oceanShaders["surfaceTemp"]->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view->GetViewMatrix()))));
    oceanShaders["surfaceTemp"]->SetUniform("FC", view->GetLogDepthConstant());
    oceanShaders["surfaceTemp"]->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
    oceanShaders["surfaceTemp"]->SetUniform("eyePos", view->GetEyePosition());
    oceanShaders["surfaceTemp"]->SetUniform("viewDir", view->GetLookingDirection());
    oceanShaders["surfaceTemp"]->SetUniform("gridSizes", params.gridSizes);
    oceanShaders["surfaceTemp"]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
    oceanShaders["surfaceTemp"]->SetUniform("texSlopeVariance", TEX_POSTPROCESS2);
    oceanShaders["surfaceTemp"]->SetUniform("u_scene_size", oceanSize);
    oceanShaders["surfaceTemp"]->SetUniform("u_gpu_tess_factor", (GLfloat)qtGPUTessFactor);
    oceanShaders["surfaceTemp"]->SetUniform("waterTemperature", waterTemperature);
    OpenGLState::BindVertexArray(vao);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, tree.patchDEI);
	glDrawElementsIndirect(GL_PATCHES, GL_UNSIGNED_SHORT, NULL);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	OpenGLState::BindVertexArray(0);
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    delete [] viewport;
}
        
void OpenGLRealOcean::DrawBacksurface(OpenGLView* view)
{
    OceanQT& tree = oceanTrees[view];
    GLint* viewport = view->GetViewport();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_QTREE_CULL, tree.patchSSBO[2]);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_3D, oceanTextures[2]);
    oceanShaders["backsurface"]->Use();
    oceanShaders["backsurface"]->SetUniform("MVP", view->GetProjectionMatrix() * view->GetViewMatrix());
    oceanShaders["backsurface"]->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view->GetViewMatrix()))));
    oceanShaders["backsurface"]->SetUniform("FC", view->GetLogDepthConstant());
    oceanShaders["backsurface"]->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
    oceanShaders["backsurface"]->SetUniform("eyePos", view->GetEyePosition());
    oceanShaders["backsurface"]->SetUniform("gridSizes", params.gridSizes);
    oceanShaders["backsurface"]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
    oceanShaders["backsurface"]->SetUniform("texSlopeVariance", TEX_POSTPROCESS2);
    oceanShaders["backsurface"]->SetUniform("u_scene_size", oceanSize);
    oceanShaders["backsurface"]->SetUniform("u_gpu_tess_factor", (GLfloat)qtGPUTessFactor);
    oceanShaders["backsurface"]->SetUniform("cWater", getLightAttenuation());
    oceanShaders["backsurface"]->SetUniform("bWater", getLightScattering());
    OpenGLState::BindVertexArray(vao);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, tree.patchDEI);
    glCullFace(GL_FRONT);
    if(wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElementsIndirect(GL_PATCHES, GL_UNSIGNED_SHORT, NULL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else
    {
        glDrawElementsIndirect(GL_PATCHES, GL_UNSIGNED_SHORT, NULL);   
    }
    glCullFace(GL_BACK);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	OpenGLState::BindVertexArray(0);
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    delete [] viewport;
}
        
void OpenGLRealOcean::DrawUnderwaterMask(OpenGLView* view)
{
    OceanQT& tree = oceanTrees[view];
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_QTREE_CULL, tree.patchSSBO[2]);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
    OpenGLState::BindVertexArray(vao);

    oceanShaders["mask"]->Use();
    oceanShaders["mask"]->SetUniform("MVP", view->GetProjectionMatrix() * view->GetViewMatrix());
    oceanShaders["mask"]->SetUniform("FC", view->GetLogDepthConstant());
    oceanShaders["mask"]->SetUniform("gridSizes", params.gridSizes);
    oceanShaders["mask"]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
    oceanShaders["mask"]->SetUniform("eyePos", view->GetEyePosition());
    oceanShaders["mask"]->SetUniform("u_scene_size", oceanSize);
    oceanShaders["mask"]->SetUniform("u_gpu_tess_factor", (GLfloat)qtGPUTessFactor);
    
    //1. Draw surface to depth buffer
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, tree.patchDEI);
    glDrawElementsIndirect(GL_PATCHES, GL_UNSIGNED_SHORT, NULL);

    //2. Draw backsurface to depth and stencil buffer
    OpenGLState::EnableStencilTest();
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);
    glCullFace(GL_FRONT);

    glDrawElementsIndirect(GL_PATCHES, GL_UNSIGNED_SHORT, NULL);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

    OpenGLState::BindVertexArray(0);
	OpenGLState::UseProgram(0);
	OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    
    //3. Draw box around whole ocean
    OpenGLOcean::DrawUnderwaterMask(view);
    
    glCullFace(GL_BACK);
    OpenGLState::DisableStencilTest();
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);
}        
    
}
