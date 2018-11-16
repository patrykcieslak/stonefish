//
//  OpenGLAtmosphere.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 22/07/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLAtmosphere.h"

#include <fstream>
#include <sstream>
#include "graphics/Console.h"
#include "utils/MathsUtil.hpp"
#include "utils/SystemUtil.hpp"

//Atmosphere textures allocation parameters
constexpr int TRANSMITTANCE_TEXTURE_WIDTH = 256;
constexpr int TRANSMITTANCE_TEXTURE_HEIGHT = 64;

constexpr int SCATTERING_TEXTURE_R_SIZE = 32;
constexpr int SCATTERING_TEXTURE_MU_SIZE = 128;
constexpr int SCATTERING_TEXTURE_MU_S_SIZE = 32;
constexpr int SCATTERING_TEXTURE_NU_SIZE = 8;

constexpr int SCATTERING_TEXTURE_WIDTH = SCATTERING_TEXTURE_NU_SIZE * SCATTERING_TEXTURE_MU_S_SIZE;
constexpr int SCATTERING_TEXTURE_HEIGHT = SCATTERING_TEXTURE_MU_SIZE;
constexpr int SCATTERING_TEXTURE_DEPTH = SCATTERING_TEXTURE_R_SIZE;

constexpr int IRRADIANCE_TEXTURE_WIDTH = 64;
constexpr int IRRADIANCE_TEXTURE_HEIGHT = 16;

constexpr double MAX_LUMINOUS_EFFICACY = 683.0; //The conversion factor between watts and lumens.
constexpr double kLambdaR = 680.0;
constexpr double kLambdaG = 550.0;
constexpr double kLambdaB = 440.0;
constexpr double kLengthUnitInMeters = 1.0; //Length unit used in OpenGL

OpenGLAtmosphere* OpenGLAtmosphere::instance = NULL;

OpenGLAtmosphere* OpenGLAtmosphere::getInstance()
{
    if(instance == NULL)
        instance = new OpenGLAtmosphere();

    return instance;
}

OpenGLAtmosphere::OpenGLAtmosphere()
{
    //Prepare for rendering
    for(unsigned short i=0; i<AtmosphereTextures::TEXTURE_COUNT; ++i) textures[i] = 0;
    skySunShader = NULL;
    atmosphereAPI = 0;
    atmBottomRadius = 0.f;
    whitePoint = glm::vec3(1.f);

    //Init shadow baking
    sunShadowmapShader = NULL;
    sunShadowmapArray = 0;
    sunDepthSampler = 0;
    sunShadowSampler = 0;
    sunShadowmapSplits = 4;
    sunShadowmapSize = 4096;
    sunShadowFBO = 0;
    sunDirection = glm::vec3(0,0,1.f);
    sunModelView = glm::mat4x4(0);
    sunShadowFrustum = NULL;
    sunShadowCPM = NULL;
    
    //Set standard sun position
    std::time_t rawTime;
    std::time(&rawTime);
    std::tm* hUtc;
    hUtc = std::gmtime(&rawTime);
    hUtc->tm_hour = 12;
    hUtc->tm_min = 0;
    hUtc->tm_sec = 0;
    SetSunPosition(19.945, 50.065, *hUtc);
}

void OpenGLAtmosphere::Init(RenderQuality quality, RenderQuality shadow)
{
    if(quality == RenderQuality::QUALITY_DISABLED)
        return;
    
    //Read base shader code
    std::ifstream defsCode(GetShaderPath() + "atmosphereDef.glsl");
    std::stringstream defsBuffer;
    defsBuffer << defsCode.rdbuf();
    glslDefinitions = defsBuffer.str();

    std::ifstream funcsCode(GetShaderPath() + "atmosphereFunc.glsl");
    std::stringstream funcsBuffer;
    funcsBuffer << funcsCode.rdbuf();
    glslFunctions = funcsBuffer.str();

    //Compute lookup textures
    switch(quality)
    {
        case RenderQuality::QUALITY_LOW:
            nPrecomputedWavelengths = 5;
            nScatteringOrders = 2;
            break;
        
        default:
        case RenderQuality::QUALITY_MEDIUM:
            nPrecomputedWavelengths = 15;
            nScatteringOrders = 4;
            break;
        
        case RenderQuality::QUALITY_HIGH:
            nPrecomputedWavelengths = 30;
            nScatteringOrders = 6;
    }
    
    Precompute();
    atmBottomRadius = 6360000.f;

    //Set shadow quality
    switch(shadow)
    {
        case RenderQuality::QUALITY_DISABLED:
            sunShadowmapSize = 0;
            sunShadowmapSplits = 0;
            break;
        
        case RenderQuality::QUALITY_LOW:
            sunShadowmapSize = 1024;
            sunShadowmapSplits = 4;
            break;
        
        case RenderQuality::QUALITY_MEDIUM:
            sunShadowmapSize = 2048;
            sunShadowmapSplits = 4;
            break;
        
        case RenderQuality::QUALITY_HIGH:
            sunShadowmapSize = 4096;
            sunShadowmapSplits = 4;
            break;
    }
    
    //Permanently bind atmosphere textures
    glActiveTexture(GL_TEXTURE0 + TEX_ATM_TRANSMITTANCE);
    glBindTexture(GL_TEXTURE_2D, textures[AtmosphereTextures::TRANSMITTANCE]);
    glActiveTexture(GL_TEXTURE0 + TEX_ATM_SCATTERING);
    glBindTexture(GL_TEXTURE_3D, textures[AtmosphereTextures::SCATTERING]);
    glActiveTexture(GL_TEXTURE0 + TEX_ATM_IRRADIANCE);
    glBindTexture(GL_TEXTURE_2D, textures[AtmosphereTextures::IRRADIANCE]);
   
    //Build atmosphere API shader
    GLint compiled;
    std::string additionalDefs = nPrecomputedWavelengths > 3 ? "" : "#define RADIANCE_API_ENABLED\n";
    additionalDefs = EarthsAtmosphere(glm::dvec3(kLambdaR, kLambdaG, kLambdaB)) + additionalDefs;
    atmosphereAPI = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "atmosphereApi.glsl", additionalDefs, &compiled);

    //Build rendering shaders
    std::vector<GLuint> compiledShaders;
    compiledShaders.push_back(atmosphereAPI);
    skySunShader = new GLSLShader(compiledShaders, "atmosphereSkySun.frag", "atmosphereSkySun.vert");
    skySunShader->AddUniform("transmittance_texture", ParameterType::INT);
    skySunShader->AddUniform("scattering_texture", ParameterType::INT);
    skySunShader->AddUniform("single_mie_scattering_texture", ParameterType::INT);
    skySunShader->AddUniform("eyePos", ParameterType::VEC3);
    skySunShader->AddUniform("sunDir", ParameterType::VEC3);
    skySunShader->AddUniform("invView", ParameterType::MAT3);
    skySunShader->AddUniform("invProj", ParameterType::MAT4);
    skySunShader->AddUniform("viewport", ParameterType::VEC2);
    skySunShader->AddUniform("whitePoint", ParameterType::VEC3);
    skySunShader->AddUniform("cosSunSize", ParameterType::FLOAT);

    //Initialize shadows
    if(shadow > RenderQuality::QUALITY_DISABLED)
    {
        sunShadowmapShader = new GLSLShader("sunCSM.frag");
        sunShadowmapShader->AddUniform("shadowmapArray", ParameterType::INT);
        sunShadowmapShader->AddUniform("shadowmapLayer", ParameterType::FLOAT);
        
        //Generate shadowmap array
        glActiveTexture(GL_TEXTURE0 + TEX_BASE);
        glGenTextures(1, &sunShadowmapArray);
        glBindTexture(GL_TEXTURE_2D_ARRAY, sunShadowmapArray);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32F, sunShadowmapSize, sunShadowmapSize, sunShadowmapSplits);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        
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
        glBindFramebuffer(GL_FRAMEBUFFER, sunShadowFBO);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, sunShadowmapArray, 0, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Sun shadow FBO initialization failed!");
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        sunShadowFrustum = new ViewFrustum[sunShadowmapSplits];
        sunShadowCPM = new glm::mat4x4[sunShadowmapSplits];
    }
    else
    {
        //Generate shadowmap array
        glActiveTexture(GL_TEXTURE0 + TEX_BASE);
        glGenTextures(1, &sunShadowmapArray);
        glBindTexture(GL_TEXTURE_2D_ARRAY, sunShadowmapArray);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32F, 1, 1, 1);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }
}

