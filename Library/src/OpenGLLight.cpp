//
//  OpenGLLight.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/12/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "OpenGLLight.h"
#include "OpenGLSolids.h"
#include "GeometryUtil.h"

//static variables
OpenGLView* OpenGLLight::activeView = NULL;
GLSLShader* OpenGLLight::spotShader = NULL;
GLSLShader* OpenGLLight::omniShader = NULL;
GLSLShader* OpenGLLight::ambientShader = NULL;
GLint OpenGLLight::diffuseTextureUnit = 0;
GLint OpenGLLight::normalTextureUnit = 0;
GLint OpenGLLight::positionTextureUnit = 0;
GLint OpenGLLight::skyDiffuseTextureUnit = 0;
GLint OpenGLLight::skyReflectionTextureUnit = 0;
GLint OpenGLLight::shadowTextureUnit = 0;
GLint OpenGLLight::ssaoTextureUnit = 0;
ColorSystem OpenGLLight::cs = {0.64f, 0.33f, 0.3f, 0.6f, 0.15f, 0.06f, 0.3127f, 0.3291f, 0.0}; //sRGB color space
////////////////////


OpenGLLight::OpenGLLight(const btVector3& position, glm::vec4 c)
{
    color = c;
    relpos = UnitSystem::SetPosition(position);
    pos = relpos;
    active = true;
    holdingEntity = NULL;
    surfaceDistance = 0;
}

OpenGLLight::~OpenGLLight()
{
    holdingEntity = NULL;
}

void OpenGLLight::GlueToEntity(SolidEntity* ent)
{
    holdingEntity = ent;
}

void OpenGLLight::Activate()
{
    active = true;
}

void OpenGLLight::Deactivate()
{
    active = false;
}

bool OpenGLLight::isActive()
{
    return active;
}

void OpenGLLight::setLightSurfaceDistance(GLfloat dist)
{
    surfaceDistance = UnitSystem::SetLength(dist);
}

glm::vec4 OpenGLLight::getColor()
{
    return color;
}

btVector3 OpenGLLight::getViewPosition()
{
    //transform to eye space
    btVector3 esPos = activeView->GetViewTransform() * pos;
    return esPos;
}

btVector3 OpenGLLight::getPosition()
{
    return pos;
}

SolidEntity* OpenGLLight::getHoldingEntity()
{
    return holdingEntity;
}

//////////////////static//////////////////////////////
void OpenGLLight::Init()
{
    //AMBIENT
    ambientShader = new GLSLShader("deferredAmbient.frag");
    ambientShader->AddUniform("texDiffuse", INT);
    ambientShader->AddUniform("texNormal", INT);
    ambientShader->AddUniform("texSSAO", INT);
    ambientShader->AddUniform("texSkyDiff", INT);
    ambientShader->AddUniform("inv_view_rot", MAT3);
    
    //OMNI
    omniShader = new GLSLShader("deferredOmni.frag");
    omniShader->AddUniform("texDiffuse", INT);
    omniShader->AddUniform("texPosition", INT);
    omniShader->AddUniform("texNormal", INT);
    omniShader->AddUniform("lightPosition", VEC3);
    omniShader->AddUniform("lightColor", VEC4);
  
    //SPOT
    spotShader = new GLSLShader("deferredSpot.frag");
    spotShader->AddUniform("texDiffuse", INT);
    spotShader->AddUniform("texPosition", INT);
    spotShader->AddUniform("texNormal", INT);
    spotShader->AddUniform("lightPosition", VEC3);
    spotShader->AddUniform("lightDirection", VEC3);
    spotShader->AddUniform("lightAngle", FLOAT);
    spotShader->AddUniform("lightColor", VEC4);
    spotShader->AddUniform("texShadow", INT);
    spotShader->AddUniform("lightClipSpace", MAT4);
}

void OpenGLLight::Destroy()
{
    delete ambientShader;
    delete omniShader;
    delete spotShader;
}

void OpenGLLight::SetTextureUnits(GLint diffuse, GLint normal, GLint position, GLint skyDiffuse, GLint skyReflection, GLint ssao, GLint shadow)
{
    diffuseTextureUnit = diffuse;
    normalTextureUnit = normal;
    positionTextureUnit = position;
    skyDiffuseTextureUnit = skyDiffuse;
    skyReflectionTextureUnit = skyReflection;
    ssaoTextureUnit = ssao;
    shadowTextureUnit = shadow;
}

void OpenGLLight::SetCamera(OpenGLView* view)
{
    activeView = view;
}

