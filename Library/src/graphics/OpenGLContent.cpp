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
//  OpenGLContent.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/06/17.
//  Copyright (c) 2017-2019 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLContent.h"

#include <map>
#include <algorithm>
#include "core/Console.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLAtmosphere.h"
#include "graphics/OpenGLView.h"
#include "graphics/OpenGLLight.h"
#include "graphics/OpenGLOcean.h"
#include "entities/forcefields/Ocean.h"
#include "entities/forcefields/Atmosphere.h"
#include "utils/SystemUtil.hpp"
#include "utils/GeometryFileUtil.h"
#define STB_IMAGE_IMPLEMENTATION
#include "utils/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "utils/stb_image_write.h"

#define clamp(x,min,max)     (x > max ? max : (x < min ? min : x))

namespace sf
{

OpenGLContent::OpenGLContent()
{
    //Initialize members
    baseVertexArray = 0;
    quadBuf = 0;
    cubeBuf = 0;
    lightsUBO = 0;
    csBuf[0] = 0;
    csBuf[1] = 0;
    cylinder.vao = 0;
    ellipsoid.vao = 0;
    lightSourceShader[0] = lightSourceShader[1] = NULL;
    eyePos = glm::vec3();
    viewDir = glm::vec3(1.f,0,0);
    viewProjection = glm::mat4();
    view = glm::mat4();
    projection = glm::mat4();
    FC = 0.f;
    viewportSize = glm::vec2(800.f,600.f);
    mode = DrawingMode::FULL;
    materialShaders = std::vector<GLSLShader*>(0);
    currentLookId = -1;

    //Get OpenGL capabilities
    maxAnisotropy = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);

    //Initialize shaders and buffers
    glGenVertexArrays(1, &baseVertexArray);
    
    //Build quad texture VBO
    GLfloat quadData[4][4] = {{-1.f, -1.f, 0.f, 0.f},
        {-1.f,  1.f, 0.f, 1.f},
        { 1.f, -1.f, 1.f, 0.f},
        { 1.f,  1.f, 1.f, 1.f}};
    