OpenGLAtmosphere::~OpenGLAtmosphere()
{
    for(unsigned short i=0; i< AtmosphereTextures::TEXTURE_COUNT; ++i)
        if(textures[i] != 0) glDeleteTextures(1, &textures[i]);

    if(skySunShader != NULL) delete skySunShader;
    if(atmosphereAPI > 0) glDeleteShader(atmosphereAPI);

    if(sunShadowmapArray != 0) glDeleteTextures(1, &sunShadowmapArray);
    
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
    sunDirection = glm::normalize(glm::vec3(cos(sunAzimuthAngle) * sin(sunZenithAngle), sin(sunAzimuthAngle) * sin(sunZenithAngle), cos(sunZenithAngle)));

    //Build sun modelview matrix
    glm::vec3 up(0,0,1.f);
    glm::vec3 right = glm::cross(sunDirection, up);
    right = glm::normalize(right);
    up = glm::normalize(glm::cross(right, sunDirection));
    sunModelView = glm::lookAt(glm::vec3(0,0,0), -sunDirection, up) * glm::mat4(1.f);

#ifdef DEBUG
    cInfo("Set sun position --> Azimuth: %1.3f  Elevation: %1.3f", azimuthDeg, elevationDeg);
#endif
}

int OpenGLAtmosphere::JulianDay(std::tm& tm)
{
    int m = tm.tm_mon + 1;
    int y = tm.tm_year + 1900;
    int d = tm.tm_mday;

    double X = (m + 9) / 12.0;
    int A = 4716 + y + (int)trunc(X);
    double Y = 275 * m / 9.0;
    double V = 7 * A / 4.0;
    double B = 1729279.5 + 367 * y + trunc(Y) - trunc(V) + d;
    double Q = (A + 83) / 100.0;
    double W = 3 * (trunc(Q) + 1) / 4.0;
    return B + 38 - (int)trunc(W);
}

