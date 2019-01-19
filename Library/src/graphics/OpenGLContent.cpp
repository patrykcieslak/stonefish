//
//  OpenGLContent.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/06/17.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLContent.h"

#include <map>
#include <algorithm>
#include "core/Console.h"
#include "core/SimulationManager.h"
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
	csBuf[0] = 0;
	csBuf[1] = 0;
    ellipsoid.mesh = NULL;
    cylinder.mesh = NULL;
	helperShader = NULL;
	texQuadShader = NULL;
	texQuadMSShader = NULL;
	texLayerQuadShader = NULL;
	texLevelQuadShader = NULL;
	texCubeShader = NULL;
	flatShader = NULL;
	eyePos = glm::vec3();
	viewDir = glm::vec3(1.f,0,0);
	viewProjection = glm::mat4();
	view = glm::mat4();
	projection = glm::mat4();
	viewportSize = glm::vec2(800.f,600.f);
	mode = DrawingMode::FULL;
	clipPlane = glm::vec4();
    materialShaders = std::vector<GLSLShader*>(0);

    //Initialize shaders and buffers
    cInfo("Loading shaders...");
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
    cylinder.mesh = BuildCylinder(1.f, 1.f, 12);
    
    glGenVertexArrays(1, &cylinder.vao);
    glGenBuffers(1, &cylinder.vboVertex);
    glGenBuffers(1, &cylinder.vboIndex);
    
    glBindVertexArray(cylinder.vao);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, cylinder.vboVertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*cylinder.mesh->vertices.size(), &cylinder.mesh->vertices[0].pos.x, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinder.vboIndex);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Face)*cylinder.mesh->faces.size(), &cylinder.mesh->faces[0].vertexID[0], GL_STATIC_DRAW);
    glBindVertexArray(0);
    
    //Ellipsoid helper
    ellipsoid.mesh = BuildSphere(1.f, 3);
    
    glGenVertexArrays(1, &ellipsoid.vao);
    glGenBuffers(1, &ellipsoid.vboVertex);
    glGenBuffers(1, &ellipsoid.vboIndex);
    
    glBindVertexArray(ellipsoid.vao);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, ellipsoid.vboVertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*ellipsoid.mesh->vertices.size(), &ellipsoid.mesh->vertices[0].pos.x, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ellipsoid.vboIndex);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Face)*ellipsoid.mesh->faces.size(), &ellipsoid.mesh->faces[0].vertexID[0], GL_STATIC_DRAW);
    glBindVertexArray(0);
    
    //Load shaders
    //Basic
    helperShader = new GLSLShader("helpers.frag","helpers.vert");
    helperShader->AddUniform("MVP", ParameterType::MAT4);
    helperShader->AddUniform("scale", ParameterType::VEC3);
    
    texQuadShader = new GLSLShader("texQuad.frag","texQuad.vert");
    texQuadShader->AddUniform("rect", ParameterType::VEC4);
    texQuadShader->AddUniform("tex", ParameterType::INT);
    texQuadShader->AddUniform("color", ParameterType::VEC4);
    
    texQuadMSShader = new GLSLShader("texQuadMS.frag","texQuad.vert");
    texQuadMSShader->AddUniform("rect", ParameterType::VEC4);
    texQuadMSShader->AddUniform("tex", ParameterType::INT);
    texQuadMSShader->AddUniform("texSize", ParameterType::IVEC2);
    
    texLayerQuadShader = new GLSLShader("texLayerQuad.frag", "texQuad.vert");
    texLayerQuadShader->AddUniform("rect", ParameterType::VEC4);
    texLayerQuadShader->AddUniform("tex", ParameterType::INT);
    texLayerQuadShader->AddUniform("layer", ParameterType::INT);
    
    texLevelQuadShader = new GLSLShader("texLevelQuad.frag", "texQuad.vert");
    texLevelQuadShader->AddUniform("rect", ParameterType::VEC4);
    texLevelQuadShader->AddUniform("tex", ParameterType::INT);
    texLevelQuadShader->AddUniform("level", ParameterType::INT);
    
    texCubeShader = new GLSLShader("texCube.frag", "texCube.vert");
    texCubeShader->AddUniform("tex", ParameterType::INT);
    
    flatShader = new GLSLShader("flat.frag", "flat.vert");
    flatShader->AddUniform("MVP", ParameterType::MAT4);
    
    //Materials
    GLint compiled;
    std::string header = "";
    std::vector<GLuint> commonMaterialShaders;
    commonMaterialShaders.push_back(OpenGLAtmosphere::getAtmosphereAPI());
    
    GLuint materialFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "material.frag", header, &compiled);
    commonMaterialShaders.push_back(materialFragment);
    
    //Blinn-Phong shader
    GLSLShader* blinnPhong = new GLSLShader(commonMaterialShaders, "blinnPhong.frag", "material.vert");
    blinnPhong->AddUniform("MVP", ParameterType::MAT4);
    blinnPhong->AddUniform("M", ParameterType::MAT4);
    blinnPhong->AddUniform("N", ParameterType::MAT3);
    blinnPhong->AddUniform("MV", ParameterType::MAT3);
    blinnPhong->AddUniform("clipPlane", ParameterType::VEC4);
    blinnPhong->AddUniform("eyePos", ParameterType::VEC3);
    blinnPhong->AddUniform("viewDir", ParameterType::VEC3);
    blinnPhong->AddUniform("color", ParameterType::VEC4);
    blinnPhong->AddUniform("tex", ParameterType::INT);
    blinnPhong->AddUniform("shininess", ParameterType::FLOAT);
    blinnPhong->AddUniform("specularStrength", ParameterType::FLOAT);
    blinnPhong->AddUniform("reflectivity", ParameterType::FLOAT);
    blinnPhong->AddUniform("numPointLights", ParameterType::INT);
    blinnPhong->AddUniform("numSpotLights", ParameterType::INT);
    blinnPhong->AddUniform("spotLightsDepthMap", ParameterType::INT);
    blinnPhong->AddUniform("spotLightsShadowMap", ParameterType::INT);
    
    for(unsigned int i=0; i<MAX_POINT_LIGHTS; ++i)
    {
        std::string lightUni = "pointLights[" + std::to_string(i) + "].";
        blinnPhong->AddUniform(lightUni + "position", ParameterType::VEC3);
        blinnPhong->AddUniform(lightUni + "color", ParameterType::VEC3);
    }
    
    for(unsigned int i=0; i<MAX_SPOT_LIGHTS; ++i)
    {
        std::string lightUni = "spotLights[" + std::to_string(i) + "].";
        blinnPhong->AddUniform(lightUni + "position", ParameterType::VEC3);
        blinnPhong->AddUniform(lightUni + "radius", ParameterType::VEC2);
        blinnPhong->AddUniform(lightUni + "color", ParameterType::VEC3);
        blinnPhong->AddUniform(lightUni + "direction", ParameterType::VEC3);
        blinnPhong->AddUniform(lightUni + "angle", ParameterType::FLOAT);
        blinnPhong->AddUniform(lightUni + "clipSpace", ParameterType::MAT4);
        blinnPhong->AddUniform(lightUni + "zNear", ParameterType::FLOAT);
        blinnPhong->AddUniform(lightUni + "zFar", ParameterType::FLOAT);
    }
    
    blinnPhong->AddUniform("sunDirection", ParameterType::VEC3);
    blinnPhong->AddUniform("sunClipSpace[0]", ParameterType::MAT4);
    blinnPhong->AddUniform("sunClipSpace[1]", ParameterType::MAT4);
    blinnPhong->AddUniform("sunClipSpace[2]", ParameterType::MAT4);
    blinnPhong->AddUniform("sunClipSpace[3]", ParameterType::MAT4);
    blinnPhong->AddUniform("sunFrustumNear[0]", ParameterType::FLOAT);
    blinnPhong->AddUniform("sunFrustumNear[1]", ParameterType::FLOAT);
    blinnPhong->AddUniform("sunFrustumNear[2]", ParameterType::FLOAT);
    blinnPhong->AddUniform("sunFrustumNear[3]", ParameterType::FLOAT);
    blinnPhong->AddUniform("sunFrustumFar[0]", ParameterType::FLOAT);
    blinnPhong->AddUniform("sunFrustumFar[1]", ParameterType::FLOAT);
    blinnPhong->AddUniform("sunFrustumFar[2]", ParameterType::FLOAT);
    blinnPhong->AddUniform("sunFrustumFar[3]", ParameterType::FLOAT);
    blinnPhong->AddUniform("sunShadowMap", ParameterType::INT);
    blinnPhong->AddUniform("sunDepthMap", ParameterType::INT);
    blinnPhong->AddUniform("transmittance_texture", ParameterType::INT);
    blinnPhong->AddUniform("scattering_texture", ParameterType::INT);
    blinnPhong->AddUniform("irradiance_texture", ParameterType::INT);
    blinnPhong->AddUniform("planetRadius", ParameterType::FLOAT);
    blinnPhong->AddUniform("whitePoint", ParameterType::VEC3);
    
    materialShaders.push_back(blinnPhong);
    
    //Cook-Torrance shader
    GLSLShader* cookTorrance = new GLSLShader(commonMaterialShaders, "cookTorrance.frag", "material.vert");
    cookTorrance->AddUniform("MVP", ParameterType::MAT4);
    cookTorrance->AddUniform("M", ParameterType::MAT4);
    cookTorrance->AddUniform("N", ParameterType::MAT3);
    cookTorrance->AddUniform("MV", ParameterType::MAT3);
    cookTorrance->AddUniform("clipPlane", ParameterType::VEC4);
    cookTorrance->AddUniform("eyePos", ParameterType::VEC3);
    cookTorrance->AddUniform("viewDir", ParameterType::VEC3);
    cookTorrance->AddUniform("color", ParameterType::VEC4);
    cookTorrance->AddUniform("tex", ParameterType::INT);
    cookTorrance->AddUniform("roughness", ParameterType::FLOAT);
    cookTorrance->AddUniform("metallic", ParameterType::FLOAT);
    cookTorrance->AddUniform("reflectivity", ParameterType::FLOAT);
    cookTorrance->AddUniform("numPointLights", ParameterType::INT);
    cookTorrance->AddUniform("numSpotLights", ParameterType::INT);
    cookTorrance->AddUniform("spotLightsDepthMap", ParameterType::INT);
    cookTorrance->AddUniform("spotLightsShadowMap", ParameterType::INT);
    
    for(unsigned int i=0; i<MAX_POINT_LIGHTS; ++i)
    {
        std::string lightUni = "pointLights[" + std::to_string(i) + "].";
        cookTorrance->AddUniform(lightUni + "position", ParameterType::VEC3);
        cookTorrance->AddUniform(lightUni + "color", ParameterType::VEC3);
    }
    
    for(unsigned int i=0; i<MAX_SPOT_LIGHTS; ++i)
    {
        std::string lightUni = "spotLights[" + std::to_string(i) + "].";
        cookTorrance->AddUniform(lightUni + "position", ParameterType::VEC3);
        cookTorrance->AddUniform(lightUni + "radius", ParameterType::VEC2);
        cookTorrance->AddUniform(lightUni + "color", ParameterType::VEC3);
        cookTorrance->AddUniform(lightUni + "direction", ParameterType::VEC3);
        cookTorrance->AddUniform(lightUni + "angle", ParameterType::FLOAT);
        cookTorrance->AddUniform(lightUni + "clipSpace", ParameterType::MAT4);
        cookTorrance->AddUniform(lightUni + "zNear", ParameterType::FLOAT);
        cookTorrance->AddUniform(lightUni + "zFar", ParameterType::FLOAT);
    }
    
    cookTorrance->AddUniform("sunDirection", ParameterType::VEC3);
    cookTorrance->AddUniform("sunClipSpace[0]", ParameterType::MAT4);
    cookTorrance->AddUniform("sunClipSpace[1]", ParameterType::MAT4);
    cookTorrance->AddUniform("sunClipSpace[2]", ParameterType::MAT4);
    cookTorrance->AddUniform("sunClipSpace[3]", ParameterType::MAT4);
    cookTorrance->AddUniform("sunFrustumNear[0]", ParameterType::FLOAT);
    cookTorrance->AddUniform("sunFrustumNear[1]", ParameterType::FLOAT);
    cookTorrance->AddUniform("sunFrustumNear[2]", ParameterType::FLOAT);
    cookTorrance->AddUniform("sunFrustumNear[3]", ParameterType::FLOAT);
    cookTorrance->AddUniform("sunFrustumFar[0]", ParameterType::FLOAT);
    cookTorrance->AddUniform("sunFrustumFar[1]", ParameterType::FLOAT);
    cookTorrance->AddUniform("sunFrustumFar[2]", ParameterType::FLOAT);
    cookTorrance->AddUniform("sunFrustumFar[3]", ParameterType::FLOAT);
    cookTorrance->AddUniform("sunShadowMap", ParameterType::INT);
    cookTorrance->AddUniform("sunDepthMap", ParameterType::INT);
    cookTorrance->AddUniform("transmittance_texture", ParameterType::INT);
    cookTorrance->AddUniform("scattering_texture", ParameterType::INT);
    cookTorrance->AddUniform("irradiance_texture", ParameterType::INT);
    cookTorrance->AddUniform("planetRadius", ParameterType::FLOAT);
    cookTorrance->AddUniform("whitePoint", ParameterType::VEC3);
    
    materialShaders.push_back(cookTorrance);
    glDeleteShader(materialFragment);
    
    //-------------Underwater shaders---------------------------
    header = "";
    commonMaterialShaders.clear();
    commonMaterialShaders.push_back(OpenGLAtmosphere::getAtmosphereAPI());
    
    materialFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "underwaterMaterial.frag", header, &compiled);
    commonMaterialShaders.push_back(materialFragment);
    
    //Blinn-Phong shader
    GLSLShader* uwBlinnPhong = new GLSLShader(commonMaterialShaders, "blinnPhong.frag", "material.vert");
    uwBlinnPhong->AddUniform("MVP", ParameterType::MAT4);
    uwBlinnPhong->AddUniform("M", ParameterType::MAT4);
    uwBlinnPhong->AddUniform("N", ParameterType::MAT3);
    uwBlinnPhong->AddUniform("MV", ParameterType::MAT3);
    uwBlinnPhong->AddUniform("clipPlane", ParameterType::VEC4);
    uwBlinnPhong->AddUniform("eyePos", ParameterType::VEC3);
    uwBlinnPhong->AddUniform("viewDir", ParameterType::VEC3);
    uwBlinnPhong->AddUniform("color", ParameterType::VEC4);
    uwBlinnPhong->AddUniform("tex", ParameterType::INT);
    uwBlinnPhong->AddUniform("shininess", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("specularStrength", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("reflectivity", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("lightAbsorption", ParameterType::VEC3);
    uwBlinnPhong->AddUniform("turbidity", ParameterType::FLOAT);
    
    uwBlinnPhong->AddUniform("numPointLights", ParameterType::INT);
    uwBlinnPhong->AddUniform("numSpotLights", ParameterType::INT);
    uwBlinnPhong->AddUniform("spotLightsDepthMap", ParameterType::INT);
    uwBlinnPhong->AddUniform("spotLightsShadowMap", ParameterType::INT);
    
    for(unsigned int i=0; i<MAX_POINT_LIGHTS; ++i)
    {
        std::string lightUni = "pointLights[" + std::to_string(i) + "].";
        uwBlinnPhong->AddUniform(lightUni + "position", ParameterType::VEC3);
        uwBlinnPhong->AddUniform(lightUni + "color", ParameterType::VEC3);
    }
    
    for(unsigned int i=0; i<MAX_SPOT_LIGHTS; ++i)
    {
        std::string lightUni = "spotLights[" + std::to_string(i) + "].";
        uwBlinnPhong->AddUniform(lightUni + "position", ParameterType::VEC3);
        uwBlinnPhong->AddUniform(lightUni + "radius", ParameterType::VEC2);
        uwBlinnPhong->AddUniform(lightUni + "color", ParameterType::VEC3);
        uwBlinnPhong->AddUniform(lightUni + "direction", ParameterType::VEC3);
        uwBlinnPhong->AddUniform(lightUni + "angle", ParameterType::FLOAT);
        uwBlinnPhong->AddUniform(lightUni + "clipSpace", ParameterType::MAT4);
        uwBlinnPhong->AddUniform(lightUni + "zNear", ParameterType::FLOAT);
        uwBlinnPhong->AddUniform(lightUni + "zFar", ParameterType::FLOAT);
    }
    
    uwBlinnPhong->AddUniform("sunDirection", ParameterType::VEC3);
    uwBlinnPhong->AddUniform("sunClipSpace[0]", ParameterType::MAT4);
    uwBlinnPhong->AddUniform("sunClipSpace[1]", ParameterType::MAT4);
    uwBlinnPhong->AddUniform("sunClipSpace[2]", ParameterType::MAT4);
    uwBlinnPhong->AddUniform("sunClipSpace[3]", ParameterType::MAT4);
    uwBlinnPhong->AddUniform("sunFrustumNear[0]", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("sunFrustumNear[1]", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("sunFrustumNear[2]", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("sunFrustumNear[3]", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("sunFrustumFar[0]", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("sunFrustumFar[1]", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("sunFrustumFar[2]", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("sunFrustumFar[3]", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("sunShadowMap", ParameterType::INT);
    uwBlinnPhong->AddUniform("sunDepthMap", ParameterType::INT);
    uwBlinnPhong->AddUniform("transmittance_texture", ParameterType::INT);
    uwBlinnPhong->AddUniform("scattering_texture", ParameterType::INT);
    uwBlinnPhong->AddUniform("irradiance_texture", ParameterType::INT);
    uwBlinnPhong->AddUniform("planetRadius", ParameterType::FLOAT);
    uwBlinnPhong->AddUniform("whitePoint", ParameterType::VEC3);
    
    materialShaders.push_back(uwBlinnPhong);
    
    //Underwater Cook-Torrance shader
    GLSLShader* uwCookTorrance = new GLSLShader(commonMaterialShaders, "cookTorrance.frag", "material.vert");
    uwCookTorrance->AddUniform("MVP", ParameterType::MAT4);
    uwCookTorrance->AddUniform("M", ParameterType::MAT4);
    uwCookTorrance->AddUniform("N", ParameterType::MAT3);
    uwCookTorrance->AddUniform("MV", ParameterType::MAT3);
    uwCookTorrance->AddUniform("clipPlane", ParameterType::VEC4);
    uwCookTorrance->AddUniform("eyePos", ParameterType::VEC3);
    uwCookTorrance->AddUniform("viewDir", ParameterType::VEC3);
    uwCookTorrance->AddUniform("color", ParameterType::VEC4);
    uwCookTorrance->AddUniform("tex", ParameterType::INT);
    uwCookTorrance->AddUniform("roughness", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("metallic", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("reflectivity", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("lightAbsorption", ParameterType::VEC3);
    uwCookTorrance->AddUniform("turbidity", ParameterType::FLOAT);
    
    uwCookTorrance->AddUniform("numPointLights", ParameterType::INT);
    uwCookTorrance->AddUniform("numSpotLights", ParameterType::INT);
    uwCookTorrance->AddUniform("spotLightsDepthMap", ParameterType::INT);
    uwCookTorrance->AddUniform("spotLightsShadowMap", ParameterType::INT);
    
    for(unsigned int i=0; i<MAX_POINT_LIGHTS; ++i)
    {
        std::string lightUni = "pointLights[" + std::to_string(i) + "].";
        uwCookTorrance->AddUniform(lightUni + "position", ParameterType::VEC3);
        uwCookTorrance->AddUniform(lightUni + "color", ParameterType::VEC3);
    }
    
    for(unsigned int i=0; i<MAX_SPOT_LIGHTS; ++i)
    {
        std::string lightUni = "spotLights[" + std::to_string(i) + "].";
        uwCookTorrance->AddUniform(lightUni + "position", ParameterType::VEC3);
        uwCookTorrance->AddUniform(lightUni + "radius", ParameterType::VEC2);
        uwCookTorrance->AddUniform(lightUni + "color", ParameterType::VEC3);
        uwCookTorrance->AddUniform(lightUni + "direction", ParameterType::VEC3);
        uwCookTorrance->AddUniform(lightUni + "angle", ParameterType::FLOAT);
        uwCookTorrance->AddUniform(lightUni + "clipSpace", ParameterType::MAT4);
        uwCookTorrance->AddUniform(lightUni + "zNear", ParameterType::FLOAT);
        uwCookTorrance->AddUniform(lightUni + "zFar", ParameterType::FLOAT);
    }
    
    uwCookTorrance->AddUniform("sunDirection", ParameterType::VEC3);
    uwCookTorrance->AddUniform("sunClipSpace[0]", ParameterType::MAT4);
    uwCookTorrance->AddUniform("sunClipSpace[1]", ParameterType::MAT4);
    uwCookTorrance->AddUniform("sunClipSpace[2]", ParameterType::MAT4);
    uwCookTorrance->AddUniform("sunClipSpace[3]", ParameterType::MAT4);
    uwCookTorrance->AddUniform("sunFrustumNear[0]", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("sunFrustumNear[1]", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("sunFrustumNear[2]", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("sunFrustumNear[3]", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("sunFrustumFar[0]", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("sunFrustumFar[1]", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("sunFrustumFar[2]", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("sunFrustumFar[3]", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("sunShadowMap", ParameterType::INT);
    uwCookTorrance->AddUniform("sunDepthMap", ParameterType::INT);
    uwCookTorrance->AddUniform("transmittance_texture", ParameterType::INT);
    uwCookTorrance->AddUniform("scattering_texture", ParameterType::INT);
    uwCookTorrance->AddUniform("irradiance_texture", ParameterType::INT);
    uwCookTorrance->AddUniform("planetRadius", ParameterType::FLOAT);
    uwCookTorrance->AddUniform("whitePoint", ParameterType::VEC3);
    
    materialShaders.push_back(uwCookTorrance);
    glDeleteShader(materialFragment);
}

OpenGLContent::~OpenGLContent()
{
	//Base shaders
	if(baseVertexArray != 0) glDeleteVertexArrays(1, &baseVertexArray);
	if(quadBuf != 0) glDeleteBuffers(1, &quadBuf);
	if(cubeBuf != 0) glDeleteBuffers(1, &cubeBuf);
	if(csBuf[0] != 0) glDeleteBuffers(2, csBuf);
	if(helperShader != NULL) delete helperShader;
	if(texQuadShader != NULL) delete texQuadShader;
	if(texQuadMSShader != NULL) delete texQuadMSShader;
	if(texLayerQuadShader != NULL) delete texLayerQuadShader;
	if(texLevelQuadShader != NULL) delete texLevelQuadShader;
	if(texCubeShader != NULL) delete texCubeShader;
	if(flatShader != NULL) delete flatShader;
	
	//Material shaders
	for(unsigned int i=0; i<materialShaders.size(); ++i)
		delete materialShaders[i];
	materialShaders.clear();
}

void OpenGLContent::Finalize()
{
	cInfo("Finalizing OpenGL rendering pipeline...");
	OpenGLLight::Init(lights);
}

void OpenGLContent::DestroyContent()
{
	for(unsigned int i=0; i<looks.size(); ++i)
	{
		for(unsigned int h=0; h<looks[i].textures.size(); ++h)
			glDeleteTextures(1, &looks[i].textures[h]);
	}
	looks.clear();
			
	for(unsigned int i=0; i<objects.size(); ++i)
	{
		glDeleteBuffers(1, &objects[i].vboVertex);
		glDeleteBuffers(1, &objects[i].vboIndex);
		glDeleteVertexArrays(1, &objects[i].vao);
	}	
	objects.clear();
	
	for(unsigned int i=0; i<views.size(); ++i)
        delete views[i];
    views.clear();
    
    for(unsigned int i=0; i<lights.size(); ++i)
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

void OpenGLContent::SetCurrentView(OpenGLView* v, bool mirror)
{
	if(mirror)
	{
		glm::vec3 n(0,0,-1.f);
		GLfloat D = 0;
		glm::mat4 reflection = glm::mat4(1.f-2.f*n.x*n.x, -2.f*n.x*n.y, -2.f*n.x*n.z, -2.f*n.x*D,
										-2.f*n.x*n.y, 1.f-2.f*n.y*n.y, -2.f*n.y*n.z, -2.f*n.y*D,
										-2.f*n.x*n.z, -2.f*n.y*n.z, 1.f-2.f*n.z*n.z, -2.f*n.z*D,
													0,			  0,			   0,		   1);
													
		/*glm::mat4 reflection = glm::mat4(1.f-2.f*n.x*n.x, -2.f*n.x*n.y, -2.f*n.x*n.z, 0,
										 -2.f*n.x*n.y, 1.f-2.f*n.y*n.y, -2.f*n.y*n.z, 0,
										 -2.f*n.x*n.z, -2.f*n.y*n.z, 1.f-2.f*n.z*n.z, 0,
										 -2.f*n.x*D,   -2.f*n.y*D,   -2.f*n.z*D,      1);*/									
													
		//glm::mat4 flip = glm::mat4(1.f,0,0,0, 0,-1.f,0,0, 0,0,1.f,0, 0,0,0,1.f);											
		
		eyePos = v->GetEyePosition();
		eyePos.z = -eyePos.z;
		viewDir = v->GetLookingDirection();
		viewDir.z = -viewDir.z;
		view = v->GetViewMatrix() * reflection;// * flip;// * reflection;// * glm::transpose(flip);
	}
	else
	{
		eyePos = v->GetEyePosition();
		viewDir = v->GetLookingDirection();
		view = v->GetViewMatrix();
	}
	
	projection = v->GetProjectionMatrix();
	viewProjection = projection * view;
}

void OpenGLContent::SetDrawingMode(DrawingMode m)
{
	mode = m;
}

void OpenGLContent::EnableClipPlane(glm::vec4 clipPlaneCoeff)
{
	clipPlane = clipPlaneCoeff;
	glEnable(GL_CLIP_DISTANCE0);
}

void OpenGLContent::DisableClipPlane()
{
	clipPlane = glm::vec4();
	glDisable(GL_CLIP_DISTANCE0);
}

void OpenGLContent::BindBaseVertexArray()
{
	glBindVertexArray(baseVertexArray);
}

void OpenGLContent::DrawSAQ()
{
	glBindVertexArray(baseVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
}

void OpenGLContent::DrawTexturedQuad(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLuint texture, glm::vec4 color)
{
	if(texQuadShader != NULL)
	{
		y = viewportSize.y-y-height;
		
		texQuadShader->Use();
		texQuadShader->SetUniform("rect", glm::vec4(x/viewportSize.x, y/viewportSize.y, width/viewportSize.x, height/viewportSize.y));
		texQuadShader->SetUniform("tex", TEX_BASE);
		texQuadShader->SetUniform("color", color);
		
        glActiveTexture(GL_TEXTURE0 + TEX_BASE);
        glBindTexture(GL_TEXTURE_2D, texture);
		glBindVertexArray(baseVertexArray);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, quadBuf); 
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
 		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDisableVertexAttribArray(0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
	}
}

void OpenGLContent::DrawTexturedQuad(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLuint texture, GLint z, bool array)
{
	if((array && texLayerQuadShader != NULL)||(!array && texLevelQuadShader != NULL))
	{
		y = viewportSize.y-y-height;
		
		if(array)
		{
			texLayerQuadShader->Use();
			texLayerQuadShader->SetUniform("rect", glm::vec4(x/viewportSize.x, y/viewportSize.y, width/viewportSize.x, height/viewportSize.y));
			texLayerQuadShader->SetUniform("tex", TEX_BASE);
			texLayerQuadShader->SetUniform("layer", z);
		}
		else
		{
			texLevelQuadShader->Use();
			texLevelQuadShader->SetUniform("rect", glm::vec4(x/viewportSize.x, y/viewportSize.y, width/viewportSize.x, height/viewportSize.y));
			texLevelQuadShader->SetUniform("tex", TEX_BASE);
			texLevelQuadShader->SetUniform("level", z);
		}
		
		glActiveTexture(GL_TEXTURE0 + TEX_BASE);
		glBindTexture(array ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_3D, texture);
		
		glBindVertexArray(baseVertexArray);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quadBuf); 
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
 		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisableVertexAttribArray(0);
		glBindVertexArray(0);
		
		glBindTexture(array ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_3D, 0);
		glUseProgram(0);
	}
}

void OpenGLContent::DrawTexturedQuad(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLuint textureMS, glm::ivec2 texSize)
{
	if(texQuadMSShader != NULL)
	{
		y = viewportSize.y-y-height;
		
		texQuadMSShader->Use();
		texQuadMSShader->SetUniform("rect", glm::vec4(x/viewportSize.x, y/viewportSize.y, width/viewportSize.x, height/viewportSize.y));
		texQuadMSShader->SetUniform("tex", TEX_BASE);
		texQuadMSShader->SetUniform("texSize", texSize);
		
		glActiveTexture(GL_TEXTURE0 + TEX_BASE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureMS);
		
		glBindVertexArray(baseVertexArray);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quadBuf); 
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
 		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisableVertexAttribArray(0);
		glBindVertexArray(0);
		
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glUseProgram(0);
	}
}	

void OpenGLContent::DrawCubemapCross(GLuint texture)
{
	if(cubeBuf != 0 && texCubeShader != NULL)
	{
		texCubeShader->Use();
		texCubeShader->SetUniform("tex", TEX_BASE);
		
		glActiveTexture(GL_TEXTURE0 + TEX_BASE);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
		
		glBindVertexArray(baseVertexArray);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		
		glBindBuffer(GL_ARRAY_BUFFER, cubeBuf); 
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
 		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 16);
		glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
		glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);
		
		glBindVertexArray(0);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glUseProgram(0);
	}
}

void OpenGLContent::DrawCoordSystem(glm::mat4 M, GLfloat size)
{
	if(csBuf[0] != 0 && helperShader != NULL)
	{
		helperShader->Use();
		helperShader->SetUniform("MVP", viewProjection*M);
		helperShader->SetUniform("scale", glm::vec3(size));
		
		glBindVertexArray(baseVertexArray);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		
		glBindBuffer(GL_ARRAY_BUFFER, csBuf[0]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, csBuf[1]);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		glDrawArrays(GL_LINES, 0, 6);
		glBindVertexArray(0);
		
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glUseProgram(0);
	}
}

void OpenGLContent::DrawCylinder(glm::mat4 M, glm::vec3 dims, glm::vec4 color)
{
    if(helperShader != NULL && cylinder.mesh != NULL)
    {
        helperShader->Use();
		helperShader->SetUniform("MVP", viewProjection*M);
		helperShader->SetUniform("scale", dims);
		
        glBindVertexArray(cylinder.vao);
        glVertexAttrib4fv(1, &color.r);
        glDrawElements(GL_TRIANGLES, 3 * (GLsizei)cylinder.mesh->faces.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }
}

void OpenGLContent::DrawEllipsoid(glm::mat4 M, glm::vec3 radii, glm::vec4 color)
{
    if(helperShader != NULL && ellipsoid.mesh != NULL)
    {
        helperShader->Use();
		helperShader->SetUniform("MVP", viewProjection*M);
		helperShader->SetUniform("scale", radii);
		
        glBindVertexArray(ellipsoid.vao);
        glVertexAttrib4fv(1, &color.r);
        glDrawElements(GL_TRIANGLES, 3 * (GLsizei)ellipsoid.mesh->faces.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }
}

void OpenGLContent::DrawPrimitives(PrimitiveType type, std::vector<glm::vec3>& vertices, glm::vec4 color, glm::mat4 M)
{
	if(helperShader != NULL && vertices.size() > 0)
	{
		GLuint vbo;
		glGenBuffers(1, &vbo);
		
		helperShader->Use();
		helperShader->SetUniform("MVP", viewProjection*M);
		helperShader->SetUniform("scale", glm::vec3(1.f));
		
		glBindVertexArray(baseVertexArray);
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
		glBindVertexArray(0);
		glDisableVertexAttribArray(0);
		glUseProgram(0);
		
		glDeleteBuffers(1, &vbo);
	}
}

void OpenGLContent::DrawObject(int objectId, int lookId, const glm::mat4& M)
{
	if(objectId >= 0 && objectId < (int)objects.size()) //Check if object exists
	{
		if(mode == DrawingMode::FLAT)
		{
			flatShader->Use();
			flatShader->SetUniform("MVP", viewProjection*M);
			glBindVertexArray(objects[objectId].vao);
			glDrawElements(GL_TRIANGLES, 3 * (GLsizei)objects[objectId].mesh->faces.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glUseProgram(0);
		}
		else
		{
			if(lookId >= 0 && lookId < (int)looks.size())
				UseLook(lookId, M);
			else
				UseStandardLook(M);
	
			glBindVertexArray(objects[objectId].vao);
			glDrawElements(GL_TRIANGLES, 3 * (GLsizei)objects[objectId].mesh->faces.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glUseProgram(0);
		}
	}
}

void OpenGLContent::SetupLights(GLSLShader* shader)
{
	int pointId = 0;
	int spotId = 0;
	
	for(unsigned int i=0; i<lights.size(); ++i)
	{
		if(lights[i]->getType() == POINT_LIGHT)
		{
			lights[i]->SetupShader(shader, pointId);
			++pointId;
		}
		else
		{
			lights[i]->SetupShader(shader, spotId);
			++spotId;
		}
	}
	
	shader->SetUniform("numPointLights", pointId);
	shader->SetUniform("numSpotLights", spotId);
	OpenGLLight::SetupShader(shader);
	SimulationApp::getApp()->getSimulationManager()->getAtmosphere()->getOpenGLAtmosphere()->SetupMaterialShader(shader);
}

void OpenGLContent::UseLook(unsigned int lookId, const glm::mat4& M)
{	
    Look& l = looks[lookId];
	GLSLShader* shader;
	
	switch(l.type)
	{		
        default:
		case SIMPLE: //Blinn-Phong
		{
			shader = mode == DrawingMode::FULL ? materialShaders[0] : materialShaders[2];
			shader->Use();
			shader->SetUniform("MVP", viewProjection*M);
			shader->SetUniform("M", M);
			shader->SetUniform("N", glm::mat3(glm::transpose(glm::inverse(M))));
			shader->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view*M))));
			shader->SetUniform("clipPlane", clipPlane);
			shader->SetUniform("eyePos", eyePos);
			shader->SetUniform("viewDir", viewDir);
			shader->SetUniform("specularStrength", l.params[0]);
			shader->SetUniform("shininess", l.params[1]);
            shader->SetUniform("reflectivity", l.reflectivity);
			shader->SetUniform("tex", TEX_BASE);
            
			glActiveTexture(GL_TEXTURE0 + TEX_BASE);
			
			if(l.textures.size() > 0)
			{
				glBindTexture(GL_TEXTURE_2D, l.textures[0]);
				shader->SetUniform("color", glm::vec4(l.color, 1.f));
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, 0);
				shader->SetUniform("color", glm::vec4(l.color, 0.f));			
			}
		}
			break;
			
		case PHYSICAL: //Cook-Torrance
		{
			shader = mode == DrawingMode::FULL ? materialShaders[1] : materialShaders[3];
			shader->Use();
			shader->SetUniform("MVP", viewProjection*M);
			shader->SetUniform("M", M);
			shader->SetUniform("N", glm::mat3(glm::transpose(glm::inverse(M))));
			shader->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view*M))));
			shader->SetUniform("clipPlane", clipPlane);
			shader->SetUniform("eyePos", eyePos);
			shader->SetUniform("viewDir", viewDir);
			shader->SetUniform("roughness", l.params[0]);
			shader->SetUniform("metallic", l.params[1]);
            shader->SetUniform("reflectivity", l.reflectivity);
			shader->SetUniform("tex", TEX_BASE);
			
			glActiveTexture(GL_TEXTURE0 + TEX_BASE);
			
			if(l.textures.size() > 0)
			{
				glBindTexture(GL_TEXTURE_2D, l.textures[0]);
				shader->SetUniform("color", glm::vec4(l.color, 1.f));
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, 0);
				shader->SetUniform("color", glm::vec4(l.color, 0.f));			
			}
		}
			break;
	}
	
	if(mode == DrawingMode::UNDERWATER)
	{
        Ocean* ocean = SimulationApp::getApp()->getSimulationManager()->getOcean();
        shader->SetUniform("turbidity", ocean->getOpenGLOcean()->getTurbidity());
		shader->SetUniform("lightAbsorption", ocean->getOpenGLOcean()->getLightAbsorption());
        
        /*if(SimulationApp::getApp()->getSimulationManager()->getOcean()->getForcefieldType() == ForcefieldType::FORCEFIELD_POOL)
        {
            Pool* pool = (Pool*)SimulationApp::getApp()->getSimulationManager()->getOcean();
            shader->SetUniform("lightAbsorption", pool->getOpenGLPool()->getLightAbsorptionCoeff());
			shader->SetUniform("turbidity", pool->getOpenGLPool()->getTurbidity());
        }*/
	}
	
	SetupLights(shader);
}

void OpenGLContent::UseStandardLook(const glm::mat4& M)
{
	GLSLShader* shader = materialShaders[0];
	shader->Use();
	shader->SetUniform("MVP", viewProjection*M);
	shader->SetUniform("M", M);
	shader->SetUniform("N", glm::mat3(glm::transpose(glm::inverse(M))));
	shader->SetUniform("clipPlane", clipPlane);
	shader->SetUniform("eyePos", eyePos);
	shader->SetUniform("viewDir", viewDir);
	shader->SetUniform("color", glm::vec4(0.5f,0.5f,0.5f,0.f));
	shader->SetUniform("shininess", 0.5f);
	shader->SetUniform("specularStrength", 0.1f);
    shader->SetUniform("reflectivity", 0.f);
	shader->SetUniform("tex", TEX_BASE);
	
	glActiveTexture(GL_TEXTURE0 + TEX_BASE);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	SetupLights(shader);
}

unsigned int OpenGLContent::BuildObject(Mesh* mesh)
{
	Object obj;
	obj.mesh = mesh;
	
	glGenVertexArrays(1, &obj.vao);
	glGenBuffers(1, &obj.vboVertex);
	glGenBuffers(1, &obj.vboIndex);
	
	glBindVertexArray(obj.vao);	
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Face)*mesh->faces.size(), &mesh->faces[0].vertexID[0], GL_STATIC_DRAW);
	glBindVertexArray(0);
	
	objects.push_back(obj);
	return (unsigned int)objects.size()-1;
}

unsigned int OpenGLContent::CreateSimpleLook(glm::vec3 rgbColor, GLfloat specular, GLfloat shininess, GLfloat reflectivity, std::string textureName)
{
    Look look;
	look.type = LookType::SIMPLE;
    look.color = rgbColor;
    look.reflectivity = reflectivity;
	look.params.push_back(specular);
	look.params.push_back(shininess);
    
	if(textureName != "") 
		look.textures.push_back(LoadTexture(textureName));
    
	looks.push_back(look);
	
	return (unsigned int)looks.size()-1;
}

unsigned int OpenGLContent::CreatePhysicalLook(glm::vec3 rgbColor, GLfloat roughness, GLfloat metalness, GLfloat reflectivity, std::string textureName)
{
	Look look;
	look.type = LookType::PHYSICAL;
	look.color = rgbColor;
    look.reflectivity = reflectivity;
	look.params.push_back(roughness);
	look.params.push_back(metalness);
	
	if(textureName != "")
		look.textures.push_back(LoadTexture(textureName));
		
	looks.push_back(look);
	
	return (unsigned int)looks.size()-1;
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
	
//Static methods
GLuint OpenGLContent::LoadTexture(std::string filename)
{
    int width, height, channels;
    GLuint texture;
    
    // Allocate image; fail out on error
    cInfo("Loading texture from: %s", filename.c_str());
    
    unsigned char* dataBuffer = stbi_load(filename.c_str(), &width, &height, &channels, 3);
    if(dataBuffer == NULL)
    {
        cError("Failed to load texture!");
        return -1;
    }
    
    GLfloat maxAniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    
    // Allocate an OpenGL texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Upload texture to memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, dataBuffer);
    // Set certain properties of texture
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
    // Wrap texture around
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Release internal buffer
    stbi_image_free(dataBuffer);
    
    return texture;
}

GLuint OpenGLContent::LoadInternalTexture(std::string filename)
{
    return LoadTexture(GetDataPath() + filename);
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
    vt.uv.x = 0;
    vt.uv.y = 0;
    mesh->vertices.push_back(vt);
    
    vt.pos.x = halfExtents;
    vt.uv.x = halfExtents*2.f;
    mesh->vertices.push_back(vt);
        
    vt.pos.y = halfExtents;
    vt.uv.y = halfExtents*2.f;
    mesh->vertices.push_back(vt);
        
    vt.pos.x = -halfExtents;
    vt.uv.x = 0;
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

Mesh* OpenGLContent::BuildBox(glm::vec3 halfExtents, unsigned int subdivisions)
{
    //Build mesh
    Mesh* mesh = new Mesh();
    Face f;
    Vertex vt;
	mesh->hasUVs = false;
	
    /////VERTICES
	glm::vec3 v1(-halfExtents.x, -halfExtents.y, -halfExtents.z);
    glm::vec3 v2(-halfExtents.x, halfExtents.y, -halfExtents.z);
    glm::vec3 v3(halfExtents.x, halfExtents.y, -halfExtents.z);
    glm::vec3 v4(halfExtents.x, -halfExtents.y, -halfExtents.z);
    glm::vec3 v5(halfExtents.x, halfExtents.y, halfExtents.z);
    glm::vec3 v6(halfExtents.x, -halfExtents.y, halfExtents.z);
	glm::vec3 v7(-halfExtents.x, -halfExtents.y, halfExtents.z);
    glm::vec3 v8(-halfExtents.x, halfExtents.y, halfExtents.z);
    
    /////FRONT
    //normal
	vt.normal = glm::vec3(0,0,-1.f);
    //vertices
    vt.pos = v1;
    vt.uv = glm::vec2(1.f/3.f, 1.f/3.f);
	mesh->vertices.push_back(vt); //0
	vt.pos = v2;
    vt.uv = glm::vec2(1.f/3.f, 2.f/3.f);
	mesh->vertices.push_back(vt); //1
	vt.pos = v3;
    vt.uv = glm::vec2(2.f/3.f, 2.f/3.f);
	mesh->vertices.push_back(vt); //2
	vt.pos = v4;
    vt.uv = glm::vec2(2.f/3.f, 1.f/3.f);
	mesh->vertices.push_back(vt); //3
	//faces
	f.vertexID[0] = 0;
	f.vertexID[1] = 1;
	f.vertexID[2] = 2;
	mesh->faces.push_back(f);
	f.vertexID[1] = 2;
	f.vertexID[2] = 3;
    mesh->faces.push_back(f);
    
    /////LEFT
    //normal
	vt.normal =  glm::vec3(1.f,0,0);
    //vertices
	vt.pos = v4;
    vt.uv = glm::vec2(0.f, 1.f/3.f);
	mesh->vertices.push_back(vt); //4
	vt.pos = v3;
    vt.uv = glm::vec2(0.f, 2.f/3.f);
	mesh->vertices.push_back(vt); //5
	vt.pos = v5;
    vt.uv = glm::vec2(1.f/3.f, 2.f/3.f);
	mesh->vertices.push_back(vt); //6
	vt.pos = v6;
    vt.uv = glm::vec2(1.f/3.f, 1.f/3.f);
	mesh->vertices.push_back(vt); //7
	//faces
	f.vertexID[0] = 4;
	f.vertexID[1] = 5;
	f.vertexID[2] = 6;
	mesh->faces.push_back(f);
	f.vertexID[1] = 6;
	f.vertexID[2] = 7;
	mesh->faces.push_back(f);
	
    /////RIGHT
    //normal
	vt.normal = glm::vec3(-1.f,0,0);
	//vertices
	vt.pos = v7;
    vt.uv = glm::vec2(2.f/3.f, 1.f/3.f);
	mesh->vertices.push_back(vt); //8
	vt.pos = v8;
    vt.uv = glm::vec2(2.f/3.f, 2.f/3.f);
	mesh->vertices.push_back(vt); //9
	vt.pos = v2;
    vt.uv = glm::vec2(1.f, 2.f/3.f);
	mesh->vertices.push_back(vt); //10
	vt.pos = v1;
    vt.uv = glm::vec2(1.f, 1.f/3.f);
	mesh->vertices.push_back(vt); //11
	//faces
	f.vertexID[0] = 8;
	f.vertexID[1] = 9;
	f.vertexID[2] = 10;
	mesh->faces.push_back(f);
	f.vertexID[1] = 10;
	f.vertexID[2] = 11;
	mesh->faces.push_back(f);
	
    /////BACK
    //normal
    vt.normal = glm::vec3(0,0,1.f);
	//vertices
	vt.pos = v6;
    vt.uv = glm::vec2(0.f, 0.0f);
	mesh->vertices.push_back(vt); //12
	vt.pos = v5;
    vt.uv = glm::vec2(0.f, 1.f/3.f);
	mesh->vertices.push_back(vt); //13
	vt.pos = v8;
    vt.uv = glm::vec2(1.f/3.f, 1.f/3.f);
	mesh->vertices.push_back(vt); //14
	vt.pos = v7;
    vt.uv = glm::vec2(1.f/3.f, 0.f);
	mesh->vertices.push_back(vt); //15
	//faces
	f.vertexID[0] = 12;
	f.vertexID[1] = 13;
	f.vertexID[2] = 14;
	mesh->faces.push_back(f);
	f.vertexID[1] = 14;
	f.vertexID[2] = 15;
	mesh->faces.push_back(f);
	
    //////TOP
    //normal
    vt.normal = glm::vec3(0,1.f,0);
	//vertices
	vt.pos = v5;
    vt.uv = glm::vec2(1.f/3.f, 2.f/3.f);
	mesh->vertices.push_back(vt); //16
	vt.pos = v3;
    vt.uv = glm::vec2(1.f/3.f, 1.f);
	mesh->vertices.push_back(vt); //17
	vt.pos = v2;
    vt.uv = glm::vec2(2.f/3.f, 1.f);
	mesh->vertices.push_back(vt); //18
	vt.pos = v8;
    vt.uv = glm::vec2(2.f/3.f, 2.f/3.f);
	mesh->vertices.push_back(vt); //19
	//faces
	f.vertexID[0] = 16;
	f.vertexID[1] = 17;
	f.vertexID[2] = 18;
	mesh->faces.push_back(f);
	f.vertexID[1] = 18;
	f.vertexID[2] = 19;
	mesh->faces.push_back(f);
	
    /////BOTTOM
    //normal
    vt.normal = glm::vec3(0,-1.f,0);
    //vertices
	vt.pos = v4;
    vt.uv = glm::vec2(1.f/3.f, 0.f);
	mesh->vertices.push_back(vt); //20
	vt.pos = v6;
    vt.uv = glm::vec2(1.f/3.f, 1.f/3.f);
	mesh->vertices.push_back(vt); //21
	vt.pos = v7;
    vt.uv = glm::vec2(2.f/3.f, 1.f/3.f);
	mesh->vertices.push_back(vt); //22
	vt.pos = v1;
    vt.uv = glm::vec2(2.f/3.f, 0.f);
	mesh->vertices.push_back(vt); //23
	//faces
	f.vertexID[0] = 20;
	f.vertexID[1] = 21;
	f.vertexID[2] = 22;
	mesh->faces.push_back(f);
	f.vertexID[1] = 22;
	f.vertexID[2] = 23;
	mesh->faces.push_back(f);
	
	for(unsigned int i=0; i<subdivisions; ++i)
		Subdivide(mesh);
		
	SmoothNormals(mesh);
	
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
	
	mesh->hasUVs = false;
	vt.uv = glm::vec2(0,0);
	
	//SIDE
    //Side vertices
    for(unsigned int i=0; i<slices; ++i)
    {
		vt.normal = glm::vec3(sin(i/(GLfloat)slices*M_PI*2.0), -cos(i/(GLfloat)slices*M_PI*2.0), 0.0);
		vt.pos = glm::vec3(sin(i/(GLfloat)slices*M_PI*2.0)*radius, -cos(i/(GLfloat)slices*M_PI*2.0)*radius, halfHeight);
        mesh->vertices.push_back(vt);
		vt.pos.z = -halfHeight;
        mesh->vertices.push_back(vt);
    }
    
    //Side faces
    for(unsigned int i=0; i<slices-1; ++i)
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
    
	//Last side
    int i = slices-1;
	vt.normal = glm::vec3(sin((GLfloat)(slices-1)/(GLfloat)slices*M_PI*2.0), -cos((GLfloat)(slices-1)/(GLfloat)slices*M_PI*2.0), 0.0);
    f.vertexID[0] = i*2;
    f.vertexID[1] = i*2+1;
    f.vertexID[2] = 0;
    mesh->faces.push_back(f);
    f.vertexID[0] = i*2+1;
    f.vertexID[1] = 1;
    f.vertexID[2] = 0;
    mesh->faces.push_back(f);
    
    //TOP CAP
    vt.normal = glm::vec3(0,0,1.f);
	vt.pos = glm::vec3(0,0,halfHeight);
	mesh->vertices.push_back(vt);
	unsigned int centerIndex = (unsigned int)mesh->vertices.size()-1;
	//vertices
	for(unsigned int i=0; i<slices; ++i)
    {
		vt.pos = glm::vec3(sin(i/(GLfloat)slices*M_PI*2.0)*radius, -cos(i/(GLfloat)slices*M_PI*2.0)*radius, halfHeight);
        mesh->vertices.push_back(vt);
    }
	//faces
	for(unsigned int i=0; i<slices-1; ++i)
	{
		f.vertexID[0] = centerIndex;
		f.vertexID[1] = centerIndex + i + 1;
		f.vertexID[2] = centerIndex + i + 2;
		mesh->faces.push_back(f);
	}
	//last face
	f.vertexID[0] = centerIndex;
	f.vertexID[1] = centerIndex + slices-1 + 1;
	f.vertexID[2] = centerIndex + 1;
	mesh->faces.push_back(f);
	
	//BOTTOM CAP
    vt.normal = glm::vec3(0,0,-1.f);
	vt.pos = glm::vec3(0,0,-halfHeight);
	mesh->vertices.push_back(vt);
	centerIndex = (unsigned int)mesh->vertices.size()-1;
	//vertices
	for(unsigned int i=0; i<slices; ++i)
    {
		vt.pos = glm::vec3(sin(i/(GLfloat)slices*M_PI*2.0)*radius, -cos(i/(GLfloat)slices*M_PI*2.0)*radius, -halfHeight);
        mesh->vertices.push_back(vt);
    }
	//faces
	for(unsigned int i=0; i<slices-1; ++i)
	{
		f.vertexID[0] = centerIndex;
		f.vertexID[1] = centerIndex+ i + 2;
		f.vertexID[2] = centerIndex+ i + 1;
		mesh->faces.push_back(f);
	}
	//last face
	f.vertexID[0] = centerIndex;
	f.vertexID[1] = centerIndex + 1;
	f.vertexID[2] = centerIndex + slices-1 + 1;
	mesh->faces.push_back(f);
	
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

	for(unsigned int i=0; i<majorSlices; ++i)
    {
        GLfloat alpha = i/(GLfloat)majorSlices*M_PI*2.f;
        
        for(unsigned int h=0; h<minorSlices; ++h)
        {
            GLfloat ry = cosf(h/(GLfloat)minorSlices*M_PI*2.f) * minorRadius;
            GLfloat rx = sinf(h/(GLfloat)minorSlices*M_PI*2.f) * minorRadius;
            
			vt.pos = glm::vec3((rx + majorRadius)*cosf(alpha), ry, (rx + majorRadius)*sinf(alpha));
			vt.normal = glm::vec3(sinf(h/(GLfloat)minorSlices*M_PI*2.f)*cosf(alpha), cosf(h/(GLfloat)minorSlices*M_PI*2.f), sinf(h/(GLfloat)minorSlices*M_PI*2.f)*sinf(alpha));
			vt.uv = glm::vec2(4.f * h/(GLfloat)minorSlices, (GLfloat)minorSlices * i/(GLfloat)majorSlices);
			mesh->vertices.push_back(vt);
			
			if(h<minorSlices-1 && i<majorSlices-1)
			{
				f.vertexID[0] = i*minorSlices + h;
				f.vertexID[1] = (i+1)*minorSlices + h;
				f.vertexID[2] = i*minorSlices + (h+1);
				mesh->faces.push_back(f);
				f.vertexID[0] = (i+1)*minorSlices + h;
				f.vertexID[1] = (i+1)*minorSlices + (h+1);
				f.vertexID[2] = i*minorSlices + (h+1);
				mesh->faces.push_back(f);
			}
			else if(i<majorSlices-1) //Last patch in the middle slice of torus
			{
				f.vertexID[0] = i*minorSlices + h;
				f.vertexID[1] = (i+1)*minorSlices + h;
				f.vertexID[2] = i*minorSlices;
				mesh->faces.push_back(f);
				f.vertexID[0] = (i+1)*minorSlices + h;
				f.vertexID[1] = (i+1)*minorSlices;
				f.vertexID[2] = i*minorSlices;
				mesh->faces.push_back(f);
			}
			else if(h<minorSlices-1) //Middle patch in the last slice of torus
			{
				f.vertexID[0] = i*minorSlices + h;
				f.vertexID[1] = h;
				f.vertexID[2] = i*minorSlices + (h+1);
				mesh->faces.push_back(f);
				f.vertexID[0] = h;
				f.vertexID[1] = h+1;
				f.vertexID[2] = i*minorSlices + (h+1);
				mesh->faces.push_back(f);
			}
			else //Last patch in the last slice of torus
			{
				f.vertexID[0] = i*minorSlices + h;
				f.vertexID[1] = h;
				f.vertexID[2] = i*minorSlices;
				mesh->faces.push_back(f);
				f.vertexID[0] = h;
				f.vertexID[1] = 0;
				f.vertexID[2] = i*minorSlices;
				mesh->faces.push_back(f);
			}
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
    if(mesh != NULL && smooth)
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
        glm::vec3 thisN = mesh->computeFaceNormal(i);
        
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
                    glm::vec3 thatN = mesh->computeFaceNormal(j);
                    
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
	
	mesh->faces = newFaces;
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