    glGenBuffers(1, &quadBuf);
    glBindBuffer(GL_ARRAY_BUFFER, quadBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    //Build cube croos VBO
    GLfloat cubeData[24][5] = {{-1.f,  0.333f, -1.f, 1.f, 1.f}, //LEFT
        {-1.f, -0.333f, -1.f,-1.f, 1.f},
        {-0.5f, 0.333f, -1.f, 1.f,-1.f},
        {-0.5f,-0.333f, -1.f,-1.f,-1.f},
        {-0.5f, 0.333f, -1.f, 1.f,-1.f}, //FRONT
        {-0.5f,-0.333f, -1.f,-1.f,-1.f},
        { 0.f,  0.333f,  1.f, 1.f,-1.f},
        { 0.f, -0.333f,  1.f,-1.f,-1.f},
        { 0.f,  0.333f,  1.f, 1.f,-1.f}, //RIGHT
        { 0.f, -0.333f,  1.f,-1.f,-1.f},
        { 0.5f, 0.333f,  1.f, 1.f, 1.f},
        { 0.5f,-0.333f,  1.f,-1.f, 1.f},
        { 0.5f, 0.333f,  1.f, 1.f, 1.f}, //BACK
        { 0.5f,-0.333f,  1.f,-1.f, 1.f},
        {  1.f, 0.333f, -1.f, 1.f, 1.f},
        {  1.f,-0.333f, -1.f,-1.f, 1.f},
        {-0.5f,-0.333f, -1.f,-1.f, 1.f}, //BOTTOM
        {-0.5f,   -1.f, -1.f,-1.f,-1.f},
        {    0,-0.333f,  1.f,-1.f, 1.f},
        {    0,   -1.f,  1.f,-1.f,-1.f},
        {-0.5f,    1.f, -1.f, 1.f, 1.f}, //TOP
        {-0.5f, 0.333f, -1.f, 1.f,-1.f},
        {    0,    1.f,  1.f, 1.f, 1.f},
        {    0, 0.333f,  1.f, 1.f,-1.f}};
    
    glGenBuffers(1, &cubeBuf);
    glBindBuffer(GL_ARRAY_BUFFER, cubeBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeData), cubeData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    //Build coord system VBO
    GLfloat csVertex[6][3] = {{0.f, 0.f, 0.f},
        {1.f, 0.f, 0.f},
        {0.f, 0.f, 0.f},
        {0.f, 1.f, 0.f},
        {0.f, 0.f, 0.f},
        {0.f, 0.f, 1.f}};
    
    GLfloat csColor[6][4] = {{1.f, 0.f, 0.f, 1.f},
        {1.f, 0.f, 0.f, 1.f},
        {0.f, 1.f, 0.f, 1.f},
        {0.f, 1.f, 0.f, 1.f},
        {0.f, 0.f, 1.f, 1.f},
        {0.f, 0.f, 1.f, 1.f}};
    
    glGenBuffers(2, csBuf);
    glBindBuffer(GL_ARRAY_BUFFER, csBuf[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(csVertex), csVertex, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, csBuf[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(csColor), csColor, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    //Cylinder helper
    Mesh* m = BuildCylinder(1.f, 1.f, 12);

    glGenVertexArrays(1, &cylinder.vao);
    glGenBuffers(1, &cylinder.vboVertex);
    glGenBuffers(1, &cylinder.vboIndex);
    cylinder.faceCount = (GLsizei)m->faces.size();

    OpenGLState::BindVertexArray(cylinder.vao);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, cylinder.vboVertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*m->vertices.size(), &m->vertices[0].pos.x, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinder.vboIndex);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Face)*m->faces.size(), &m->faces[0].vertexID[0], GL_STATIC_DRAW);
    OpenGLState::BindVertexArray(0);
    
    delete m;

    //Ellipsoid helper
    m = BuildSphere(1.f, 3);
    
    glGenVertexArrays(1, &ellipsoid.vao);
    glGenBuffers(1, &ellipsoid.vboVertex);
    glGenBuffers(1, &ellipsoid.vboIndex);
    ellipsoid.faceCount = (GLsizei)m->faces.size();

    OpenGLState::BindVertexArray(ellipsoid.vao);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, ellipsoid.vboVertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)* m->vertices.size(), &m->vertices[0].pos.x, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ellipsoid.vboIndex);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Face) * m->faces.size(), &m->faces[0].vertexID[0], GL_STATIC_DRAW);
    OpenGLState::BindVertexArray(0);

    delete m;

    //Generate UBOs
    glGenBuffers(1, &lightsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, lightsUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(LightsUBO), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, UBO_LIGHTS, lightsUBO, 0, sizeof(LightsUBO));
    memset(&lightsUBOData, 0, sizeof(LightsUBO));

    ViewUBO viewZero;
    viewZero.eye = glm::vec4(0.f);
	viewZero.VP = glm::perspectiveFov(1.57f, 800.f, 600.f, 0.1f, 10000.f);
    OpenGLView::ExtractFrustumFromVP(viewZero.frustum, viewZero.VP);
    glGenBuffers(1, &viewUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, viewUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ViewUBO), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, UBO_VIEW, viewUBO, 0, sizeof(ViewUBO));
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ViewUBO), &viewZero);
    
    //Load shaders
    //-----BASIC-----
    basicShaders["helper"] = new GLSLShader("helpers.frag","helpers.vert");
    basicShaders["helper"]->AddUniform("MVP", ParameterType::MAT4);
    basicShaders["helper"]->AddUniform("scale", ParameterType::VEC3);
    
    basicShaders["tex_saq"] = new GLSLShader("texQuad.frag");
    basicShaders["tex_saq"]->AddUniform("tex", ParameterType::INT);
    basicShaders["tex_saq"]->AddUniform("color", ParameterType::VEC4);
    
    basicShaders["tex_quad"] = new GLSLShader("texQuad.frag","texQuad.vert");
    basicShaders["tex_quad"]->AddUniform("rect", ParameterType::VEC4);
    basicShaders["tex_quad"]->AddUniform("tex", ParameterType::INT);
    basicShaders["tex_quad"]->AddUniform("color", ParameterType::VEC4);
    
    basicShaders["tex_layer_quad"] = new GLSLShader("texLayerQuad.frag", "texQuad.vert");
    basicShaders["tex_layer_quad"]->AddUniform("rect", ParameterType::VEC4);
    basicShaders["tex_layer_quad"]->AddUniform("tex", ParameterType::INT);
    basicShaders["tex_layer_quad"]->AddUniform("layer", ParameterType::INT);
    
    basicShaders["tex_level_quad"] = new GLSLShader("texLevelQuad.frag", "texQuad.vert");
    basicShaders["tex_level_quad"]->AddUniform("rect", ParameterType::VEC4);
    basicShaders["tex_level_quad"]->AddUniform("tex", ParameterType::INT);
    basicShaders["tex_level_quad"]->AddUniform("level", ParameterType::INT);
    
    basicShaders["tex_cube"] = new GLSLShader("texCube.frag", "texCube.vert");
    basicShaders["tex_cube"]->AddUniform("tex", ParameterType::INT);
    
    basicShaders["flat"] = new GLSLShader("flat.frag", "flat.vert");
    basicShaders["flat"]->AddUniform("MVP", ParameterType::MAT4);
    basicShaders["flat"]->AddUniform("FC", ParameterType::FLOAT);

    basicShaders["shadow"] = new GLSLShader("shadow.frag", "shadow.vert");
    basicShaders["shadow"]->AddUniform("MVP", ParameterType::MAT4);
    
    //-----MATERIALS-----
    GLint compiled; 
	std::vector<GLuint> commonMaterialShaders;
    commonMaterialShaders.push_back(OpenGLAtmosphere::getAtmosphereAPI());
    
	GLuint pcssFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "lighting.frag", "", &compiled);
	commonMaterialShaders.push_back(pcssFragment);
	
	GLuint materialFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "material.frag", "", &compiled);
	commonMaterialShaders.push_back(materialFragment);
	
    //Blinn-Phong shader
    GLSLShader* blinnPhong = new GLSLShader(commonMaterialShaders, "blinnPhong.frag", "material.vert");
    blinnPhong->AddUniform("shininess", ParameterType::FLOAT);
    blinnPhong->AddUniform("specularStrength", ParameterType::FLOAT);
    blinnPhong->AddUniform("reflectivity", ParameterType::FLOAT);
    materialShaders.push_back(blinnPhong);
    
    //Cook-Torrance shader
    GLSLShader* cookTorrance = new GLSLShader(commonMaterialShaders, "cookTorrance.frag", "material.vert");
    cookTorrance->AddUniform("roughness", ParameterType::FLOAT);
    cookTorrance->AddUniform("metallic", ParameterType::FLOAT);
    cookTorrance->AddUniform("reflectivity", ParameterType::FLOAT);
    materialShaders.push_back(cookTorrance);
    glDeleteShader(materialFragment);
	
    //-------------Underwater shaders---------------------------
    commonMaterialShaders.pop_back(); //"material.frag"
	GLuint oceanOpticsFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "oceanOptics.frag", "", &compiled);
    commonMaterialShaders.push_back(oceanOpticsFragment);
    materialFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "uwMaterial.frag", "", &compiled);
    commonMaterialShaders.push_back(materialFragment);
    GLuint oceanSurfaceFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "oceanSurfaceFlat.glsl", "", &compiled);
    commonMaterialShaders.push_back(oceanSurfaceFragment);
    
    //Underwater Blinn-Phong shader
    GLSLShader* uwBlinnPhong = new GLSLShader(commonMaterialShaders, "blinnPhong.frag", "material.vert");
    uwBlinnPhong->AddUniform("shininess", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("specularStrength", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("reflectivity", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("cWater", ParameterType::VEC3);
    uwBlinnPhong->AddUniform("bWater", ParameterType::VEC3);
    materialShaders.push_back(uwBlinnPhong);
    
    //Underwater Cook-Torrance shader
    GLSLShader* uwCookTorrance = new GLSLShader(commonMaterialShaders, "cookTorrance.frag", "material.vert");
    uwCookTorrance->AddUniform("roughness", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("metallic", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("reflectivity", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("cWater", ParameterType::VEC3);
    uwCookTorrance->AddUniform("bWater", ParameterType::VEC3);
    materialShaders.push_back(uwCookTorrance);
    
    commonMaterialShaders.pop_back(); //"oceanSurfaceFlat.glsl"
    glDeleteShader(oceanSurfaceFragment);
    oceanSurfaceFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "oceanSurface.glsl", "", &compiled);
    commonMaterialShaders.push_back(oceanSurfaceFragment);

    //Underwater Blinn-Phong shader (waves)
    uwBlinnPhong = new GLSLShader(commonMaterialShaders, "blinnPhong.frag", "material.vert");
    uwBlinnPhong->AddUniform("shininess", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("specularStrength", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("reflectivity", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("cWater", ParameterType::VEC3);
    uwBlinnPhong->AddUniform("bWater", ParameterType::VEC3);
    uwBlinnPhong->AddUniform("texWaveFFT", ParameterType::INT);
    uwBlinnPhong->AddUniform("gridSizes", ParameterType::VEC4);
    materialShaders.push_back(uwBlinnPhong);
    
    //Underwater Cook-Torrance shader (waves)
    uwCookTorrance = new GLSLShader(commonMaterialShaders, "cookTorrance.frag", "material.vert");
    uwCookTorrance->AddUniform("roughness", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("metallic", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("reflectivity", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("cWater", ParameterType::VEC3);
    uwCookTorrance->AddUniform("bWater", ParameterType::VEC3);
    uwCookTorrance->AddUniform("texWaveFFT", ParameterType::INT);
    uwCookTorrance->AddUniform("gridSizes", ParameterType::VEC4);
    materialShaders.push_back(uwCookTorrance);

    glDeleteShader(materialFragment);
    glDeleteShader(oceanSurfaceFragment);
	
    //Add common uniforms
    for(size_t i=0; i<materialShaders.size(); ++i)
    {
        materialShaders[i]->AddUniform("MVP", ParameterType::MAT4);
        materialShaders[i]->AddUniform("M", ParameterType::MAT4);
        materialShaders[i]->AddUniform("N", ParameterType::MAT3);
        materialShaders[i]->AddUniform("MV", ParameterType::MAT3);
        materialShaders[i]->AddUniform("FC", ParameterType::FLOAT);
        materialShaders[i]->AddUniform("eyePos", ParameterType::VEC3);
        materialShaders[i]->AddUniform("viewDir", ParameterType::VEC3);
        materialShaders[i]->AddUniform("color", ParameterType::VEC4);
        materialShaders[i]->AddUniform("tex", ParameterType::INT);
        materialShaders[i]->AddUniform("spotLightsDepthMap", ParameterType::INT);
        materialShaders[i]->AddUniform("spotLightsShadowMap", ParameterType::INT);
        materialShaders[i]->AddUniform("sunShadowMap", ParameterType::INT);
        materialShaders[i]->AddUniform("sunDepthMap", ParameterType::INT);
        materialShaders[i]->AddUniform("transmittance_texture", ParameterType::INT);
        materialShaders[i]->AddUniform("scattering_texture", ParameterType::INT);
        materialShaders[i]->AddUniform("irradiance_texture", ParameterType::INT);
        materialShaders[i]->BindUniformBlock("SunSky", UBO_SUNSKY);
        materialShaders[i]->BindUniformBlock("Lights", UBO_LIGHTS);
    }

    //Set permanent texture units
    for(size_t i=0; i<materialShaders.size(); ++i)
    {
        materialShaders[i]->Use();
        materialShaders[i]->SetUniform("tex", TEX_MAT_DIFFUSE);
        materialShaders[i]->SetUniform("spotLightsShadowMap", TEX_SPOT_SHADOW);
        materialShaders[i]->SetUniform("spotLightsDepthMap", TEX_SPOT_DEPTH);
        materialShaders[i]->SetUniform("sunDepthMap", TEX_SUN_DEPTH);
        materialShaders[i]->SetUniform("sunShadowMap", TEX_SUN_SHADOW);
        materialShaders[i]->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
        materialShaders[i]->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
        materialShaders[i]->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);
    }

    //Light source rendering shaders
    std::vector<GLuint> commonLightShaders;
    commonLightShaders.push_back(OpenGLAtmosphere::getAtmosphereAPI());
    commonLightShaders.push_back(pcssFragment);
    
    //Above surface
    GLuint lightSourceFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "lightSource.frag", "", &compiled);
	commonLightShaders.push_back(lightSourceFragment);
	
    lightSourceShader[0] = new GLSLShader(commonLightShaders, "light.frag", "material.vert");

    //Under surface
    commonLightShaders.pop_back();
    glDeleteShader(lightSourceFragment);

    commonLightShaders.push_back(oceanOpticsFragment);
    lightSourceFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "uwLightSource.frag", "", &compiled);
	commonLightShaders.push_back(lightSourceFragment);
	
    lightSourceShader[1] = new GLSLShader(commonLightShaders, "light.frag", "material.vert");
    lightSourceShader[1]->AddUniform("cWater", ParameterType::VEC3);
    lightSourceShader[1]->AddUniform("bWater", ParameterType::VEC3);
    
    //Add common uniforms
    for(size_t i=0; i<2; ++i)
    {
        lightSourceShader[i]->AddUniform("MVP", ParameterType::MAT4);
        lightSourceShader[i]->AddUniform("M", ParameterType::MAT4);
        lightSourceShader[i]->AddUniform("N", ParameterType::MAT3);
        lightSourceShader[i]->AddUniform("MV", ParameterType::MAT3);
        lightSourceShader[i]->AddUniform("FC", ParameterType::FLOAT);
        lightSourceShader[i]->AddUniform("eyePos", ParameterType::VEC3);
        lightSourceShader[i]->AddUniform("viewDir", ParameterType::VEC3);
        lightSourceShader[i]->AddUniform("color", ParameterType::VEC3);
        lightSourceShader[i]->AddUniform("lightId", ParameterType::IVEC2);
        lightSourceShader[i]->AddUniform("spotLightsDepthMap", ParameterType::INT);
        lightSourceShader[i]->AddUniform("spotLightsShadowMap", ParameterType::INT);
        lightSourceShader[i]->AddUniform("sunShadowMap", ParameterType::INT);
        lightSourceShader[i]->AddUniform("sunDepthMap", ParameterType::INT);
        lightSourceShader[i]->AddUniform("transmittance_texture", ParameterType::INT);
        lightSourceShader[i]->AddUniform("scattering_texture", ParameterType::INT);
        lightSourceShader[i]->AddUniform("irradiance_texture", ParameterType::INT);
        lightSourceShader[i]->BindUniformBlock("SunSky", UBO_SUNSKY);
        lightSourceShader[i]->BindUniformBlock("Lights", UBO_LIGHTS);
    }

    //Set permanent texture units
    for(size_t i=0; i<2; ++i)
    {
        lightSourceShader[i]->Use();
        lightSourceShader[i]->SetUniform("spotLightsShadowMap", TEX_SPOT_SHADOW);
        lightSourceShader[i]->SetUniform("spotLightsDepthMap", TEX_SPOT_DEPTH);
        lightSourceShader[i]->SetUniform("sunDepthMap", TEX_SUN_DEPTH);
        lightSourceShader[i]->SetUniform("sunShadowMap", TEX_SUN_SHADOW);
        lightSourceShader[i]->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
        lightSourceShader[i]->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
        lightSourceShader[i]->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);
    }

    OpenGLState::UseProgram(0);

    glDeleteShader(pcssFragment);
    glDeleteShader(oceanOpticsFragment);
    glDeleteShader(lightSourceFragment);
}

OpenGLContent::~OpenGLContent()
{
    //Base shaders
    if(baseVertexArray != 0) glDeleteVertexArrays(1, &baseVertexArray);
    if(quadBuf != 0) glDeleteBuffers(1, &quadBuf);
    if(cubeBuf != 0) glDeleteBuffers(1, &cubeBuf);
    if(csBuf[0] != 0) glDeleteBuffers(2, csBuf);
    if(lightsUBO != 0) glDeleteBuffers(1, &lightsUBO);
    if(viewUBO != 0) glDeleteBuffers(1, &viewUBO);
    delete basicShaders["helper"];
    delete basicShaders["tex_saq"];
    delete basicShaders["tex_quad"];
    delete basicShaders["tex_layer_quad"];
    delete basicShaders["tex_level_quad"];
    delete basicShaders["tex_cube"];
    delete basicShaders["flat"];
    delete basicShaders["shadow"];
    if(lightSourceShader[0] != NULL) delete lightSourceShader[0];
    if(lightSourceShader[1] != NULL) delete lightSourceShader[1];
    
    //Material shaders
    for(size_t i=0; i<materialShaders.size(); ++i)
        delete materialShaders[i];
    materialShaders.clear();
    
    //Views
    if(views.size() == 1) //Trackball left after destroying content
        delete views[0];
    views.clear();
}

void OpenGLContent::Finalize()
{
    cInfo("Finalizing OpenGL rendering pipeline...");
    OpenGLLight::Init(lights);
}

void OpenGLContent::DestroyContent()
{
    for(size_t i=0; i<looks.size(); ++i)
    {
        for(size_t h=0; h<looks[i].textures.size(); ++h)
            glDeleteTextures(1, &looks[i].textures[h]);
    }
    looks.clear();
    lookNameManager.ClearNames();
            
    for(size_t i=0; i<objects.size(); ++i)
    {
        glDeleteBuffers(1, &objects[i].vboVertex);
        glDeleteBuffers(1, &objects[i].vboIndex);
        glDeleteVertexArrays(1, &objects[i].vao);
    }	
    objects.clear();
    
    for(size_t i=0; i<views.size(); ++i)
		delete views[i];
	views.clear();
    
    for(size_t i=0; i<lights.size(); ++i)
        delete lights[i];
    lights.clear();
    OpenGLLight::Destroy();
}

void OpenGLContent::SetViewportSize(unsigned int width, unsigned int height)
{
    viewportSize = glm::vec2(width, height);
}

void OpenGLContent::SetProjectionMatrix(glm::mat4 P)
{
    projection = P;
    viewProjection = projection * view;
}

void OpenGLContent::SetViewMatrix(glm::mat4 V)
{
    view = V;
    viewProjection = projection * view;
}

glm::mat4 OpenGLContent::GetViewMatrix()
{
    return view;
}

void OpenGLContent::SetCurrentView(OpenGLView* v)
{
	eyePos = v->GetEyePosition();
    viewDir = v->GetLookingDirection();
    view = v->GetViewMatrix();
    projection = v->GetProjectionMatrix();
    viewProjection = projection * view;
    FC = v->GetLogDepthConstant();

    glBindBuffer(GL_UNIFORM_BUFFER, viewUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ViewUBO), v->getViewUBOData());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLContent::SetDrawingMode(DrawingMode m)
{
    mode = m;
}

void OpenGLContent::BindBaseVertexArray()
{
    OpenGLState::BindVertexArray(baseVertexArray);
}

void OpenGLContent::DrawSAQ()
{
    OpenGLState::BindVertexArray(baseVertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    OpenGLState::BindVertexArray(0);
}

void OpenGLContent::DrawTexturedSAQ(GLuint texture, glm::vec4 color)
{
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, texture);
    basicShaders["tex_saq"]->Use();
    basicShaders["tex_saq"]->SetUniform("tex", TEX_BASE);
    basicShaders["tex_saq"]->SetUniform("color", color);
    
    OpenGLState::BindVertexArray(baseVertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    OpenGLState::BindVertexArray(0);
    
    OpenGLState::UnbindTexture(TEX_BASE);
    OpenGLState::UseProgram(0);
}

void OpenGLContent::DrawTexturedQuad(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLuint texture, glm::vec4 color)
{
    if(basicShaders["tex_quad"] != NULL)
    {
        y = viewportSize.y-y-height;
        
        basicShaders["tex_quad"]->Use();
        basicShaders["tex_quad"]->SetUniform("rect", glm::vec4(x/viewportSize.x, y/viewportSize.y, width/viewportSize.x, height/viewportSize.y));
        basicShaders["tex_quad"]->SetUniform("tex", TEX_BASE);
        basicShaders["tex_quad"]->SetUniform("color", color);
        
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, texture);
        OpenGLState::BindVertexArray(baseVertexArray);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, quadBuf); 
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDisableVertexAttribArray(0);
        OpenGLState::BindVertexArray(0);
        OpenGLState::UnbindTexture(TEX_BASE);
        OpenGLState::UseProgram(0);
    }
}

void OpenGLContent::DrawTexturedQuad(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLuint texture, GLint z, bool array)
{
    if((array && basicShaders["tex_layer_quad"] != NULL)||(!array && basicShaders["tex_level_quad"] != NULL))
    {
        y = viewportSize.y-y-height;
        
        if(array)
        {
            basicShaders["tex_layer_quad"]->Use();
            basicShaders["tex_layer_quad"]->SetUniform("rect", glm::vec4(x/viewportSize.x, y/viewportSize.y, width/viewportSize.x, height/viewportSize.y));
            basicShaders["tex_layer_quad"]->SetUniform("tex", TEX_BASE);
            basicShaders["tex_layer_quad"]->SetUniform("layer", z);
        }
        else
        {
            basicShaders["tex_level_quad"]->Use();
            basicShaders["tex_level_quad"]->SetUniform("rect", glm::vec4(x/viewportSize.x, y/viewportSize.y, width/viewportSize.x, height/viewportSize.y));
            basicShaders["tex_level_quad"]->SetUniform("tex", TEX_BASE);
            basicShaders["tex_level_quad"]->SetUniform("level", z);
        }
        
        OpenGLState::BindTexture(TEX_BASE, array ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_3D, texture);
        OpenGLState::BindVertexArray(baseVertexArray);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, quadBuf); 
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDisableVertexAttribArray(0);
        OpenGLState::BindVertexArray(0);
        OpenGLState::UnbindTexture(TEX_BASE);
        OpenGLState::UseProgram(0);
    }
}

void OpenGLContent::DrawCubemapCross(GLuint texture)
{
    if(cubeBuf != 0 && basicShaders["tex_cube"] != NULL)
    {
        basicShaders["tex_cube"]->Use();
        basicShaders["tex_cube"]->SetUniform("tex", TEX_BASE);
        
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_CUBE_MAP, texture);
        OpenGLState::BindVertexArray(baseVertexArray);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        
        glBindBuffer(GL_ARRAY_BUFFER, cubeBuf); 
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 16);
        glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);
        
        OpenGLState::BindVertexArray(0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        OpenGLState::UnbindTexture(TEX_BASE);
        OpenGLState::UseProgram(0);
    }
}

void OpenGLContent::DrawCoordSystem(glm::mat4 M, GLfloat size)
{
    if(csBuf[0] != 0 && basicShaders["helper"] != NULL)
    {
        basicShaders["helper"]->Use();
        basicShaders["helper"]->SetUniform("MVP", viewProjection*M);
        basicShaders["helper"]->SetUniform("scale", glm::vec3(size));
        
        OpenGLState::BindVertexArray(baseVertexArray);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        
        glBindBuffer(GL_ARRAY_BUFFER, csBuf[0]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, csBuf[1]);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glDrawArrays(GL_LINES, 0, 6);
        OpenGLState::BindVertexArray(0);
        
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        OpenGLState::UseProgram(0);
    }
}

void OpenGLContent::DrawCylinder(glm::mat4 M, glm::vec3 dims, glm::vec4 color)
{
    if(basicShaders["helper"] != NULL && cylinder.vao != 0)
    {
        basicShaders["helper"]->Use();
        basicShaders["helper"]->SetUniform("MVP", viewProjection*M);
        basicShaders["helper"]->SetUniform("scale", dims);
        
        OpenGLState::BindVertexArray(cylinder.vao);
        glVertexAttrib4fv(1, &color.r);
        glDrawElements(GL_TRIANGLES, 3 * cylinder.faceCount, GL_UNSIGNED_INT, 0);
        OpenGLState::BindVertexArray(0);
        OpenGLState::UseProgram(0);
    }
}

void OpenGLContent::DrawEllipsoid(glm::mat4 M, glm::vec3 radii, glm::vec4 color)
{
    if(basicShaders["helper"] != NULL && ellipsoid.vao != 0)
    {
        basicShaders["helper"]->Use();
        basicShaders["helper"]->SetUniform("MVP", viewProjection*M);
        basicShaders["helper"]->SetUniform("scale", radii);
        
        OpenGLState::BindVertexArray(ellipsoid.vao);
        glVertexAttrib4fv(1, &color.r);
        glDrawElements(GL_TRIANGLES, 3 * ellipsoid.faceCount, GL_UNSIGNED_INT, 0);
        OpenGLState::BindVertexArray(0);
        OpenGLState::UseProgram(0);
    }
}

void OpenGLContent::DrawPrimitives(PrimitiveType type, std::vector<glm::vec3>& vertices, glm::vec4 color, glm::mat4 M)
{
    if(basicShaders["helper"] != NULL && vertices.size() > 0)
    {
        GLuint vbo;
        glGenBuffers(1, &vbo);
        
        basicShaders["helper"]->Use();
        basicShaders["helper"]->SetUniform("MVP", viewProjection*M);
        basicShaders["helper"]->SetUniform("scale", glm::vec3(1.f));
        
        OpenGLState::BindVertexArray(baseVertexArray);
        glEnableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vertices.size(), &vertices[0].x, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glVertexAttrib4fv(1, &color.r);
        
        switch(type)
        {
            case LINES:
                glDrawArrays(GL_LINES, 0, (GLsizei)vertices.size());
                break;
            
            case LINE_STRIP:
                glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)vertices.size());
                break;
                
            case POINTS:
            default:
                glDrawArrays(GL_POINTS, 0, (GLsizei)vertices.size());
                break;
        }
        OpenGLState::BindVertexArray(0);
        glDisableVertexAttribArray(0);
        OpenGLState::UseProgram(0);
        
        glDeleteBuffers(1, &vbo);
    }
}