void OpenGLAtmosphere::SetSunPosition(double longitudeDeg, double latitudeDeg, std::tm& utc)
{
    double latitude = latitudeDeg/180 * M_PI;
    double longitude = longitudeDeg/180.0 * M_PI;
    double meridian = 0.0; //GMT
    double decimalHours = (double)utc.tm_hour + (double)utc.tm_min/60.0 + (double)utc.tm_sec/60.0;
    std::tm utc0 = utc;
    utc0.tm_mday = 1; //1
    utc0.tm_mon = 0; //January
    int J = JulianDay(utc) - JulianDay(utc0);
    double solarTime = decimalHours + 0.17*sin(4.0*M_PI*(J - 80)/373.0) - 0.129*sin(2.0*M_PI*(J - 8)/355.0) + 12.0*(meridian - longitude)/M_PI;
    double declination = 0.4093*sin(2.0*M_PI*(J - 81)/368.0);
    double elevation = asin(sin(latitude)*sin(declination) - cos(latitude)*cos(declination)*cos(M_PI*solarTime/12.0));
    double azimuth = atan2(sin(M_PI*solarTime/12.0), cos(M_PI*solarTime/12.0)*sin(latitude) - tan(declination)*cos(latitude));
    SetSunPosition(azimuth/M_PI*180.0, elevation/M_PI*180.0);
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

GLfloat OpenGLAtmosphere::getAtmosphereBottomRadius()
{
    return atmBottomRadius;
}

GLuint OpenGLAtmosphere::getAtmosphereAPI()
{
    return atmosphereAPI;
}

GLuint OpenGLAtmosphere::getAtmosphereTexture(AtmosphereTextures id)
{
    return textures[id];
}

void OpenGLAtmosphere::DrawSkyAndSun(const OpenGLCamera* view)
{
    glm::mat4 viewMatrix = view->GetViewMatrix();
    glm::mat4 projection = view->GetProjectionMatrix();
    GLint* viewport = view->GetViewport();

    skySunShader->Use();
    skySunShader->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    skySunShader->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    skySunShader->SetUniform("single_mie_scattering_texture", TEX_ATM_SCATTERING);
    skySunShader->SetUniform("eyePos", view->GetEyePosition());
    skySunShader->SetUniform("sunDir", GetSunDirection());
    skySunShader->SetUniform("invProj", glm::inverse(projection));
    skySunShader->SetUniform("invView", glm::mat3(glm::inverse(viewMatrix)));
    skySunShader->SetUniform("viewport", glm::vec2(viewport[2], viewport[3]));
    skySunShader->SetUniform("whitePoint", whitePoint);
    skySunShader->SetUniform("cosSunSize", (GLfloat)cosf(0.00935f/2.f));
    OpenGLContent::getInstance()->DrawSAQ();
    glUseProgram(0);
}

void OpenGLAtmosphere::BakeShadowmaps(OpenGLPipeline* pipe, OpenGLCamera* view)
{
    if(sunShadowmapSize == 0)
        return;
    
    //Pre-set splits
    for(unsigned int i = 0; i < sunShadowmapSplits; ++i) {
        sunShadowFrustum[i].fov = view->GetFOVY() + 0.2f; //avoid artifacts in the borders
        GLint* viewport = view->GetViewport();
        sunShadowFrustum[i].ratio = (GLfloat)viewport[2]/(GLfloat)viewport[3];
        delete [] viewport;
    }

    //Compute the z-distances for each split as seen in camera space
    UpdateSplitDist(view->GetNearClip(), view->GetFarClip());

    //Render maps
    glCullFace(GL_FRONT); //GL_FRONT -> no shadow acne but problems with filtering

    glBindFramebuffer(GL_FRAMEBUFFER, sunShadowFBO);
    glViewport(0, 0, sunShadowmapSize, sunShadowmapSize);

    glm::vec3 camPos = view->GetEyePosition();
    glm::vec3 camDir = view->GetLookingDirection();
    glm::vec3 camUp = view->GetUpDirection();

    // for all shadow splits
    for(unsigned int i = 0; i < sunShadowmapSplits; ++i) {
        //Compute the camera frustum slice boundary points in world space
        UpdateFrustumCorners(sunShadowFrustum[i], camPos, camDir, camUp);

        //Adjust the view frustum of the light, so that it encloses the camera frustum slice fully.
        //note that this function sets the projection matrix as it sees best fit
        glm::mat4 cp = BuildCropProjMatrix(sunShadowFrustum[i]);
        sunShadowCPM[i] =  cp * sunModelView;

        OpenGLContent::getInstance()->SetProjectionMatrix(cp);
        OpenGLContent::getInstance()->SetViewMatrix(sunModelView);
        //Draw current depth map
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, sunShadowmapArray, 0, i);
        glClear(GL_DEPTH_BUFFER_BIT);
        pipe->DrawObjects();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glCullFace(GL_BACK);
}

//Computes the near and far distances for every frustum slice in camera eye space
void OpenGLAtmosphere::UpdateSplitDist(GLfloat nd, GLfloat fd)
{
    GLfloat lambda = 0.95f;
    GLfloat ratio = fd/nd;
    sunShadowFrustum[0].near = nd;

    for(unsigned int i = 1; i < sunShadowmapSplits; ++i) {
        GLfloat si = i / (GLfloat)sunShadowmapSplits;

        sunShadowFrustum[i].near = lambda*(nd*powf(ratio, si)) + (1.f-lambda)*(nd + (fd - nd)*si);
        sunShadowFrustum[i-1].far = sunShadowFrustum[i].near * 1.005f;
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
    btVector3 aabbMin;
    btVector3 aabbMax;
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

void OpenGLAtmosphere::SetupMaterialShader(GLSLShader* shader)
{
    //Calculate shadow splits
    glm::mat4 bias(0.5f, 0.f, 0.f, 0.f,
                   0.f, 0.5f, 0.f, 0.f,
                   0.f, 0.f, 0.5f, 0.f,
                   0.5f, 0.5f, 0.5f, 1.f);
    GLfloat frustumFar[4];
    GLfloat frustumNear[4];
    glm::mat4 lightClipSpace[4];

    //For every inactive split
    /*for(unsigned int i = sunShadowmapSplits; i<4; ++i) {
        frustumNear[i] = 0;
        frustumFar[i] = 0;
        lightClipSpace[i] = glm::mat4();
    }*/

    //For every active split
    for(unsigned int i = 0; i < 4; ++i) {
        frustumNear[i] = sunShadowFrustum[i].near;
        frustumFar[i] = sunShadowFrustum[i].far;
        lightClipSpace[i] = (bias * sunShadowCPM[i]); // compute a matrix that transforms from world space to light clip space
    }

    shader->SetUniform("planetRadius", atmBottomRadius+1.f);
    shader->SetUniform("sunDirection", GetSunDirection());
    shader->SetUniform("sunClipSpace[0]", lightClipSpace[0]);
    shader->SetUniform("sunClipSpace[1]", lightClipSpace[1]);
    shader->SetUniform("sunClipSpace[2]", lightClipSpace[2]);
    shader->SetUniform("sunClipSpace[3]", lightClipSpace[3]);
    shader->SetUniform("sunFrustumNear[0]", frustumNear[0]);
    shader->SetUniform("sunFrustumNear[1]", frustumNear[1]);
    shader->SetUniform("sunFrustumNear[2]", frustumNear[2]);
    shader->SetUniform("sunFrustumNear[3]", frustumNear[3]);
    shader->SetUniform("sunFrustumFar[0]", frustumFar[0]);
    shader->SetUniform("sunFrustumFar[1]", frustumFar[1]);
    shader->SetUniform("sunFrustumFar[2]", frustumFar[2]);
    shader->SetUniform("sunFrustumFar[3]", frustumFar[3]);
    shader->SetUniform("sunDepthMap", TEX_SUN_DEPTH);
    shader->SetUniform("sunShadowMap", TEX_SUN_SHADOW);
    shader->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    shader->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    shader->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);
    shader->SetUniform("whitePoint", whitePoint);

    //Bind textures and samplers
    glActiveTexture(GL_TEXTURE0 + TEX_SUN_SHADOW);
    glBindTexture(GL_TEXTURE_2D_ARRAY, sunShadowmapArray);
    glBindSampler(TEX_SUN_SHADOW, sunShadowSampler);

    glActiveTexture(GL_TEXTURE0 + TEX_SUN_DEPTH);
    glBindTexture(GL_TEXTURE_2D_ARRAY, sunShadowmapArray);
    glBindSampler(TEX_SUN_DEPTH, sunDepthSampler);
}

void OpenGLAtmosphere::SetupOceanShader(GLSLShader* shader)
{
    shader->SetUniform("planetRadius", atmBottomRadius+1.f);
    shader->SetUniform("sunDirection", GetSunDirection());
    shader->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    shader->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    shader->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);
    shader->SetUniform("whitePoint", whitePoint);
    shader->SetUniform("cosSunSize", (GLfloat)cos(0.00935/2.0));
}

void OpenGLAtmosphere::Precompute()
{
    cInfo("Precomputing atmosphere...");
#ifdef DEBUG
    uint64_t start = GetTimeInMicroseconds();
#else
    GLSLShader::Silent();
#endif
    //Delete old textures when recomputing
    for(unsigned short i=0; i< AtmosphereTextures::TEXTURE_COUNT; ++i)
        if(textures[i] != 0) glDeleteTextures(1, &textures[i]);

    //Generate new empty textures to store the result of precomputation
    glGenTextures(AtmosphereTextures::TEXTURE_COUNT, textures);

    //Transmittance
    glBindTexture(GL_TEXTURE_2D, textures[AtmosphereTextures::TRANSMITTANCE]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL); //16F precision for the transmittance gives artifacts.

    //Scattering
    glBindTexture(GL_TEXTURE_3D, textures[AtmosphereTextures::SCATTERING]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, 0, GL_RGBA, GL_FLOAT, NULL);

    //Irradiance
    glBindTexture(GL_TEXTURE_2D, textures[AtmosphereTextures::IRRADIANCE]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);

    //Create temporary textures
    GLuint delta_irradiance_texture;
    glGenTextures(1, &delta_irradiance_texture);
    glBindTexture(GL_TEXTURE_2D, delta_irradiance_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);

    GLuint delta_rayleigh_scattering_texture;
    glGenTextures(1, &delta_rayleigh_scattering_texture);
    glBindTexture(GL_TEXTURE_3D, delta_rayleigh_scattering_texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, 0, GL_RGB, GL_FLOAT, NULL);

    GLuint delta_mie_scattering_texture;
    glGenTextures(1, &delta_mie_scattering_texture);
    glBindTexture(GL_TEXTURE_3D, delta_mie_scattering_texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, 0, GL_RGB, GL_FLOAT, NULL);

    GLuint delta_scattering_density_texture;
    glGenTextures(1, &delta_scattering_density_texture);
    glBindTexture(GL_TEXTURE_3D, delta_scattering_density_texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, 0, GL_RGB, GL_FLOAT, NULL);

    GLuint delta_multiple_scattering_texture = delta_rayleigh_scattering_texture;

    //Create temporary framebuffer
    GLuint fbo;
    GLuint vao;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // The actual precomputations depend on whether we want to store precomputed
    // irradiance or illuminance values.
    if(nPrecomputedWavelengths <= 3) {
        glm::dvec3 lambdas({kLambdaR, kLambdaG, kLambdaB});
        glm::dmat3 luminance_from_radiance({1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0});
        luminance_from_radiance = glm::transpose(luminance_from_radiance);

        PrecomputePass(fbo, delta_irradiance_texture, delta_rayleigh_scattering_texture,
                       delta_mie_scattering_texture, delta_scattering_density_texture,
                       delta_multiple_scattering_texture, lambdas, luminance_from_radiance, false);
    } else {
        int num_iterations = (nPrecomputedWavelengths + 2) / 3;
        double dlambda = (kLambdaMax - kLambdaMin) / (3 * num_iterations);
        for (int i = 0; i < num_iterations; ++i) {
            glm::dvec3 lambdas(kLambdaMin + (3 * i + 0.5) * dlambda, kLambdaMin + (3 * i + 1.5) * dlambda, kLambdaMin + (3 * i + 2.5) * dlambda);

            auto coeff = [dlambda](double lambda, int component) {
                // Note that we don't include MAX_LUMINOUS_EFFICACY here, to avoid
                // artefacts due to too large values when using half precision on GPU.
                // We add this term back in kAtmosphereShader, via
                // SKY_SPECTRAL_RADIANCE_TO_LUMINANCE (see also the comments in the
                // Model constructor).
                double x = CieColorMatchingFunctionTableValue(lambda, 1);
                double y = CieColorMatchingFunctionTableValue(lambda, 2);
                double z = CieColorMatchingFunctionTableValue(lambda, 3);
                return static_cast<float>((XYZ_TO_SRGB[component * 3] * x + XYZ_TO_SRGB[component * 3 + 1] * y + XYZ_TO_SRGB[component * 3 + 2] * z) * dlambda);
            };

            glm::dmat3 luminance_from_radiance({coeff(lambdas[0], 0), coeff(lambdas[1], 0), coeff(lambdas[2], 0),
                                                coeff(lambdas[0], 1), coeff(lambdas[1], 1), coeff(lambdas[2], 1),
                                                coeff(lambdas[0], 2), coeff(lambdas[1], 2), coeff(lambdas[2], 2)
                                               });
            luminance_from_radiance = glm::transpose(luminance_from_radiance);

            PrecomputePass(fbo, delta_irradiance_texture, delta_rayleigh_scattering_texture, delta_mie_scattering_texture,
                           delta_scattering_density_texture, delta_multiple_scattering_texture,
                           lambdas, luminance_from_radiance, i > 0);
        }

        //After the above iterations, the transmittance texture contains the
        //transmittance for the 3 wavelengths used at the last iteration. But we
        //want the transmittance at kLambdaR, kLambdaG, kLambdaB instead, so we
        //must recompute it here for these 3 wavelengths:
        GLSLHeader header;
        header.useInFragment = true;
        header.useInVertex = header.useInTessCtrl = header.useInTessEval = header.useInGeometry = false;
        header.code = EarthsAtmosphere(glm::dvec3(kLambdaR, kLambdaG, kLambdaB));
        GLSLShader transmittanceShader(header, "atmosphereTransmittance.frag"); //No uniforms

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures[AtmosphereTextures::TRANSMITTANCE], 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glViewport(0, 0, TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
        transmittanceShader.Use();
        DrawBlendedSAQ({});
    }

    //Destroy temporary OpenGL resources
    glUseProgram(0);
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteVertexArrays(1, &vao);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &delta_scattering_density_texture);
    glDeleteTextures(1, &delta_mie_scattering_texture);
    glDeleteTextures(1, &delta_rayleigh_scattering_texture);
    glDeleteTextures(1, &delta_irradiance_texture);

#ifdef DEBUG
    int64_t stop = GetTimeInMicroseconds();
    std::cout << "Precomputed in " << std::to_string((stop-start)/1000) << "ms." << std::endl;
#else
    GLSLShader::Verbose();
#endif
}

void OpenGLAtmosphere::PrecomputePass(GLuint fbo, GLuint delta_irradiance_texture, GLuint delta_rayleigh_scattering_texture,
                                      GLuint delta_mie_scattering_texture, GLuint delta_scattering_density_texture,
                                      GLuint delta_multiple_scattering_texture, const glm::dvec3& lambdas, const glm::dmat3& luminance_from_radiance,
                                      bool blend)
{
    //Load precompute shaders
    GLSLHeader header;
    header.useInFragment = true;
    header.useInVertex = header.useInTessCtrl = header.useInTessEval = header.useInGeometry = false;
    header.code = EarthsAtmosphere(lambdas);

    GLSLShader transmittanceShader(header, "atmosphereTransmittance.frag"); //No uniforms
    GLSLShader directIrradianceShader(header, "atmosphereDirectIrradiance.frag");
    directIrradianceShader.AddUniform("texTransmittance", ParameterType::INT);
    GLSLShader singleScatteringShader(header, "atmosphereSingleScattering.frag", "", "atmosphere.geom");
    singleScatteringShader.AddUniform("texTransmittance", ParameterType::INT);
    singleScatteringShader.AddUniform("luminanceFromRadiance", ParameterType::MAT3);
    singleScatteringShader.AddUniform("layer", ParameterType::INT);
    GLSLShader scatteringDensityShader(header, "atmosphereScatteringDensity.frag", "", "atmosphere.geom");
    scatteringDensityShader.AddUniform("texTransmittance", ParameterType::INT);
    scatteringDensityShader.AddUniform("texSingleRayleighScattering", ParameterType::INT);
    scatteringDensityShader.AddUniform("texSingleMieScattering", ParameterType::INT);
    scatteringDensityShader.AddUniform("texMultipleScattering", ParameterType::INT);
    scatteringDensityShader.AddUniform("texIrradiance", ParameterType::INT);
    scatteringDensityShader.AddUniform("scatteringOrder", ParameterType::INT);
    scatteringDensityShader.AddUniform("layer", ParameterType::INT);
    GLSLShader indirectIrradianceShader(header, "atmosphereIndirectIrradiance.frag");
    indirectIrradianceShader.AddUniform("texSingleRayleighScattering", ParameterType::INT);
    indirectIrradianceShader.AddUniform("texSingleMieScattering", ParameterType::INT);
    indirectIrradianceShader.AddUniform("texMultipleScattering", ParameterType::INT);
    indirectIrradianceShader.AddUniform("scatteringOrder", ParameterType::INT);
    indirectIrradianceShader.AddUniform("luminanceFromRadiance", ParameterType::MAT3);
    GLSLShader multipleScatteringShader(header, "atmosphereMultipleScattering.frag", "", "atmosphere.geom");
    multipleScatteringShader.AddUniform("texTransmittance", ParameterType::INT);
    multipleScatteringShader.AddUniform("texScatteringDensity", ParameterType::INT);
    multipleScatteringShader.AddUniform("luminanceFromRadiance", ParameterType::MAT3);
    multipleScatteringShader.AddUniform("layer", ParameterType::INT);

    const GLuint kDrawBuffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};

    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

    //Compute the transmittance, and store it in transmittance_texture_.
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures[AtmosphereTextures::TRANSMITTANCE], 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glViewport(0, 0, TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
    transmittanceShader.Use();
    DrawBlendedSAQ({});

    //Compute the direct irradiance, store it in delta_irradiance_texture and,
    //depending on 'blend', either initialize irradiance_texture_ with zeros or
    //leave it unchanged (we don't want the direct irradiance in
    //irradiance_texture_, but only the irradiance from the sky).
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, delta_irradiance_texture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, textures[AtmosphereTextures::IRRADIANCE], 0);
    glDrawBuffers(2, kDrawBuffers);
    glViewport(0, 0, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);
    directIrradianceShader.Use();
    directIrradianceShader.SetUniform("texTransmittance", TEX_POSTPROCESS1);
    
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D, textures[AtmosphereTextures::TRANSMITTANCE]);
    //glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, textures[AtmosphereTextures::TRANSMITTANCE]);
    
    DrawBlendedSAQ({false, blend});

    // Compute the rayleigh and mie single scattering, store them in
    // delta_rayleigh_scattering_texture and delta_mie_scattering_texture, and
    // either store them or accumulate them in scattering_texture_ and
    // optional_single_mie_scattering_texture_.
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, delta_rayleigh_scattering_texture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, delta_mie_scattering_texture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, textures[AtmosphereTextures::SCATTERING], 0);
    glDrawBuffers(3, kDrawBuffers);
    glViewport(0, 0, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT);
    singleScatteringShader.Use();
    singleScatteringShader.SetUniform("texTransmittance", TEX_POSTPROCESS1);
    singleScatteringShader.SetUniform("luminanceFromRadiance", glm::mat3(luminance_from_radiance));
    
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    
    glBindTexture(GL_TEXTURE_2D, textures[AtmosphereTextures::TRANSMITTANCE]);
    //glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, textures[AtmosphereTextures::TRANSMITTANCE]);
    
    for(int layer = 0; layer < SCATTERING_TEXTURE_DEPTH; ++layer)
    {
        singleScatteringShader.SetUniform("layer", layer);
        DrawBlendedSAQ({false, false, blend, blend});
    }

    //Compute the 2nd, 3rd and 4th order of scattering, in sequence.
    for(unsigned int scattering_order = 2; scattering_order <= nScatteringOrders; ++scattering_order)
    {
        //Compute the scattering density, and store it in
        //delta_scattering_density_texture.
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, delta_scattering_density_texture, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, 0, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, 0, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glViewport(0, 0, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT);

        scatteringDensityShader.Use();
        scatteringDensityShader.SetUniform("texTransmittance", TEX_POSTPROCESS1);
        scatteringDensityShader.SetUniform("texSingleRayleighScattering", TEX_POSTPROCESS2);
        scatteringDensityShader.SetUniform("texSingleMieScattering", TEX_POSTPROCESS3);
        scatteringDensityShader.SetUniform("texMultipleScattering", TEX_POSTPROCESS4);
        scatteringDensityShader.SetUniform("texIrradiance", TEX_POSTPROCESS5);
        scatteringDensityShader.SetUniform("scatteringOrder", (int)scattering_order);
        
        glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
        glBindTexture(GL_TEXTURE_2D, textures[AtmosphereTextures::TRANSMITTANCE]);
        glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
        glBindTexture(GL_TEXTURE_3D, delta_rayleigh_scattering_texture);
        glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS3);
        glBindTexture(GL_TEXTURE_3D, delta_mie_scattering_texture);
        glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS4);
        glBindTexture(GL_TEXTURE_3D, delta_multiple_scattering_texture);
        glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS5);
        glBindTexture(GL_TEXTURE_2D, delta_irradiance_texture);
        