void OpenGLLight::RenderAmbientLight(const btTransform& viewTransform, bool zAxisUp)
{
    glm::mat4 proj = activeView->GetProjectionMatrix();
    proj = glm::inverse(proj);
    
    //GLint* viewport = activeView->GetViewport();
    
    btMatrix3x3 flip;
    flip.setEulerZYX(zAxisUp ? -M_PI_2 : M_PI_2, 0, 0);
    flip = flip * (viewTransform.getBasis().inverse());
    
    GLfloat IVRMatrix[9];
    SetFloatvFromMat(flip, IVRMatrix);
    glm::mat3 ivr = glm::make_mat3(IVRMatrix);
    
    ambientShader->Enable();
    ambientShader->SetUniform("texDiffuse", diffuseTextureUnit);
    ambientShader->SetUniform("texNormal", normalTextureUnit);
    ambientShader->SetUniform("texSkyDiff", skyDiffuseTextureUnit);
    ambientShader->SetUniform("texSSAO", ssaoTextureUnit);
    ambientShader->SetUniform("inv_view_rot", ivr);
    //ambientShader->SetUniform("texSkyReflect", skyReflectionTextureUnit);
    //ambientShader->SetUniform("inv_proj", proj);
    //ambientShader->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
    OpenGLSolids::DrawScreenAlignedQuad();
    ambientShader->Disable();
}

glm::vec4 OpenGLLight::ColorFromTemperature(GLfloat temperatureK, GLfloat intensity)
{
    glm::vec4 color4;
    GLfloat c1,c2,c3;
    bbSpectrumToXYZ(temperatureK, c1, c2, c3);
    xyzToRGB(c1, c2, c3, c1, c2, c3);
    color4[0] = c1*intensity;
    color4[1] = c2*intensity;
    color4[2] = c3*intensity;
    color4[3] = 1.f;
    return color4;
}

GLfloat OpenGLLight::bbSpectrum(GLfloat wavelength, GLfloat temperature)
{
    double wlm = wavelength * 1e-9;   /* Wavelength in meters */
    return (3.74183e-16 * pow(wlm, -5.0)) / (exp(1.4388e-2 / (wlm * temperature)) - 1.0);
}

void OpenGLLight::bbSpectrumToXYZ(GLfloat temperature, GLfloat& x, GLfloat& y, GLfloat& z)
{
    GLfloat X = 0, Y = 0, Z = 0, XYZ;
    
    static GLfloat cie_colour_match[81][3] = {
        {0.0014,0.0000,0.0065}, {0.0022,0.0001,0.0105}, {0.0042,0.0001,0.0201},
        {0.0076,0.0002,0.0362}, {0.0143,0.0004,0.0679}, {0.0232,0.0006,0.1102},
        {0.0435,0.0012,0.2074}, {0.0776,0.0022,0.3713}, {0.1344,0.0040,0.6456},
        {0.2148,0.0073,1.0391}, {0.2839,0.0116,1.3856}, {0.3285,0.0168,1.6230},
        {0.3483,0.0230,1.7471}, {0.3481,0.0298,1.7826}, {0.3362,0.0380,1.7721},
        {0.3187,0.0480,1.7441}, {0.2908,0.0600,1.6692}, {0.2511,0.0739,1.5281},
        {0.1954,0.0910,1.2876}, {0.1421,0.1126,1.0419}, {0.0956,0.1390,0.8130},
        {0.0580,0.1693,0.6162}, {0.0320,0.2080,0.4652}, {0.0147,0.2586,0.3533},
        {0.0049,0.3230,0.2720}, {0.0024,0.4073,0.2123}, {0.0093,0.5030,0.1582},
        {0.0291,0.6082,0.1117}, {0.0633,0.7100,0.0782}, {0.1096,0.7932,0.0573},
        {0.1655,0.8620,0.0422}, {0.2257,0.9149,0.0298}, {0.2904,0.9540,0.0203},
        {0.3597,0.9803,0.0134}, {0.4334,0.9950,0.0087}, {0.5121,1.0000,0.0057},
        {0.5945,0.9950,0.0039}, {0.6784,0.9786,0.0027}, {0.7621,0.9520,0.0021},
        {0.8425,0.9154,0.0018}, {0.9163,0.8700,0.0017}, {0.9786,0.8163,0.0014},
        {1.0263,0.7570,0.0011}, {1.0567,0.6949,0.0010}, {1.0622,0.6310,0.0008},
        {1.0456,0.5668,0.0006}, {1.0026,0.5030,0.0003}, {0.9384,0.4412,0.0002},
        {0.8544,0.3810,0.0002}, {0.7514,0.3210,0.0001}, {0.6424,0.2650,0.0000},
        {0.5419,0.2170,0.0000}, {0.4479,0.1750,0.0000}, {0.3608,0.1382,0.0000},
        {0.2835,0.1070,0.0000}, {0.2187,0.0816,0.0000}, {0.1649,0.0610,0.0000},
        {0.1212,0.0446,0.0000}, {0.0874,0.0320,0.0000}, {0.0636,0.0232,0.0000},
        {0.0468,0.0170,0.0000}, {0.0329,0.0119,0.0000}, {0.0227,0.0082,0.0000},
        {0.0158,0.0057,0.0000}, {0.0114,0.0041,0.0000}, {0.0081,0.0029,0.0000},
        {0.0058,0.0021,0.0000}, {0.0041,0.0015,0.0000}, {0.0029,0.0010,0.0000},
        {0.0020,0.0007,0.0000}, {0.0014,0.0005,0.0000}, {0.0010,0.0004,0.0000},
        {0.0007,0.0002,0.0000}, {0.0005,0.0002,0.0000}, {0.0003,0.0001,0.0000},
        {0.0002,0.0001,0.0000}, {0.0002,0.0001,0.0000}, {0.0001,0.0000,0.0000},
        {0.0001,0.0000,0.0000}, {0.0001,0.0000,0.0000}, {0.0000,0.0000,0.0000}
    };
    
    for (int i=0, lambda=380; lambda<780.1; i++,lambda+=5)
    {
        GLfloat Me;
        Me = bbSpectrum(lambda, temperature);
        X += Me * cie_colour_match[i][0];
        Y += Me * cie_colour_match[i][1];
        Z += Me * cie_colour_match[i][2];
    }
    
    XYZ = (X + Y + Z);
    x = X / XYZ;
    y = Y / XYZ;
    z = Z / XYZ;
}