void OpenGLContent::DrawObject(int objectId, int lookId, const glm::mat4& M)
{
    if(objectId >= 0 && objectId < (int)objects.size()) //Check if object exists
    {
        switch(mode)
        {
            case DrawingMode::RAW:
            {
                OpenGLState::BindVertexArray(objects[objectId].vao);
                glDrawElements(GL_TRIANGLES, sizeof(Face) * objects[objectId].faceCount, GL_UNSIGNED_INT, 0);
                OpenGLState::BindVertexArray(0);
            }
            break;

            case DrawingMode::SHADOW:
            {
                basicShaders["shadow"]->Use();
                basicShaders["shadow"]->SetUniform("MVP", viewProjection*M);
                OpenGLState::BindVertexArray(objects[objectId].vao);
                glDrawElements(GL_TRIANGLES, sizeof(Face) * objects[objectId].faceCount, GL_UNSIGNED_INT, 0);
                OpenGLState::BindVertexArray(0);
            }
            break;
            
            case DrawingMode::FLAT:
            {
                basicShaders["flat"]->Use();
                basicShaders["flat"]->SetUniform("MVP", viewProjection*M);
                basicShaders["flat"]->SetUniform("FC", FC);
                OpenGLState::BindVertexArray(objects[objectId].vao);
                glDrawElements(GL_TRIANGLES, sizeof(Face) * objects[objectId].faceCount, GL_UNSIGNED_INT, 0);
                OpenGLState::BindVertexArray(0);
            }
            break;

            default:
            {
                if(lookId >= 0 && lookId < (int)looks.size())
                    UseLook(lookId, M);
                else
                    UseStandardLook(M);
        
                OpenGLState::BindVertexArray(objects[objectId].vao);
                glDrawElements(GL_TRIANGLES, sizeof(Face) * objects[objectId].faceCount, GL_UNSIGNED_INT, 0);
                OpenGLState::BindVertexArray(0);
            }
            break;
        }
    }
}

void OpenGLContent::DrawLightSource(unsigned int lightId)
{
    if(lightId >= 0 && lightId < lights.size())
    {
        int objectId = lights[lightId]->getSourceObject();
        
        if(objectId >= 0 && objectId < (int)objects.size())
        {
            //Render light source
            glm::vec4 colorLi = lights[lightId]->getColorLi();
            glm::mat4 M = lights[lightId]->getTransform();
            GLint type = lights[lightId]->getType() == LightType::POINT_LIGHT ? 0 : 1; 
            GLint id = lights[lightId]->getType() == LightType::POINT_LIGHT ? lightId : lightId - lightsUBOData.numPointLights;

            GLSLShader* shader = mode == DrawingMode::FULL ? lightSourceShader[0] : lightSourceShader[1];
            shader->Use();
            shader->SetUniform("MVP", viewProjection * M);
            shader->SetUniform("M", M);
            shader->SetUniform("N", glm::mat3(glm::transpose(glm::inverse(M))));
            shader->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view*M))));
            shader->SetUniform("FC", FC);
            shader->SetUniform("eyePos", eyePos);
            shader->SetUniform("viewDir", viewDir);
            shader->SetUniform("color", glm::vec3(colorLi) * colorLi.a);
            shader->SetUniform("lightId", glm::ivec2(type, id));
            
            if(mode == DrawingMode::UNDERWATER)
            {
                Ocean* ocean = SimulationApp::getApp()->getSimulationManager()->getOcean();
                shader->SetUniform("cWater", ocean->getOpenGLOcean()->getLightAttenuation());
                shader->SetUniform("bWater", ocean->getOpenGLOcean()->getLightScattering());
            }

            OpenGLState::BindVertexArray(objects[objectId].vao);
            glDrawElements(GL_TRIANGLES, sizeof(Face) * objects[objectId].faceCount, GL_UNSIGNED_INT, 0);
            OpenGLState::BindVertexArray(0);
        }
    }
}