//        glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, textures[AtmosphereTextures::TRANSMITTANCE]);
//        glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_3D, delta_rayleigh_scattering_texture);
//        glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS3, GL_TEXTURE_3D, delta_mie_scattering_texture);
//        glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS4, GL_TEXTURE_3D, delta_multiple_scattering_texture);
//        glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS5, GL_TEXTURE_2D, delta_irradiance_texture);

        for(int layer = 0; layer < SCATTERING_TEXTURE_DEPTH; ++layer)
        {
            scatteringDensityShader.SetUniform("layer", layer);
            DrawBlendedSAQ({});
        }

        //Compute the indirect irradiance, store it in delta_irradiance_texture and accumulate it in irradiance_texture.
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, delta_irradiance_texture, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, textures[AtmosphereTextures::IRRADIANCE], 0);
        glDrawBuffers(2, kDrawBuffers);
        glViewport(0, 0, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);

        indirectIrradianceShader.Use();
        indirectIrradianceShader.SetUniform("texSingleRayleighScattering",	TEX_POSTPROCESS2);
        indirectIrradianceShader.SetUniform("texSingleMieScattering", TEX_POSTPROCESS3);
        indirectIrradianceShader.SetUniform("texMultipleScattering", TEX_POSTPROCESS4);
        indirectIrradianceShader.SetUniform("scatteringOrder", (int)scattering_order - 1);
        indirectIrradianceShader.SetUniform("luminanceFromRadiance", glm::mat3(luminance_from_radiance));
        
        glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
        glBindTexture(GL_TEXTURE_3D, delta_rayleigh_scattering_texture);
        glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS3);
        glBindTexture(GL_TEXTURE_3D, delta_mie_scattering_texture);
        glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS4);
        glBindTexture(GL_TEXTURE_3D, delta_multiple_scattering_texture);
        
        //glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_3D, delta_rayleigh_scattering_texture);
        //glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS3, GL_TEXTURE_3D, delta_mie_scattering_texture);
        //glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS4, GL_TEXTURE_3D, delta_multiple_scattering_texture);
        DrawBlendedSAQ({false, true});

        //Compute the multiple scattering, store it in delta_multiple_scattering_texture, and accumulate it in scattering_texture.
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, delta_multiple_scattering_texture, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, textures[AtmosphereTextures::SCATTERING], 0);
        glDrawBuffers(2, kDrawBuffers);
        glViewport(0, 0, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT);

        multipleScatteringShader.Use();
        multipleScatteringShader.SetUniform("texTransmittance", TEX_POSTPROCESS1);
        multipleScatteringShader.SetUniform("texScatteringDensity", TEX_POSTPROCESS2);
        multipleScatteringShader.SetUniform("luminanceFromRadiance", glm::mat3(luminance_from_radiance));
        
        glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
        glBindTexture(GL_TEXTURE_2D, textures[AtmosphereTextures::TRANSMITTANCE]);
        glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
        glBindTexture(GL_TEXTURE_3D, delta_scattering_density_texture);
        
        //glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, textures[AtmosphereTextures::TRANSMITTANCE]);
        //glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_3D, delta_scattering_density_texture);

        for(int layer = 0; layer < SCATTERING_TEXTURE_DEPTH; ++layer) {
            multipleScatteringShader.SetUniform("layer", layer);
            DrawBlendedSAQ({false, true});
        }
    }
    
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
    glBindTexture(GL_TEXTURE_3D, 0);
    
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS3);
    glBindTexture(GL_TEXTURE_3D, 0);
    
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS4);
    glBindTexture(GL_TEXTURE_3D, 0);
    
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS5);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, 0, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, 0, 0);
}