void OpenGLLight::xyzToRGB(GLfloat x, GLfloat y, GLfloat z, GLfloat& r, GLfloat& g, GLfloat& b)
{
    GLfloat xr, yr, zr, xg, yg, zg, xb, yb, zb;
    GLfloat xw, yw, zw;
    GLfloat rx, ry, rz, gx, gy, gz, bx, by, bz;
    GLfloat rw, gw, bw;
    
    xr = cs.xRed;    yr = cs.yRed;    zr = 1 - (xr + yr);
    xg = cs.xGreen;  yg = cs.yGreen;  zg = 1 - (xg + yg);
    xb = cs.xBlue;   yb = cs.yBlue;   zb = 1 - (xb + yb);
    
    xw = cs.xWhite;  yw = cs.yWhite;  zw = 1 - (xw + yw);
    
    /* xyz -> rgb matrix, before scaling to white. */
    rx = (yg * zb) - (yb * zg);  ry = (xb * zg) - (xg * zb);  rz = (xg * yb) - (xb * yg);
    gx = (yb * zr) - (yr * zb);  gy = (xr * zb) - (xb * zr);  gz = (xb * yr) - (xr * yb);
    bx = (yr * zg) - (yg * zr);  by = (xg * zr) - (xr * zg);  bz = (xr * yg) - (xg * yr);
    
    /* White scaling factors.
     Dividing by yw scales the white luminance to unity, as conventional. */
    rw = ((rx * xw) + (ry * yw) + (rz * zw)) / yw;
    gw = ((gx * xw) + (gy * yw) + (gz * zw)) / yw;
    bw = ((bx * xw) + (by * yw) + (bz * zw)) / yw;
    
    /* xyz -> rgb matrix, correctly scaled to white. */
    rx = rx / rw;  ry = ry / rw;  rz = rz / rw;
    gx = gx / gw;  gy = gy / gw;  gz = gz / gw;
    bx = bx / bw;  by = by / bw;  bz = bz / bw;
    
    /* rgb of the desired point */
    r = (rx * x) + (ry * y) + (rz * z);
    g = (gx * x) + (gy * y) + (gz * z);
    b = (bx * x) + (by * y) + (bz * z);
    
    /* constrain rgb */
    GLfloat w;
    w = (0 < r) ? 0 : r;
    w = (w < g) ? w : g;
    w = (w < b) ? w : b;
    w = -w;
    
    /* add just enough white to make r, g, b all positive. */
    if(w > 0)
    {
        r += w;
        g += w;
        b += w;
    }
    
    /* normalize rgb */
    GLfloat maximum = Max(r, Max(g,b));
    if(maximum > 1.0)
    {
        r /= maximum;
        g /= maximum;
        b /= maximum;
    }
}