void OpenGLContent::SetupLights()
{
    int pointId = 0;
    int spotId = 0;
    
    for(size_t i=0; i<lights.size(); ++i)
    {
        if(lights[i]->getType() == POINT_LIGHT)
        {
            lights[i]->SetupShader(&lightsUBOData.pointLights[pointId]);
            ++pointId;
        }
        else
        {
            lights[i]->SetupShader(&lightsUBOData.spotLights[spotId]);
            ++spotId;
        }
    }
    
    glBindBuffer(GL_UNIFORM_BUFFER, lightsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightsUBO), &lightsUBOData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLContent::UseLook(unsigned int lookId, const glm::mat4& M)
{	
    Look& l = looks[lookId];
    GLSLShader* shader;
    bool updateMaterial = (int)lookId != currentLookId;
    currentLookId = (int)lookId;

    bool waves = false;
    Ocean* ocean = SimulationApp::getApp()->getSimulationManager()->getOcean();
    if(ocean != NULL && ocean->hasWaves()) waves = true;

    switch(l.type)
    {		
        default:
        case SIMPLE: //Blinn-Phong
        {
            shader = mode == DrawingMode::FULL ? materialShaders[0] : (waves ? materialShaders[4] : materialShaders[2]);
            shader->Use();
            shader->SetUniform("MVP", viewProjection*M);
            shader->SetUniform("M", M);
            shader->SetUniform("N", glm::mat3(glm::transpose(glm::inverse(M))));
            shader->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view*M))));
            shader->SetUniform("FC", FC);
            shader->SetUniform("eyePos", eyePos);
            shader->SetUniform("viewDir", viewDir);

            if(updateMaterial)
            {
                shader->SetUniform("specularStrength", l.params[0]);
                shader->SetUniform("shininess", l.params[1]);
                shader->SetUniform("reflectivity", l.reflectivity);
               
                if(l.textures.size() > 0)
                {
                    shader->SetUniform("color", glm::vec4(l.color, 1.f));
                    OpenGLState::BindTexture(TEX_MAT_DIFFUSE, GL_TEXTURE_2D, l.textures[0]);
                }
                else
                {
                    shader->SetUniform("color", glm::vec4(l.color, 0.f));			
                    OpenGLState::UnbindTexture(TEX_MAT_DIFFUSE);
                }
            }
        }
            break;
            
        case PHYSICAL: //Cook-Torrance
        {
            shader = mode == DrawingMode::FULL ? materialShaders[1] : (waves ? materialShaders[5] : materialShaders[3]);
            shader->Use();
            shader->SetUniform("MVP", viewProjection*M);
            shader->SetUniform("M", M);
            shader->SetUniform("N", glm::mat3(glm::transpose(glm::inverse(M))));
            shader->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view*M))));
            shader->SetUniform("FC", FC);
            shader->SetUniform("eyePos", eyePos);
            shader->SetUniform("viewDir", viewDir);

            if(updateMaterial)
            {
                shader->SetUniform("roughness", l.params[0]);
                shader->SetUniform("metallic", l.params[1]);
                shader->SetUniform("reflectivity", l.reflectivity);
            
                if(l.textures.size() > 0)
                {
                    shader->SetUniform("color", glm::vec4(l.color, 1.f));
                    OpenGLState::BindTexture(TEX_MAT_DIFFUSE, GL_TEXTURE_2D, l.textures[0]);
                }
                else
                {
                    shader->SetUniform("color", glm::vec4(l.color, 0.f));			
                    OpenGLState::UnbindTexture(TEX_MAT_DIFFUSE);
                }
            }
        }
            break;
    }
    
    if(mode == DrawingMode::UNDERWATER)
    {
        shader->SetUniform("cWater", ocean->getOpenGLOcean()->getLightAttenuation());
        shader->SetUniform("bWater", ocean->getOpenGLOcean()->getLightScattering());
        if(waves)
        {
            OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, ocean->getOpenGLOcean()->getWaveTexture());
            shader->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
            shader->SetUniform("gridSizes", ocean->getOpenGLOcean()->getWaveGridSizes());
        }
    }
}

void OpenGLContent::UseStandardLook(const glm::mat4& M)
{
    bool updateMaterial = currentLookId >= 0;
    currentLookId = -1;

    GLSLShader* shader = mode == DrawingMode::FULL ? materialShaders[1] : materialShaders[3];
    shader->Use();
    shader->SetUniform("MVP", viewProjection*M);
    shader->SetUniform("M", M);
    shader->SetUniform("N", glm::mat3(glm::transpose(glm::inverse(M))));
    shader->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view*M))));
    shader->SetUniform("FC", FC);
    shader->SetUniform("eyePos", eyePos);
    shader->SetUniform("viewDir", viewDir);

    if(updateMaterial)
    {
        shader->SetUniform("roughness", 0.5f);
        shader->SetUniform("metallic", 0.f);
        shader->SetUniform("reflectivity", 0.f);
        shader->SetUniform("color", glm::vec4(0.5f, 0.5f, 0.5f, 0.f));
        OpenGLState::UnbindTexture(TEX_MAT_DIFFUSE);
    }
}

unsigned int OpenGLContent::BuildObject(Mesh* mesh)
{
    Object obj;
    
    glGenVertexArrays(1, &obj.vao);
    glGenBuffers(1, &obj.vboVertex);
    glGenBuffers(1, &obj.vboIndex);
    obj.faceCount = (GLsizei)mesh->faces.size();
    
    OpenGLState::BindVertexArray(obj.vao);	
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ARRAY_BUFFER, obj.vboVertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*mesh->vertices.size(), &mesh->vertices[0].pos.x, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)sizeof(glm::vec3));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)*2));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.vboIndex);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Face) * mesh->faces.size(), &mesh->faces[0].vertexID[0], GL_STATIC_DRAW);
    OpenGLState::BindVertexArray(0);
    
    objects.push_back(obj);
    return (unsigned int)objects.size()-1;
}

std::string OpenGLContent::CreateSimpleLook(std::string name, glm::vec3 rgbColor, GLfloat specular, GLfloat shininess, GLfloat reflectivity, std::string textureName)
{
    Look look;
    look.name = lookNameManager.AddName(name);
    look.type = LookType::SIMPLE;
    look.color = rgbColor;
    look.reflectivity = reflectivity;
    look.params.push_back(specular);
    look.params.push_back(shininess);
    
    if(textureName != "") 
        look.textures.push_back(LoadTexture(textureName));
    
    looks.push_back(look);
    
    return look.name;
}

std::string OpenGLContent::CreatePhysicalLook(std::string name, glm::vec3 rgbColor, GLfloat roughness, GLfloat metalness, GLfloat reflectivity, std::string textureName)
{
    Look look;
    look.name = lookNameManager.AddName(name);
    look.type = LookType::PHYSICAL;
    look.color = rgbColor;
    look.reflectivity = reflectivity;
    look.params.push_back(roughness);
    look.params.push_back(metalness);
    
    if(textureName != "")
        look.textures.push_back(LoadTexture(textureName, false, maxAnisotropy));
        
    looks.push_back(look);
    
    return look.name;
}

void OpenGLContent::AddView(OpenGLView *view)
{
    views.push_back(view);
}

OpenGLView* OpenGLContent::getView(unsigned int id)
{
    if(id < views.size())
        return views[id];
    else
        return NULL;
}

unsigned int OpenGLContent::getViewsCount()
{
    return (unsigned int)views.size();
}

void OpenGLContent::AddLight(OpenGLLight* light)
{
    lights.push_back(light);

    std::sort(lights.begin(), lights.end());
    lightsUBOData.numPointLights = 0;
    lightsUBOData.numSpotLights = 0;

    for(size_t i=0; i<lights.size(); ++i)
    {
        switch(lights[i]->getType())
        {
            case LightType::POINT_LIGHT:
                ++lightsUBOData.numPointLights;
                break;

            case LightType::SPOT_LIGHT:
                ++lightsUBOData.numSpotLights;
                break;
        }
    }
}

OpenGLLight* OpenGLContent::getLight(unsigned int id)
{
    if(id < lights.size())
        return lights[id];
    else
        return NULL;
}

unsigned int OpenGLContent::getLightsCount()
{
    return (unsigned int)lights.size();
}

int OpenGLContent::getLookId(std::string name)
{
    for(size_t i=0; i<looks.size(); ++i)
        if(looks[i].name == name)
            return (int)i;
            
    return -1;
}
    
//Static methods
GLuint OpenGLContent::LoadTexture(std::string filename, bool hasAlphaChannel, GLfloat anisotropy)
{
    int width, height, channels;
    int reqChannels = hasAlphaChannel ? 4 : 3;
    GLuint texture;
    
    // Allocate image; fail out on error
    stbi_set_flip_vertically_on_load(true);
    unsigned char* dataBuffer = stbi_load(filename.c_str(), &width, &height, &channels, reqChannels);
    if(dataBuffer == NULL)
    {
        cError("Failed to load texture from: %s", filename.c_str());
        return 0;
    }
    
    if(channels != reqChannels)
    {
        cWarning("Texture has %d channels while expected %d channels!", channels, reqChannels);
    }

    cInfo("Loaded texture from: %s", filename.c_str());
    
    glGenTextures(1, &texture);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, reqChannels == 3 ? GL_RGB8 : GL_RGBA8, width, height, 0, reqChannels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, dataBuffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    if(anisotropy > 0.f)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    OpenGLState::UnbindTexture(TEX_BASE);
    
    stbi_image_free(dataBuffer);
    
    return texture;
}

GLuint OpenGLContent::LoadInternalTexture(std::string filename, bool hasAlphaChannel, GLfloat anisotropy)
{
    return LoadTexture(GetShaderPath() + filename, hasAlphaChannel, anisotropy);
}

GLuint OpenGLContent::GenerateTexture(GLenum target, glm::uvec3 dimensions, GLenum internalFormat, GLenum format, GLenum type, const void* data, 
                                      FilteringMode fm, bool repeat, bool anisotropy)
{
    GLuint texture = 0;
    glGenTextures(1, &texture);
    OpenGLState::BindTexture(TEX_BASE, target, texture);
    
    switch(target)
    {
        case GL_TEXTURE_1D:
            if(dimensions.x == 0)
            {
                OpenGLState::UnbindTexture(TEX_BASE);
                glDeleteTextures(1, &texture);
                cError("Texture dimensions cannot be equal to 0!");
                return 0;
            }
            glTexImage1D(target, 0, internalFormat, dimensions.x, 0, format, type, data);
            break;

        case GL_TEXTURE_2D:
            if(dimensions.x == 0 || dimensions.y == 0)
            {
                OpenGLState::UnbindTexture(TEX_BASE);
                glDeleteTextures(1, &texture);
                cError("Texture dimensions cannot be equal to 0!");
                return 0;
            }
            glTexImage2D(target, 0, internalFormat, dimensions.x, dimensions.y, 0, format, type, data);
            break;

        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_3D:
            if(dimensions.x == 0 || dimensions.y == 0 || dimensions.z == 0)
            {
                OpenGLState::UnbindTexture(TEX_BASE);
                glDeleteTextures(1, &texture);
                cError("Texture dimensions cannot be equal to 0!");
                return 0;
            }
            glTexImage3D(target, 0, internalFormat, dimensions.x, dimensions.y, dimensions.z, 0, format, type, data);
            break;

        default:
            OpenGLState::UnbindTexture(TEX_BASE);
            glDeleteTextures(1, &texture);
            cError("Unsupported texture format requested!");
            return 0;
    }

    GLenum repeatMode = repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    glTexParameteri(target, GL_TEXTURE_WRAP_S, repeatMode);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, repeatMode);
    if(target == GL_TEXTURE_3D)
        glTexParameteri(target, GL_TEXTURE_WRAP_R, repeatMode);

    if(anisotropy)
        glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY, OpenGLState::GetMaxAnisotropy());

    switch(fm)
    {
        case FilteringMode::NEAREST:
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;

        case FilteringMode::BILINEAR:
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;

        case FilteringMode::BILINEAR_MIPMAP:
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glGenerateMipmap(target);
            break;

        case FilteringMode::TRILINEAR:
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glGenerateMipmap(target);
            break;
    }

    OpenGLState::UnbindTexture(TEX_BASE);
    return texture;
}