std::string OpenGLAtmosphere::EarthsAtmosphere(const glm::dvec3& lambdas)
{
    //All values ar for Earth's atmosphere
    //Setup atmosphere properties
    constexpr double kSolarIrradiance[48] = {
        1.11776, 1.14259, 1.01249, 1.14716, 1.72765, 1.73054, 1.6887, 1.61253,
        1.91198, 2.03474, 2.02042, 2.02212, 1.93377, 1.95809, 1.91686, 1.8298,
        1.8685, 1.8931, 1.85149, 1.8504, 1.8341, 1.8345, 1.8147, 1.78158, 1.7533,
        1.6965, 1.68194, 1.64654, 1.6048, 1.52143, 1.55622, 1.5113, 1.474, 1.4482,
        1.41018, 1.36775, 1.34188, 1.31429, 1.28303, 1.26758, 1.2367, 1.2082,
        1.18737, 1.14683, 1.12362, 1.1058, 1.07124, 1.04992
    };
    // Values from http://www.iup.uni-bremen.de/gruppen/molspec/databases/ referencespectra/o3spectra2011/index.html for 233K, summed and averaged in
    // each bin (e.g. the value for 360nm is the average of the original values for all wavelengths between 360 and 370nm). Values in m^2.
    constexpr double kOzoneCrossSection[48] = {
        1.18e-27, 2.182e-28, 2.818e-28, 6.636e-28, 1.527e-27, 2.763e-27, 5.52e-27,
        8.451e-27, 1.582e-26, 2.316e-26, 3.669e-26, 4.924e-26, 7.752e-26, 9.016e-26,
        1.48e-25, 1.602e-25, 2.139e-25, 2.755e-25, 3.091e-25, 3.5e-25, 4.266e-25,
        4.672e-25, 4.398e-25, 4.701e-25, 5.019e-25, 4.305e-25, 3.74e-25, 3.215e-25,
        2.662e-25, 2.238e-25, 1.852e-25, 1.473e-25, 1.209e-25, 9.423e-26, 7.455e-26,
        6.566e-26, 5.105e-26, 4.15e-26, 4.228e-26, 3.237e-26, 2.451e-26, 2.801e-26,
        2.534e-26, 1.624e-26, 1.465e-26, 2.078e-26, 1.383e-26, 7.105e-27
    };
    // From https://en.wikipedia.org/wiki/Dobson_unit, in molecules.m^-2.
    constexpr double kDobsonUnit = 2.687e20;
    // Maximum number density of ozone molecules, in m^-3 (computed so at to get 300 Dobson units of ozone - for this we divide 300 DU by the integral of
    // the ozone density profile defined below, which is equal to 15km).
    constexpr double kMaxOzoneNumberDensity = 300.0 * kDobsonUnit / 15000.0;
    // Wavelength independent solar irradiance "spectrum" (not physically realistic, but was used in the original implementation).
    //constexpr double kConstantSolarIrradiance = 1.5;
    constexpr double kBottomRadius = 6360000.0;
    constexpr double kTopRadius = 6420000.0;
    constexpr double kRayleigh = 1.24062e-6;
    constexpr double kRayleighScaleHeight = 8000.0;
    constexpr double kMieScaleHeight = 1200.0;
    constexpr double kMieAngstromAlpha = 0.0;
    constexpr double kMieAngstromBeta = 5.328e-3;
    constexpr double kMieSingleScatteringAlbedo = 0.9;
    constexpr double kMiePhaseFunctionG = 0.8;
    constexpr double kGroundAlbedo = 0.1;
    constexpr double kSunAngularRadius = 0.00935 / 2.0;
    //constexpr double kSunSolidAngle = M_PI * kSunAngularRadius * kSunAngularRadius;
    const double max_sun_zenith_angle = 102.0/180.0 * M_PI;
    double length_unit_in_meters = kLengthUnitInMeters;

    DensityProfileLayer rayleigh_layer({0.0, 1.0, -1.0 / kRayleighScaleHeight, 0.0, 0.0});
    DensityProfileLayer mie_layer({0.0, 1.0, -1.0 / kMieScaleHeight, 0.0, 0.0});
    // Density profile increasing linearly from 0 to 1 between 10 and 25km, and
    // decreasing linearly from 1 to 0 between 25 and 40km. This is an approximate
    // profile from http://www.kln.ac.lk/science/Chemistry/Teaching_Resources/
    // Documents/Introduction%20to%20atmospheric%20chemistry.pdf (page 10).
    std::vector<DensityProfileLayer> ozone_density;
    ozone_density.push_back(DensityProfileLayer({25000.0, 0.0, 0.0, 1.0 / 15000.0, -2.0 / 3.0}));
    ozone_density.push_back(DensityProfileLayer({0.0, 0.0, 0.0, -1.0 / 15000.0, 8.0 / 3.0}));

    std::vector<double> wavelengths;
    std::vector<double> solar_irradiance;
    std::vector<double> rayleigh_scattering;
    std::vector<double> mie_scattering;
    std::vector<double> mie_extinction;
    std::vector<double> absorption_extinction;
    std::vector<double> ground_albedo;

    for(int l = kLambdaMin; l <= kLambdaMax; l += 10) {
        double lambda = static_cast<double>(l) * 1e-3;  // micro-meters
        double mie = kMieAngstromBeta / kMieScaleHeight * pow(lambda, - kMieAngstromAlpha);
        wavelengths.push_back(l);
        solar_irradiance.push_back(kSolarIrradiance[(l - kLambdaMin) / 10]);
        rayleigh_scattering.push_back(kRayleigh * pow(lambda, -4));
        mie_scattering.push_back(mie * kMieSingleScatteringAlbedo);
        mie_extinction.push_back(mie);
        absorption_extinction.push_back(kMaxOzoneNumberDensity * kOzoneCrossSection[(l - kLambdaMin) / 10]);
        ground_albedo.push_back(kGroundAlbedo);
    }

    //Compute white balance
    double white_point_r = 1.0;
    double white_point_g = 1.0;
    double white_point_b = 1.0;
    ConvertSpectrumToLinearSrgb(wavelengths, solar_irradiance, &white_point_r, &white_point_g, &white_point_b);
    double white_point = (white_point_r + white_point_g + white_point_b) / 3.0;
    whitePoint = glm::vec3(white_point_r/white_point, white_point_g/white_point, white_point_b/white_point);

    //C++ Lambdas
    auto to_string = [&wavelengths](const std::vector<double>& v, const glm::dvec3& lambdas, double scale) {
        double r = Interpolate(wavelengths, v, lambdas[0]) * scale;
        double g = Interpolate(wavelengths, v, lambdas[1]) * scale;
        double b = Interpolate(wavelengths, v, lambdas[2]) * scale;
        return "vec3(" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + ")";
    };

    auto density_layer = [length_unit_in_meters](const DensityProfileLayer& layer) {
        return "DensityProfileLayer(" +
               std::to_string(layer.width / length_unit_in_meters) + "," +
               std::to_string(layer.exp_term) + "," +
               std::to_string(layer.exp_scale * length_unit_in_meters) + "," +
               std::to_string(layer.linear_term * length_unit_in_meters) + "," +
               std::to_string(layer.constant_term) + ")";
    };

    auto density_profile = [density_layer](std::vector<DensityProfileLayer> layers) {
        constexpr int kLayerCount = 2;
        while (layers.size() < kLayerCount) {
            layers.insert(layers.begin(), DensityProfileLayer());
        }
        std::string result = "DensityProfile(DensityProfileLayer[" + std::to_string(kLayerCount) + "](";
        for(int i = 0; i < kLayerCount; ++i) {
            result += density_layer(layers[i]);
            result += i < kLayerCount - 1 ? "," : "))";
        }
        return result;
    };

    // Compute the values for the SKY_RADIANCE_TO_LUMINANCE constant. In theory
    // this should be 1 in precomputed illuminance mode (because the precomputed
    // textures already contain illuminance values). In practice, however, storing
    // true illuminance values in half precision textures yields artefacts
    // (because the values are too large), so we store illuminance values divided
    // by MAX_LUMINOUS_EFFICACY instead. This is why, in precomputed illuminance
    // mode, we set SKY_RADIANCE_TO_LUMINANCE to MAX_LUMINOUS_EFFICACY.
    bool precompute_illuminance = nPrecomputedWavelengths > 3;
    double sky_k_r, sky_k_g, sky_k_b;
    if(precompute_illuminance)
        sky_k_r = sky_k_g = sky_k_b = MAX_LUMINOUS_EFFICACY;
    else
        ComputeSpectralRadianceToLuminanceFactors(wavelengths, solar_irradiance, -3 /* lambda_power */, &sky_k_r, &sky_k_g, &sky_k_b);

    // Compute the values for the SUN_RADIANCE_TO_LUMINANCE constant.
    double sun_k_r, sun_k_g, sun_k_b;
    ComputeSpectralRadianceToLuminanceFactors(wavelengths, solar_irradiance, 0 /* lambda_power */, &sun_k_r, &sun_k_g, &sun_k_b);

    std::string header;
    //Shader language version
    header = "#version 330\n";
    //Constants
    header += "const int TRANSMITTANCE_TEXTURE_WIDTH = " + std::to_string(TRANSMITTANCE_TEXTURE_WIDTH) + ";\n";
    header += "const int TRANSMITTANCE_TEXTURE_HEIGHT = " + std::to_string(TRANSMITTANCE_TEXTURE_HEIGHT) + ";\n";
    header += "const int SCATTERING_TEXTURE_R_SIZE = " + std::to_string(SCATTERING_TEXTURE_R_SIZE) + ";\n";
    header += "const int SCATTERING_TEXTURE_MU_SIZE = " + std::to_string(SCATTERING_TEXTURE_MU_SIZE) + ";\n";
    header += "const int SCATTERING_TEXTURE_MU_S_SIZE = " + std::to_string(SCATTERING_TEXTURE_MU_S_SIZE) + ";\n";
    header += "const int SCATTERING_TEXTURE_NU_SIZE = " + std::to_string(SCATTERING_TEXTURE_NU_SIZE) + ";\n";
    header += "const int IRRADIANCE_TEXTURE_WIDTH = " + std::to_string(IRRADIANCE_TEXTURE_WIDTH) + ";\n";
    header += "const int IRRADIANCE_TEXTURE_HEIGHT = " + std::to_string(IRRADIANCE_TEXTURE_HEIGHT) + ";\n";
    header += "#define COMBINED_SCATTERING_TEXTURES\n";
    header += glslDefinitions;
    //Atmosphere structure
    header += "const AtmosphereParameters atmosphere = AtmosphereParameters(\n";
    header += to_string(solar_irradiance, lambdas, 1.0) + ",\n"
              + std::to_string(kSunAngularRadius) + ",\n"
              + std::to_string(kBottomRadius / length_unit_in_meters) + ",\n"
              + std::to_string(kTopRadius / length_unit_in_meters) + ",\n"
              + density_profile({rayleigh_layer}) + ",\n"
              + to_string(rayleigh_scattering, lambdas, length_unit_in_meters) + ",\n"
              + density_profile({mie_layer}) + ",\n"
              + to_string(mie_scattering, lambdas, length_unit_in_meters) + ",\n"
              + to_string(mie_extinction, lambdas, length_unit_in_meters) + ",\n"
              + std::to_string(kMiePhaseFunctionG) + ",\n"
              + density_profile(ozone_density) + ",\n"
              + to_string(absorption_extinction, lambdas, length_unit_in_meters) + ",\n"
              + to_string(ground_albedo, lambdas, 1.0) + ",\n"
              + std::to_string(cos(max_sun_zenith_angle)) + ");\n"
              + "const vec3 SKY_SPECTRAL_RADIANCE_TO_LUMINANCE = vec3(" + std::to_string(sky_k_r) + "," + std::to_string(sky_k_g) + "," + std::to_string(sky_k_b) + ");\n"
              + "const vec3 SUN_SPECTRAL_RADIANCE_TO_LUMINANCE = vec3(" + std::to_string(sun_k_r) + "," + std::to_string(sun_k_g) + "," + std::to_string(sun_k_b) + ");\n";
    //Functions
    header += glslFunctions;
    return header;
}