Mesh* OpenGLContent::BuildPlane(GLfloat halfExtents)
{
    Mesh* mesh = new Mesh;
    mesh->hasUVs = true;
    Face f;
    Vertex vt;
    
    //Only one normal
    vt.pos.z = 0;
    vt.normal = glm::vec3(0,0,-1.f);
    
    vt.pos.x = -halfExtents;
    vt.pos.y = -halfExtents;
    vt.uv = glm::vec2(0.f,0.f);
    mesh->vertices.push_back(vt);
    
    vt.pos.x = halfExtents;
    vt.uv = glm::vec2(0.f, halfExtents*2.f);
    mesh->vertices.push_back(vt);
        
    vt.pos.y = halfExtents;
    vt.uv = glm::vec2(halfExtents*2.f, halfExtents*2.f);
    mesh->vertices.push_back(vt);
        
    vt.pos.x = -halfExtents;
    vt.uv = glm::vec2(halfExtents*2.f, 0.f);
    mesh->vertices.push_back(vt);
    
    f.vertexID[0] = 0;
    f.vertexID[1] = 2;
    f.vertexID[2] = 1;
    mesh->faces.push_back(f);
    f.vertexID[1] = 3;
    f.vertexID[2] = 2;
    mesh->faces.push_back(f);
    return mesh;
}

Mesh* OpenGLContent::BuildBox(glm::vec3 halfExtents, unsigned int subdivisions, unsigned int uvMode)
{
    //Build mesh
    Mesh* mesh = new Mesh();
    Face f;
    Vertex vt;
    mesh->hasUVs = true;
    
    /////VERTICES
    glm::vec3 v1(-halfExtents.x, -halfExtents.y, -halfExtents.z);
    glm::vec3 v2(-halfExtents.x, halfExtents.y, -halfExtents.z);
    glm::vec3 v3(halfExtents.x, halfExtents.y, -halfExtents.z);
    glm::vec3 v4(halfExtents.x, -halfExtents.y, -halfExtents.z);
    glm::vec3 v5(halfExtents.x, halfExtents.y, halfExtents.z);
    glm::vec3 v6(halfExtents.x, -halfExtents.y, halfExtents.z);
    glm::vec3 v7(-halfExtents.x, -halfExtents.y, halfExtents.z);
    glm::vec3 v8(-halfExtents.x, halfExtents.y, halfExtents.z);
    
    /////TOP
    //normal
    vt.normal = glm::vec3(0,0,-1.f);
    //vertices
    vt.pos = v1;
    mesh->vertices.push_back(vt); //0
    vt.pos = v2;
    mesh->vertices.push_back(vt); //1
    vt.pos = v3;
    mesh->vertices.push_back(vt); //2
    vt.pos = v4;
    mesh->vertices.push_back(vt); //3
    //faces
    f.vertexID[0] = 0;
    f.vertexID[1] = 1;
    f.vertexID[2] = 2;
    mesh->faces.push_back(f);
    f.vertexID[1] = 2;
    f.vertexID[2] = 3;
    mesh->faces.push_back(f);
    
    /////FRONT
    //normal
    vt.normal =  glm::vec3(1.f,0,0);
    //vertices
    vt.pos = v4;
    mesh->vertices.push_back(vt); //4
    vt.pos = v3;
    mesh->vertices.push_back(vt); //5
    vt.pos = v5;
    mesh->vertices.push_back(vt); //6
    vt.pos = v6;
    mesh->vertices.push_back(vt); //7
    //faces
    f.vertexID[0] = 4;
    f.vertexID[1] = 5;
    f.vertexID[2] = 6;
    mesh->faces.push_back(f);
    f.vertexID[1] = 6;
    f.vertexID[2] = 7;
    mesh->faces.push_back(f);
    
    /////BACK
    //normal
    vt.normal = glm::vec3(-1.f,0,0);
    //vertices
    vt.pos = v7;
    mesh->vertices.push_back(vt); //8
    vt.pos = v8;
    mesh->vertices.push_back(vt); //9
    vt.pos = v2;
    mesh->vertices.push_back(vt); //10
    vt.pos = v1;
    mesh->vertices.push_back(vt); //11
    //faces
    f.vertexID[0] = 8;
    f.vertexID[1] = 9;
    f.vertexID[2] = 10;
    mesh->faces.push_back(f);
    f.vertexID[1] = 10;
    f.vertexID[2] = 11;
    mesh->faces.push_back(f);
    
    /////BOTTOM
    //normal
    vt.normal = glm::vec3(0,0,1.f);
    //vertices
    vt.pos = v6;
    mesh->vertices.push_back(vt); //12
    vt.pos = v5;
    mesh->vertices.push_back(vt); //13
    vt.pos = v8;
    mesh->vertices.push_back(vt); //14
    vt.pos = v7;
    mesh->vertices.push_back(vt); //15
    //faces
    f.vertexID[0] = 12;
    f.vertexID[1] = 13;
    f.vertexID[2] = 14;
    mesh->faces.push_back(f);
    f.vertexID[1] = 14;
    f.vertexID[2] = 15;
    mesh->faces.push_back(f);
    
    //////LEFT
    //normal
    vt.normal = glm::vec3(0,1.f,0);
    //vertices
    vt.pos = v5;
    mesh->vertices.push_back(vt); //16
    vt.pos = v3;
    mesh->vertices.push_back(vt); //17
    vt.pos = v2;
    mesh->vertices.push_back(vt); //18
    vt.pos = v8;
    mesh->vertices.push_back(vt); //19
    //faces
    f.vertexID[0] = 16;
    f.vertexID[1] = 17;
    f.vertexID[2] = 18;
    mesh->faces.push_back(f);
    f.vertexID[1] = 18;
    f.vertexID[2] = 19;
    mesh->faces.push_back(f);
    
    /////RIGHT
    //normal
    vt.normal = glm::vec3(0,-1.f,0);
    //vertices
    vt.pos = v4;
    mesh->vertices.push_back(vt); //20
    vt.pos = v6;
    mesh->vertices.push_back(vt); //21
    vt.pos = v7;
    mesh->vertices.push_back(vt); //22
    vt.pos = v1;
    mesh->vertices.push_back(vt); //23
    //faces
    f.vertexID[0] = 20;
    f.vertexID[1] = 21;
    f.vertexID[2] = 22;
    mesh->faces.push_back(f);
    f.vertexID[1] = 22;
    f.vertexID[2] = 23;
    mesh->faces.push_back(f);
    
    //Texture coordinates
    switch(uvMode)
    {
        default:
        case 0:
            mesh->vertices[0].uv = glm::vec2(2.f/3.f, 1.f);
            mesh->vertices[1].uv = glm::vec2(1.f/3.f, 1.f);
            mesh->vertices[2].uv = glm::vec2(1.f/3.f, 2.f/3.f);
            mesh->vertices[3].uv = glm::vec2(2.f/3.f, 2.f/3.f);
            
            mesh->vertices[4].uv = glm::vec2(2.f/3.f, 2.f/3.f);
            mesh->vertices[5].uv = glm::vec2(1.f/3.f, 2.f/3.f);
            mesh->vertices[6].uv = glm::vec2(1.f/3.f, 1.f/3.f);
            mesh->vertices[7].uv = glm::vec2(2.f/3.f, 1.f/3.f);
            
            mesh->vertices[8].uv = glm::vec2(0.f, 2.f/3.f);
            mesh->vertices[9].uv = glm::vec2(1.f/3.f, 2.f/3.f);
            mesh->vertices[10].uv = glm::vec2(1.f/3.f, 1.f);
            mesh->vertices[11].uv = glm::vec2(0.f, 1.f);
            
            mesh->vertices[12].uv = glm::vec2(2.f/3.f, 1.f/3.f);
            mesh->vertices[13].uv = glm::vec2(1.f/3.f, 1.f/3.f);
            mesh->vertices[14].uv = glm::vec2(1.f/3.f, 0.f);
            mesh->vertices[15].uv = glm::vec2(2.f/3.f, 0.f);
            
            mesh->vertices[16].uv = glm::vec2(1.f/3.f, 1.f/3.f);
            mesh->vertices[17].uv = glm::vec2(1.f/3.f, 2.f/3.f);
            mesh->vertices[18].uv = glm::vec2(0.f, 2.f/3.f);
            mesh->vertices[19].uv = glm::vec2(0.f, 1.f/3.f);
            
            mesh->vertices[20].uv = glm::vec2(2.f/3.f, 2.f/3.f);
            mesh->vertices[21].uv = glm::vec2(2.f/3.f, 1.f/3.f);
            mesh->vertices[22].uv = glm::vec2(1.f, 1.f/3.f);
            mesh->vertices[23].uv = glm::vec2(1.f, 2.f/3.f);
            break;
            
        case 1:
            mesh->vertices[0].uv = glm::vec2(1.f, 1.f);
            mesh->vertices[1].uv = glm::vec2(0.f, 1.f);
            mesh->vertices[2].uv = glm::vec2(0.f, 0.f);
            mesh->vertices[3].uv = glm::vec2(1.f, 0.f);
            
            mesh->vertices[4].uv = glm::vec2(1.f, 1.f);
            mesh->vertices[5].uv = glm::vec2(0.f, 1.f);
            mesh->vertices[6].uv = glm::vec2(0.f, 0.f);
            mesh->vertices[7].uv = glm::vec2(1.f, 0.f);
            
            mesh->vertices[8].uv = glm::vec2(0.f, 0.f);
            mesh->vertices[9].uv = glm::vec2(1.f, 0.f);
            mesh->vertices[10].uv = glm::vec2(1.f, 1.f);
            mesh->vertices[11].uv = glm::vec2(0.f, 1.f);
            
            mesh->vertices[12].uv = glm::vec2(1.f, 1.f);
            mesh->vertices[13].uv = glm::vec2(0.f, 1.f);
            mesh->vertices[14].uv = glm::vec2(0.f, 0.f);
            mesh->vertices[15].uv = glm::vec2(1.f, 0.f);
            
            mesh->vertices[16].uv = glm::vec2(1.f, 0.f);
            mesh->vertices[17].uv = glm::vec2(1.f, 1.f);
            mesh->vertices[18].uv = glm::vec2(0.f, 1.f);
            mesh->vertices[19].uv = glm::vec2(0.f, 0.f);
            
            mesh->vertices[20].uv = glm::vec2(0.f, 1.f);
            mesh->vertices[21].uv = glm::vec2(0.f, 0.f);
            mesh->vertices[22].uv = glm::vec2(1.f, 0.f);
            mesh->vertices[23].uv = glm::vec2(1.f, 1.f);
            break;
            
        case 2:
            mesh->vertices[0].uv = glm::vec2(halfExtents.y*2.f, halfExtents.x*2.f);
            mesh->vertices[1].uv = glm::vec2(0.f, halfExtents.x*2.f);
            mesh->vertices[2].uv = glm::vec2(0.f, 0.f);
            mesh->vertices[3].uv = glm::vec2(halfExtents.y*2.f, 0.f);
            
            mesh->vertices[4].uv = glm::vec2(halfExtents.y*2.f, halfExtents.z*2.f);
            mesh->vertices[5].uv = glm::vec2(0.f, halfExtents.z*2.f);
            mesh->vertices[6].uv = glm::vec2(0.f, 0.f);
            mesh->vertices[7].uv = glm::vec2(halfExtents.y*2.f, 0.f);
            
            mesh->vertices[8].uv = glm::vec2(0.f, 0.f);
            mesh->vertices[9].uv = glm::vec2(halfExtents.y*2.f, 0.f);
            mesh->vertices[10].uv = glm::vec2(halfExtents.y*2.f, halfExtents.z*2.f);
            mesh->vertices[11].uv = glm::vec2(0.f, halfExtents.z*2.f);
            
            mesh->vertices[12].uv = glm::vec2(halfExtents.y*2.f, halfExtents.x*2.f);
            mesh->vertices[13].uv = glm::vec2(0.f, halfExtents.x*2.f);
            mesh->vertices[14].uv = glm::vec2(0.f, 0.f);
            mesh->vertices[15].uv = glm::vec2(halfExtents.y*2.f, 0.f);
            
            mesh->vertices[16].uv = glm::vec2(halfExtents.x*2.f, 0.f);
            mesh->vertices[17].uv = glm::vec2(halfExtents.x*2.f, halfExtents.z*2.f);
            mesh->vertices[18].uv = glm::vec2(0.f, halfExtents.z*2.f);
            mesh->vertices[19].uv = glm::vec2(0.f, 0.f);
            
            mesh->vertices[20].uv = glm::vec2(0.f, halfExtents.z*2.f);
            mesh->vertices[21].uv = glm::vec2(0.f, 0.f);
            mesh->vertices[22].uv = glm::vec2(halfExtents.x*2.f, 0.f);
            mesh->vertices[23].uv = glm::vec2(halfExtents.x*2.f, halfExtents.z*2.f);
            break;
    }
    
    //Subdivide
    for(unsigned int i=0; i<subdivisions; ++i)
        Subdivide(mesh);
        
    //SmoothNormals(mesh);
    
    return mesh;
}

Mesh* OpenGLContent::BuildSphere(GLfloat radius, unsigned int subdivisions) 
{
    Mesh* mesh = new Mesh;
    Face f;
    Vertex vt;
    vt.normal = glm::vec3(1,0,0);
    mesh->hasUVs = false;
    vt.uv = glm::vec2(0,0);
    
    //Basuc icosahedron
    const GLfloat X = -.525731112119133606f;
    const GLfloat Z = -.850650808352039932f;
    const GLfloat N = 0.f;
    
    vt.pos = glm::vec3(-X,N,Z);
    vt.normal = vt.pos;
    mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(X,N,Z);
    vt.normal = vt.pos;
    mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(-X,N,-Z);
    vt.normal = vt.pos;
    mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(X,N,-Z);
    vt.normal = vt.pos;
    mesh->vertices.push_back(vt);
    
    vt.pos = glm::vec3(N,Z,X);
    vt.normal = vt.pos;
    mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(N,Z,-X);
    vt.normal = vt.pos;
    mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(N,-Z,X);
    vt.normal = vt.pos;
    mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(N,-Z,-X);
    vt.normal = vt.pos;
    mesh->vertices.push_back(vt);
    
    vt.pos = glm::vec3(Z,X,N);
    vt.normal = vt.pos;
    mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(-Z,X,N);
    vt.normal = vt.pos;
    mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(Z,-X,N);
    vt.normal = vt.pos;
    mesh->vertices.push_back(vt);
    vt.pos = glm::vec3(-Z,-X,N);
    vt.normal = vt.pos;
    mesh->vertices.push_back(vt);
    
    f.vertexID[0] = 0;
    f.vertexID[1] = 4;
    f.vertexID[2] = 1;
    mesh->faces.push_back(f);
    f.vertexID[0] = 0;
    f.vertexID[1] = 9;
    f.vertexID[2] = 4;
    mesh->faces.push_back(f);
    f.vertexID[0] = 9;
    f.vertexID[1] = 5;
    f.vertexID[2] = 4;
    mesh->faces.push_back(f);
    f.vertexID[0] = 4;
    f.vertexID[1] = 5;
    f.vertexID[2] = 8;
    mesh->faces.push_back(f);
    f.vertexID[0] = 4;
    f.vertexID[1] = 8;
    f.vertexID[2] = 1;
    mesh->faces.push_back(f);
    f.vertexID[0] = 8;
    f.vertexID[1] = 10;
    f.vertexID[2] = 1;
    mesh->faces.push_back(f);
    f.vertexID[0] = 8;
    f.vertexID[1] = 3;
    f.vertexID[2] = 10;
    mesh->faces.push_back(f);
    f.vertexID[0] = 5;
    f.vertexID[1] = 3;
    f.vertexID[2] = 8;
    mesh->faces.push_back(f);
    f.vertexID[0] = 5;
    f.vertexID[1] = 2;
    f.vertexID[2] = 3;
    mesh->faces.push_back(f);
    f.vertexID[0] = 2;
    f.vertexID[1] = 7;
    f.vertexID[2] = 3;
    mesh->faces.push_back(f);
    f.vertexID[0] = 7;
    f.vertexID[1] = 10;
    f.vertexID[2] = 3;
    mesh->faces.push_back(f);
    f.vertexID[0] = 7;
    f.vertexID[1] = 6;
    f.vertexID[2] = 10;
    mesh->faces.push_back(f);
    f.vertexID[0] = 7;
    f.vertexID[1] = 11;
    f.vertexID[2] = 6;
    mesh->faces.push_back(f);
    f.vertexID[0] = 11;
    f.vertexID[1] = 0;
    f.vertexID[2] = 6;
    mesh->faces.push_back(f);
    f.vertexID[0] = 0;
    f.vertexID[1] = 1;
    f.vertexID[2] = 6;
    mesh->faces.push_back(f);
    f.vertexID[0] = 6;
    f.vertexID[1] = 1;
    f.vertexID[2] = 10;
    mesh->faces.push_back(f);
    f.vertexID[0] = 9;
    f.vertexID[1] = 0;
    f.vertexID[2] = 11;
    mesh->faces.push_back(f);
    f.vertexID[0] = 9;
    f.vertexID[1] = 11;
    f.vertexID[2] = 2;
    mesh->faces.push_back(f);
    f.vertexID[0] = 9;
    f.vertexID[1] = 2;
    f.vertexID[2] = 5;
    mesh->faces.push_back(f);
    f.vertexID[0] = 7;
    f.vertexID[1] = 2;
    f.vertexID[2] = 11;
    mesh->faces.push_back(f);
    
    //Subdivide to get smooth sphere
    for(unsigned int i=0; i<subdivisions; ++i)
        Subdivide(mesh, true);
        
    //Scale by radius
    for(unsigned int i=0; i<mesh->vertices.size(); ++i)
        mesh->vertices[i].pos *= radius;
    
    return mesh;
}

Mesh* OpenGLContent::BuildCylinder(GLfloat radius, GLfloat height, unsigned int slices)
{
    Mesh* mesh = new Mesh;
    GLfloat halfHeight = height/2.f;
    Vertex vt;
    Face f;
    mesh->hasUVs = true;
    
    //SIDE
    //Side vertices
    for(unsigned int i=0; i<=slices; ++i)
    {
        vt.normal = glm::vec3(sinf(i/(GLfloat)slices*M_PI*2.f), -cosf(i/(GLfloat)slices*M_PI*2.f), 0.0);
        vt.pos = glm::vec3(sinf(i/(GLfloat)slices*M_PI*2.f)*radius, -cosf(i/(GLfloat)slices*M_PI*2.f)*radius, halfHeight);
        vt.uv = glm::vec2(1.f-i/(GLfloat)slices, 0.f);
        mesh->vertices.push_back(vt);
        vt.pos.z = -halfHeight;
        vt.uv = glm::vec2(1.f-i/(GLfloat)slices, 1.f);
        mesh->vertices.push_back(vt);
    }
    
    //Side faces
    for(unsigned int i=0; i<slices; ++i)
    {
        f.vertexID[0] = i*2;
        f.vertexID[1] = i*2+1;
        f.vertexID[2] = i*2+2;
        mesh->faces.push_back(f);
        f.vertexID[0] = i*2+1;
        f.vertexID[1] = i*2+3;
        f.vertexID[2] = i*2+2;
        mesh->faces.push_back(f);
    }
    
    //BOTTOM CAP
    vt.normal = glm::vec3(0,0,1.f);
    vt.pos = glm::vec3(0,0,halfHeight);
    vt.uv = glm::vec2(0.5f, 0.5f);
    mesh->vertices.push_back(vt);
    unsigned int centerIndex = (unsigned int)mesh->vertices.size()-1;
    //vertices
    for(unsigned int i=0; i<=slices; ++i)
    {
        vt.pos = glm::vec3(sinf(i/(GLfloat)slices*M_PI*2.f)*radius, -cosf(i/(GLfloat)slices*M_PI*2.f)*radius, halfHeight);
        vt.uv = glm::vec2(cosf(i/(GLfloat)slices*M_PI*2.f)/2.f+0.5f, sinf(i/(GLfloat)slices*M_PI*2.f)/2.f+0.5f);
        mesh->vertices.push_back(vt);
    }
    //faces
    for(unsigned int i=0; i<slices; ++i)
    {
        f.vertexID[0] = centerIndex;
        f.vertexID[1] = centerIndex + i + 1;
        f.vertexID[2] = centerIndex + i + 2;
        mesh->faces.push_back(f);
    }
    
    //TOP CAP
    vt.normal = glm::vec3(0,0,-1.f);
    vt.pos = glm::vec3(0,0,-halfHeight);
    vt.uv = glm::vec2(0.5f, 0.5f);
    mesh->vertices.push_back(vt);
    centerIndex = (unsigned int)mesh->vertices.size()-1;
    //vertices
    for(unsigned int i=0; i<=slices; ++i)
    {
        vt.pos = glm::vec3(sinf(i/(GLfloat)slices*M_PI*2.f)*radius, -cosf(i/(GLfloat)slices*M_PI*2.f)*radius, -halfHeight);
        vt.uv = glm::vec2(-cosf(i/(GLfloat)slices*M_PI*2.f)/2.f+0.5f, sinf(i/(GLfloat)slices*M_PI*2.f)/2.f+0.5f);
        mesh->vertices.push_back(vt);
    }
    
    //faces
    for(unsigned int i=0; i<slices; ++i)
    {
        f.vertexID[0] = centerIndex;
        f.vertexID[1] = centerIndex+ i + 2;
        f.vertexID[2] = centerIndex+ i + 1;
        mesh->faces.push_back(f);
    }
    
    Subdivide(mesh);
    Subdivide(mesh);
    return mesh;
}

Mesh* OpenGLContent::BuildTorus(GLfloat majorRadius, GLfloat minorRadius, unsigned int majorSlices, unsigned int minorSlices)
{
    Mesh* mesh = new Mesh;
    mesh->hasUVs = true;
    Face f;
    Vertex vt;

    //Vertices
    for(unsigned int i=0; i<=majorSlices; ++i)
    {
        GLfloat alpha = i/(GLfloat)majorSlices*M_PI*2.f;
        
        for(unsigned int h=0; h<=minorSlices; ++h)
        {
            GLfloat ry = cosf(h/(GLfloat)minorSlices*M_PI*2.f) * minorRadius;
            GLfloat rx = sinf(h/(GLfloat)minorSlices*M_PI*2.f) * minorRadius;
            
            vt.pos = glm::vec3((rx + majorRadius)*cosf(alpha), ry, (rx + majorRadius)*sinf(alpha));
            vt.normal = glm::vec3(sinf(h/(GLfloat)minorSlices*M_PI*2.f)*cosf(alpha), cosf(h/(GLfloat)minorSlices*M_PI*2.f), sinf(h/(GLfloat)minorSlices*M_PI*2.f)*sinf(alpha));
            vt.uv = glm::vec2(i/(GLfloat)majorSlices, h/(GLfloat)minorSlices);
            mesh->vertices.push_back(vt);
            
        }
    }
    
    //Faces
    for(unsigned int i=0; i<majorSlices; ++i)
    {
        for(unsigned int h=0; h<minorSlices; ++h)
        {
            f.vertexID[0] = i*(minorSlices+1) + h;
            f.vertexID[1] = (i+1)*(minorSlices+1) + h;
            f.vertexID[2] = i*(minorSlices+1) + (h+1);
            mesh->faces.push_back(f);
            f.vertexID[0] = (i+1)*(minorSlices+1) + h;
            f.vertexID[1] = (i+1)*(minorSlices+1) + (h+1);
            f.vertexID[2] = i*(minorSlices+1) + (h+1);
            mesh->faces.push_back(f);
        }
    }
        
    return mesh;
}
    