void OpenGLAtmosphere::ComputeSpectralRadianceToLuminanceFactors(const std::vector<double>& wavelengths, const std::vector<double>& solar_irradiance,
        double lambda_power, double* k_r, double* k_g, double* k_b)
{
    *k_r = 0.0;
    *k_g = 0.0;
    *k_b = 0.0;
    double solar_r = Interpolate(wavelengths, solar_irradiance, kLambdaR);
    double solar_g = Interpolate(wavelengths, solar_irradiance, kLambdaG);
    double solar_b = Interpolate(wavelengths, solar_irradiance, kLambdaB);
    int dlambda = 1;
    for(int lambda = kLambdaMin; lambda < kLambdaMax; lambda += dlambda) {
        double x_bar = CieColorMatchingFunctionTableValue(lambda, 1);
        double y_bar = CieColorMatchingFunctionTableValue(lambda, 2);
        double z_bar = CieColorMatchingFunctionTableValue(lambda, 3);
        const double* xyz2srgb = XYZ_TO_SRGB;
        double r_bar = xyz2srgb[0] * x_bar + xyz2srgb[1] * y_bar + xyz2srgb[2] * z_bar;
        double g_bar = xyz2srgb[3] * x_bar + xyz2srgb[4] * y_bar + xyz2srgb[5] * z_bar;
        double b_bar = xyz2srgb[6] * x_bar + xyz2srgb[7] * y_bar + xyz2srgb[8] * z_bar;
        double irradiance = Interpolate(wavelengths, solar_irradiance, lambda);
        *k_r += r_bar * irradiance / solar_r * pow(lambda / kLambdaR, lambda_power);
        *k_g += g_bar * irradiance / solar_g * pow(lambda / kLambdaG, lambda_power);
        *k_b += b_bar * irradiance / solar_b * pow(lambda / kLambdaB, lambda_power);
    }
    *k_r *= MAX_LUMINOUS_EFFICACY * dlambda;
    *k_g *= MAX_LUMINOUS_EFFICACY * dlambda;
    *k_b *= MAX_LUMINOUS_EFFICACY * dlambda;
}