Mesh* OpenGLContent::BuildWing(GLfloat baseChordLength, GLfloat tipChordLength, GLfloat maxCamber, GLfloat maxCamberPos, GLfloat profileThickness, GLfloat wingLength)
{
    Mesh* mesh = new Mesh;
    mesh->hasUVs = true;

    GLfloat T = profileThickness/100.f;
    GLfloat m = maxCamber/100.f;
    GLfloat p = maxCamberPos/100.f;
    GLfloat taper = tipChordLength/baseChordLength;
    GLfloat offset = baseChordLength/2.f;
    
    int div = 20;
    GLfloat xt[div+1];
    GLfloat yt[div+1];
    for(int i=0; i<=div; ++i)
    {
        GLfloat beta = (GLfloat)i/(GLfloat)div * M_PI;
        xt[i] = 0.5f * (1.f-cosf(beta));
        yt[i] = T/0.2f * (0.2969*sqrtf(xt[i])
                          -0.1260*xt[i]
                          -0.3516*xt[i]*xt[i]
                          +0.2843*xt[i]*xt[i]*xt[i]
                          -0.1036*xt[i]*xt[i]*xt[i]*xt[i]);
    }
    
    GLfloat xu[div+1];
    GLfloat xl[div+1];
    GLfloat yu[div+1];
    GLfloat yl[div+1];
    
    if(m == 0.f || p == 0.f) //No camber
    {
        for(int i=0; i<=div; ++i)
        {
            xu[i] = xl[i] = xt[i];
            yu[i] = yt[i];
            yl[i] = -yt[i];
        }
    }
    else //Camber
    {
        GLfloat yc[div+1];
        GLfloat dyc_dx[div+1];
        
        for(int i=0; i<=div; ++i)
        {
            if(xt[i] <= p)
            {
                yc[i] = (m/(p*p))*(2.f*p*xt[i]-xt[i]*xt[i]);
                dyc_dx[i] = (m/(p*p))*(2.f*p-2.f*xt[i]);
            }
            else //p < xt[i] < 1.0
            {
                yc[i] = m/((1.f-p)*(1.f-p))*(1.f-2.f*p+2.f*p*xt[i]-xt[i]*xt[i]);
                dyc_dx[i] = m/((1.f-p)*(1.f-p))*(2.f*p-2.f*xt[i]);
            }
            
            GLfloat theta = atan(dyc_dx[i]);
            xu[i] = xt[i] - yt[i] * sinf(theta);
            yu[i] = yc[i] + yt[i] * cosf(theta);
            xl[i] = xt[i] + yt[i] * sinf(theta);
            yl[i] = yc[i] - yt[i] * cosf(theta);
            
            cInfo("%1.3f\n", xu[i]);
        }
    }
    
    //Vertices
    Vertex v;
    
    //----Top side
    //Front base
    v.pos = glm::vec3(offset-xu[0]*baseChordLength,0.f,0.f);
    v.normal = glm::vec3(1.f,0.f,0.f);
    v.uv = glm::vec2(0.f,0.f);
    mesh->vertices.push_back(v);
    
    //Front tip
    v.pos = glm::vec3(offset*taper-xu[0]*tipChordLength,wingLength,0.f);
    v.uv = glm::vec2(0.f,1.f);
    mesh->vertices.push_back(v);
    
    //Middle
    v.normal = glm::vec3(0.f,0.f,-1.f);
    GLfloat dint = 0.f;
    
    for(int i=1; i<div; ++i)
    {
        //Calculate UV
        glm::vec2 dUV(xu[i]-xu[i-1], yu[i]-yu[i-1]);
        dint += dUV.length();
        
        //Base vertex
        v.pos = glm::vec3(offset-xu[i]*baseChordLength,0.f,-yu[i]*baseChordLength);
        v.uv = glm::vec2(dint, 0.f);
        mesh->vertices.push_back(v);
        
        //Tip vertex
        v.pos = glm::vec3(offset*taper-xu[i]*tipChordLength,wingLength,-yu[i]*tipChordLength);
        v.uv = glm::vec2(dint, 1.f);
        mesh->vertices.push_back(v);
    }
    
    glm::vec2 dUVe(1.f-xu[div-1], 0.f-yu[div-1]);
    dint += dUVe.length();
    
    //Normalize UVs
    for(int i=1; i<div; ++i)
    {
        mesh->vertices[i*2].normal.x /= dint*2.f;
        mesh->vertices[i*2+1].normal.x /= dint*2.f;
    }
    
    //Trailing edge base
    v.pos = glm::vec3(offset-xu[div]*baseChordLength,0.f,0.f);
    v.uv = glm::vec2(0.5f,0.f);
    mesh->vertices.push_back(v);
    
    //Trailing edge tip
    v.pos = glm::vec3(offset*taper-xu[div]*tipChordLength,wingLength,0.f);
    v.uv = glm::vec2(0.5f,1.f);
    mesh->vertices.push_back(v);
    
    //Faces
    for(int i=0; i<div; ++i)
    {
        Face f;
        f.vertexID[0] = i*2;
        f.vertexID[1] = i*2+2;
        f.vertexID[2] = i*2+1;
        mesh->faces.push_back(f);
        f.vertexID[0] = i*2+2;
        f.vertexID[1] = i*2+3;
        f.vertexID[2] = i*2+1;
        mesh->faces.push_back(f);
    }
    
    //----Bottom side
    //Front base
    v.pos = glm::vec3(offset-xl[0]*baseChordLength,0.f,0.f);
    v.normal = glm::vec3(1.f,0.f,0.f);
    v.uv = glm::vec2(0.5f,0.f);
    mesh->vertices.push_back(v);
    
    //Front tip
    v.pos = glm::vec3(offset*taper-xl[0]*tipChordLength,wingLength,0.f);
    v.uv = glm::vec2(0.5f,1.f);
    mesh->vertices.push_back(v);
    
    //Middle
    v.normal = glm::vec3(0.f,0.f,1.f);
    dint = 0.f;
    
    for(int i=1; i<div; ++i)
    {
        //Calculate UV
        glm::vec2 dUV(xl[i]-xl[i-1], yl[i]-yl[i-1]);
        dint += dUV.length();
        
        v.pos = glm::vec3(offset-xl[i]*baseChordLength,0.f,-yl[i]*baseChordLength);
        v.uv = glm::vec2(dint, 0.f);
        mesh->vertices.push_back(v);
        
        v.pos = glm::vec3(offset*taper-xl[i]*tipChordLength,wingLength,-yl[i]*tipChordLength);
        v.uv = glm::vec2(dint, 1.f);
        mesh->vertices.push_back(v);
    }
    
    dUVe = glm::vec2(1.f-xl[div-1], 0.f-yl[div-1]);
    dint += dUVe.length();
    
    //Normalize UVs
    for(int i=div+2; i<=2*div; ++i)
    {
        mesh->vertices[i*2].normal.x = mesh->vertices[i*2].normal.x/(dint*2.f) + 0.5f;
        mesh->vertices[i*2+1].normal.x = mesh->vertices[i*2+1].normal.x/(dint*2.f) + 0.5f;
    }
    
    //Trailing edge base
    v.pos = glm::vec3(offset-xl[div]*baseChordLength,0.f,0.f);
    v.uv = glm::vec2(1.f,0.f);
    mesh->vertices.push_back(v);
    
    //Trailing edge tip
    v.pos = glm::vec3(offset*taper-xl[div]*tipChordLength,wingLength,0.f);
    v.uv = glm::vec2(1.f,1.f);
    mesh->vertices.push_back(v);
    
    //Faces
    for(int i=div+1; i<=2*div; ++i)
    {
        Face f;
        f.vertexID[0] = i*2;
        f.vertexID[1] = i*2+1;
        f.vertexID[2] = i*2+2;
        mesh->faces.push_back(f);
        f.vertexID[0] = i*2+2;
        f.vertexID[1] = i*2+1;
        f.vertexID[2] = i*2+3;
        mesh->faces.push_back(f);
    }
    
    //Smooth the profile normals
    SmoothNormals(mesh);

    //Base cap
    v.normal = glm::vec3(0.f,-1.f,0.f);
    v.uv = glm::vec2(0.f,0.f); //No UVs for the caps
    
    //Vertices
    for(int i=0; i<div; ++i)
    {
        v.pos = glm::vec3(offset-xu[i]*baseChordLength,0.f,-yu[i]*baseChordLength);
        mesh->vertices.push_back(v);
        
        v.pos = glm::vec3(offset-xl[i]*baseChordLength,0.f,-yl[i]*baseChordLength);
        mesh->vertices.push_back(v);
    }
    
    v.pos = glm::vec3(offset-xu[div]*baseChordLength,0.f,0.f);
    mesh->vertices.push_back(v);
    
    v.pos = glm::vec3(offset-xl[div]*baseChordLength,0.f,0.f);
    mesh->vertices.push_back(v);
    
    //Faces
    for(int i=2*div+2; i<=3*div+1; ++i)
    {
        Face f;
        f.vertexID[0] = i*2;
        f.vertexID[1] = i*2+1;
        f.vertexID[2] = i*2+2;
        mesh->faces.push_back(f);
        f.vertexID[0] = i*2+2;
        f.vertexID[1] = i*2+1;
        f.vertexID[2] = i*2+3;
        mesh->faces.push_back(f);
    }
    
    //Tip cap
    v.normal = glm::vec3(0.f,1.f,0.f);
    v.uv = glm::vec2(0.f,0.f); //No UVs for the caps
    
    //Vertices
    for(int i=0; i<div; ++i)
    {
        v.pos = glm::vec3(offset*taper-xl[i]*tipChordLength,wingLength,-yl[i]*tipChordLength);
        mesh->vertices.push_back(v);
        
        v.pos = glm::vec3(offset*taper-xu[i]*tipChordLength,wingLength,-yu[i]*tipChordLength);
        mesh->vertices.push_back(v);
    }
    
    v.pos = glm::vec3(offset*taper-xl[div]*tipChordLength,wingLength,0.f);
    mesh->vertices.push_back(v);
    
    v.pos = glm::vec3(offset*taper-xu[div]*tipChordLength,wingLength,0.f);
    mesh->vertices.push_back(v);
    
    //Faces
    for(int i=3*div+3; i<=4*div+2; ++i)
    {
        Face f;
        f.vertexID[0] = i*2;
        f.vertexID[1] = i*2+1;
        f.vertexID[2] = i*2+2;
        mesh->faces.push_back(f);
        f.vertexID[0] = i*2+2;
        f.vertexID[1] = i*2+1;
        f.vertexID[2] = i*2+3;
        mesh->faces.push_back(f);
    }
    
    return mesh;
}
    
Mesh* OpenGLContent::BuildTerrain(GLfloat* heightfield, int sizeX, int sizeY, GLfloat scaleX, GLfloat scaleY, GLfloat maxHeight)
{
    Mesh* mesh = new Mesh;
    mesh->hasUVs = true;
    
    Face f;
    Vertex vt;
    
    GLfloat fullSizeX = (sizeX-1) * scaleX;
    GLfloat fullSizeY = (sizeY-1) * scaleY;
    GLfloat offsetX = fullSizeX/2.f;
    GLfloat offsetY = fullSizeY/2.f;
    
    for(int i=0; i<sizeY; ++i)
        for(int j=0; j<sizeX; ++j)
        {
            vt.pos = glm::vec3((GLfloat)j/(GLfloat)(sizeX-1)*fullSizeX-offsetX, (GLfloat)i/(GLfloat)(sizeY-1)*fullSizeY-offsetY, heightfield[i*sizeX + j]-maxHeight/2.f);
            vt.normal = glm::vec3(0.f,0.f,-1.f);
            vt.uv = glm::vec2((GLfloat)j/(GLfloat)(sizeX-1), (GLfloat)i/(GLfloat)(sizeY-1));
            mesh->vertices.push_back(vt);
        }
    
    for(int i=0; i<sizeY-1; ++i)
        for(int j=0; j<sizeX-1; ++j)
        {
            f.vertexID[0] = i*sizeX + j;
            f.vertexID[1] = (i+1)*sizeX + j;
            f.vertexID[2] = i*sizeX + j + 1;
            mesh->faces.push_back(f);
            f.vertexID[0] = f.vertexID[1];
            f.vertexID[1] = (i+1)*sizeX + j + 1;
            mesh->faces.push_back(f);
        }
    
    SmoothNormals(mesh);
    
    return mesh;
}

Mesh* OpenGLContent::LoadMesh(std::string filename, GLfloat scale, bool smooth)
{
    Mesh* mesh = LoadGeometryFromFile(filename, scale);
    if(mesh == NULL)
        abort();
    if(smooth)
        SmoothNormals(mesh);
    return mesh;
}

void OpenGLContent::TransformMesh(Mesh* mesh, const Transform& T)
{
    glm::mat4 gT = glMatrixFromTransform(T);
    glm::mat3 gR(gT);
    
    for(size_t i=0; i<mesh->vertices.size(); ++i)
    {
        mesh->vertices[i].pos = glm::vec3(gT * glm::vec4(mesh->vertices[i].pos, 1.f));
        mesh->vertices[i].normal = gR * mesh->vertices[i].normal;
    }
}

void OpenGLContent::SmoothNormals(Mesh* mesh)
{
    //For all faces
    for(unsigned int i=0; i<mesh->faces.size(); ++i)
    {
        Face thisFace = mesh->faces[i];
        glm::vec3 thisN = mesh->ComputeFaceNormal(i);
        
        //For every vertex
        for(unsigned short int h=0; h<3; ++h)
        {
            glm::vec3 n = thisN;
            unsigned int contrib = 1;
            
            //Loop through all faces
            for(unsigned int j=0; j<mesh->faces.size(); ++j)
            {
                if(j != i) //Reject same face
                {
                    Face thatFace = mesh->faces[j];
                    glm::vec3 thatN = mesh->ComputeFaceNormal(j);
                    
                    for(unsigned short int k=0; k<3; k++)
                    {
                        if((mesh->vertices[thatFace.vertexID[k]].pos == mesh->vertices[thisFace.vertexID[h]].pos)&&(glm::dot(thatN,thisN) > 0.707f))
                        {
                            n += thatN;
                            contrib++;
                            break;
                        }
                    }
                }
            }
            
            n /= (GLfloat)contrib;
            mesh->vertices[mesh->faces[i].vertexID[h]].normal = n;
        }
    }
}

GLuint vertex4EdgeIco(std::map<std::pair<GLuint, GLuint>, GLuint>& lookup, Mesh* mesh, GLuint firstID, GLuint secondID)
{
    std::map<std::pair<GLuint, GLuint>, GLuint>::key_type key(firstID, secondID);
    if(key.first > key.second)
        std::swap(key.first, key.second);
        
    auto inserted=lookup.insert({key, mesh->vertices.size()});
    if(inserted.second)
    {
        glm::vec3 edge0 = mesh->vertices[firstID].pos;
        glm::vec3 edge1 = mesh->vertices[secondID].pos;
        Vertex vt;
        vt.pos =  glm::normalize(edge0 + edge1);
        vt.normal = vt.pos;
        vt.uv = glm::vec2(0,0);
        mesh->vertices.push_back(vt);
    }
    
    return inserted.first->second;
}

GLuint vertex4Edge(std::map<std::pair<GLuint, GLuint>, GLuint>& lookup, Mesh* mesh, GLuint firstID, GLuint secondID)
{
    std::map<std::pair<GLuint, GLuint>, GLuint>::key_type key(firstID, secondID);
    if(key.first > key.second)
        std::swap(key.first, key.second);
        
    auto inserted=lookup.insert({key, mesh->vertices.size()});
    if(inserted.second)
    {
        glm::vec3 edge0 = mesh->vertices[firstID].pos;
        glm::vec3 edge1 = mesh->vertices[secondID].pos;
        glm::vec3 edge0n = mesh->vertices[firstID].normal;
        glm::vec3 edge1n = mesh->vertices[secondID].normal;
        glm::vec2 edge0uv = mesh->vertices[firstID].uv;
        glm::vec2 edge1uv = mesh->vertices[secondID].uv;
        
        Vertex vt;
        vt.pos = (edge0 + edge1)/(GLfloat)2;
        vt.normal = glm::normalize(edge0n + edge1n);
        vt.uv = (edge0uv + edge1uv)/(GLfloat)2;
        mesh->vertices.push_back(vt);
    }
    
    return inserted.first->second;
}
    
GLfloat OpenGLContent::ComputeAverageFaceArea(Mesh* mesh)
{
    GLfloat area = 0.f;
    for(size_t i=0; i<mesh->faces.size(); ++i)
        area += mesh->ComputeFaceArea(i);
    return area/(GLfloat)mesh->faces.size();
}

void OpenGLContent::Subdivide(Mesh* mesh, bool icoMode)
{
    std::map<std::pair<GLuint, GLuint>, GLuint> lookup;
    std::vector<Face> newFaces;
    
    for(GLuint i=0; i<mesh->faces.size(); ++i)
    {
        GLuint mid[3];
        
        if(icoMode)
        {
            for(int edge = 0; edge<3; ++edge)
                mid[edge] = vertex4EdgeIco(lookup, mesh, mesh->faces[i].vertexID[edge], mesh->faces[i].vertexID[(edge+1)%3]);
        }
        else
        {
            for(int edge = 0; edge<3; ++edge)
                mid[edge] = vertex4Edge(lookup, mesh, mesh->faces[i].vertexID[edge], mesh->faces[i].vertexID[(edge+1)%3]);
        }
            
        Face f;
        f.vertexID[0] = mesh->faces[i].vertexID[0];
        f.vertexID[1] = mid[0];
        f.vertexID[2] = mid[2];
        newFaces.push_back(f);
        
        f.vertexID[0] = mesh->faces[i].vertexID[1];
        f.vertexID[1] = mid[1];
        f.vertexID[2] = mid[0];
        newFaces.push_back(f);
        
        f.vertexID[0] = mesh->faces[i].vertexID[2];
        f.vertexID[1] = mid[2];
        f.vertexID[2] = mid[1];
        newFaces.push_back(f);
        
        f.vertexID[0] = mid[0];
        f.vertexID[1] = mid[1];
        f.vertexID[2] = mid[2];
        newFaces.push_back(f);
    }
    
    mesh->faces.clear();
    mesh->faces = newFaces;
}

void OpenGLContent::Refine(Mesh* mesh, GLfloat sizeThreshold)
{
    const GLfloat minArea = 0.01f*0.01f;
    GLfloat avgFaceArea = std::max(ComputeAverageFaceArea(mesh), minArea);
    size_t nSubdivided = 0;
#ifdef DEBUG
    size_t nFaceBefore = mesh->faces.size();
#endif
    
    while(1)
    {
        std::vector<Face> newFaces;
        
        for(size_t i=0; i<mesh->faces.size(); ++i)
        {
            if(mesh->ComputeFaceArea(i) > sizeThreshold * avgFaceArea)
            {
                GLuint mid[3];
                std::map<std::pair<GLuint, GLuint>, GLuint> lookup;
                
                for(unsigned int edge = 0; edge<3; ++edge)
                    mid[edge] = vertex4Edge(lookup, mesh, mesh->faces[i].vertexID[edge], mesh->faces[i].vertexID[(edge+1)%3]);
                
                Face f;
                f.vertexID[0] = mesh->faces[i].vertexID[0];
                f.vertexID[1] = mid[0];
                f.vertexID[2] = mid[2];
                newFaces.push_back(f);
                
                f.vertexID[0] = mesh->faces[i].vertexID[1];
                f.vertexID[1] = mid[1];
                f.vertexID[2] = mid[0];
                newFaces.push_back(f);
                
                f.vertexID[0] = mesh->faces[i].vertexID[2];
                f.vertexID[1] = mid[2];
                f.vertexID[2] = mid[1];
                newFaces.push_back(f);
                
                f.vertexID[0] = mid[0];
                f.vertexID[1] = mid[1];
                f.vertexID[2] = mid[2];
                newFaces.push_back(f);
                
                ++nSubdivided;
            }
            else
                newFaces.push_back(mesh->faces[i]);
        }
        
        if(nSubdivided > 0)
        {
            mesh->faces.clear();
            mesh->faces = newFaces;
            avgFaceArea = std::max(ComputeAverageFaceArea(mesh), minArea);
            nSubdivided = 0;
        }
        else
            break;
    }
    
#ifdef DEBUG
    cInfo("Mesh refined (%ld/%ld).", nFaceBefore, mesh->faces.size());
#endif
}
    
void OpenGLContent::AABB(Mesh* mesh, glm::vec3& min, glm::vec3& max)
{
    GLfloat minX=BT_LARGE_FLOAT, maxX=-BT_LARGE_FLOAT;
    GLfloat minY=BT_LARGE_FLOAT, maxY=-BT_LARGE_FLOAT;
    GLfloat minZ=BT_LARGE_FLOAT, maxZ=-BT_LARGE_FLOAT;
    
    for(unsigned int i=0; i<mesh->vertices.size(); i++)
    {
        glm::vec3 vertex = mesh->vertices[i].pos;
        
        if(vertex.x > maxX)
            maxX = vertex.x;
        
        if(vertex.x < minX)
            minX = vertex.x;
        
        if(vertex.y > maxY)
            maxY = vertex.y;
        
        if(vertex.y < minY)
            minY = vertex.y;
        
        if(vertex.z >maxZ)
            maxZ = vertex.z;
        
        if(vertex.z < minZ)
            minZ = vertex.z;
    }
    
    min = glm::vec3(minX, minY, minZ);
    max = glm::vec3(maxX, maxY, maxZ);
}

void OpenGLContent::AABS(Mesh* mesh, GLfloat& bsRadius, glm::vec3& bsCenterOffset)
{
    glm::vec3 tempCenter(0,0,0);
    
    for(unsigned int i=0; i<mesh->vertices.size(); ++i)
        tempCenter += mesh->vertices[i].pos;
    
    tempCenter /= (GLfloat)mesh->vertices.size();
    
    GLfloat radius = 0;
    
    for(unsigned int i=0; i<mesh->vertices.size(); ++i)
    {
        glm::vec3 v = mesh->vertices[i].pos;
        GLfloat r = glm::length(v-tempCenter);
        if(r > radius)
            radius = r;
    }
    
    bsRadius = radius;
    bsCenterOffset = tempCenter;
}

}