void OpenGLAtmosphere::ConvertSpectrumToLinearSrgb(const std::vector<double>& wavelengths, const std::vector<double>& spectrum, double* r, double* g, double* b)
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    const int dlambda = 1;

    for(int lambda = kLambdaMin; lambda < kLambdaMax; lambda += dlambda) {
        double value = Interpolate(wavelengths, spectrum, lambda);
        x += CieColorMatchingFunctionTableValue(lambda, 1) * value;
        y += CieColorMatchingFunctionTableValue(lambda, 2) * value;
        z += CieColorMatchingFunctionTableValue(lambda, 3) * value;
    }
    *r = MAX_LUMINOUS_EFFICACY * (XYZ_TO_SRGB[0] * x + XYZ_TO_SRGB[1] * y + XYZ_TO_SRGB[2] * z) * dlambda;
    *g = MAX_LUMINOUS_EFFICACY * (XYZ_TO_SRGB[3] * x + XYZ_TO_SRGB[4] * y + XYZ_TO_SRGB[5] * z) * dlambda;
    *b = MAX_LUMINOUS_EFFICACY * (XYZ_TO_SRGB[6] * x + XYZ_TO_SRGB[7] * y + XYZ_TO_SRGB[8] * z) * dlambda;
}

void OpenGLAtmosphere::DrawBlendedSAQ(const std::vector<bool>& enableBlend)
{
    for(unsigned int i=0; i<enableBlend.size(); ++i)
        if(enableBlend[i])
            glEnablei(GL_BLEND, i);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    for(unsigned int i = 0; i<enableBlend.size(); ++i)
        glDisablei(GL_BLEND, i);
}

void OpenGLAtmosphere::ShowAtmosphereTexture(AtmosphereTextures id, glm::vec4 rect)
{
    switch(id) {
    case TRANSMITTANCE:
    case IRRADIANCE:
        OpenGLContent::getInstance()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, textures[id]);
        break;

    case SCATTERING:
        OpenGLContent::getInstance()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, textures[id], 0, false);
        break;

    default:
        break;
    }
}

void OpenGLAtmosphere::ShowSunShadowmaps(GLfloat x, GLfloat y, GLfloat scale)
{
    //Texture setup
    glActiveTexture(GL_TEXTURE0 + TEX_SUN_SHADOW);
    glBindTexture(GL_TEXTURE_2D_ARRAY, sunShadowmapArray);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    //Render the shadowmaps
    sunShadowmapShader->Use();
    sunShadowmapShader->SetUniform("shadowmapArray", TEX_SUN_SHADOW);
    for(unsigned int i = 0; i < sunShadowmapSplits; ++i) {
        glViewport(x + sunShadowmapSize * scale * i, y, sunShadowmapSize * scale, sunShadowmapSize * scale);
        sunShadowmapShader->SetUniform("shadowmapLayer", (GLfloat)i);
        OpenGLContent::getInstance()->DrawSAQ();
    }
    glUseProgram(0);

    //Reset
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}
